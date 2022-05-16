#include "liblt.h"

#include "lt_common.h"

#include <intrin.h>

#include <windows.h>
#include <winternl.h>
#include <versionhelpers.h>

#include <shlwapi.h>
#pragma comment(lib, "Shlwapi.lib")

#include <shellscalingapi.h>
#pragma comment(lib, "Shcore.lib")

#include <Wbemidl.h>
#pragma comment(lib, "wbemuuid.lib")

#define INITGUID
#include <ddraw.h>
#undef INITGUID

#pragma comment(lib, "dxguid.lib")

#include <dxgi1_4.h>

#include "..\..\builds\bin\client\ltsend_exe.h"
#define lt_send_executable ___builds_bin_client_ltsend_exe

//////////////////////////////////////////////////////////////////////////

#define STRINGIFY(a) #a
#define STRINGIFY_VALUE(a) STRINGIFY(a)

#define REMOTE_POINTER
#define REMOTE_DATA

//////////////////////////////////////////////////////////////////////////

const char * lt_get_product_name()
{
  extern const char * g_lt_product_name;
  return g_lt_product_name;
}

uint64_t lt_get_product_major_version()
{
  extern const uint64_t g_lt_product_major_version;
  return g_lt_product_major_version;
}

uint64_t lt_get_product_minor_version()
{
  extern const uint64_t g_lt_product_minor_version;
  return g_lt_product_minor_version;
}

bool lt_is_debug_build()
{
  extern const bool g_lt_is_debug_build;
  return g_lt_is_debug_build;
}

const char * lt_get_remote_host()
{
  extern const char * g_lt_remote_host;
  return g_lt_remote_host;
}

const char * lt_get_output_folder_path()
{
  extern const char * g_lt_folder_path;
  return g_lt_folder_path;
}

const bool lt_get_crash_stack_trace_include_data()
{
  extern const bool g_lt_crash_stack_trace_include_data;
  return g_lt_crash_stack_trace_include_data;;
}

const bool lt_error_include_stack_trace()
{
  extern const bool g_lt_error_include_stack_trace;
  return g_lt_error_include_stack_trace;
}

const bool lt_warn_include_stack_trace()
{
  extern const bool g_lt_warn_include_stack_trace;
  return g_lt_warn_include_stack_trace;
}

//////////////////////////////////////////////////////////////////////////

static bool lt_init();
static void lt_write_block(IN const uint8_t *pData, const size_t size);
static void lt_write_block_internal(IN const uint8_t *pData, const size_t size);
static size_t lt_write_stack_trace(OUT uint8_t *pStackTrace, const bool includeData);
static size_t lt_write_foreign_stack_trace(OUT uint8_t *pStackTrace, const bool includeData, const DWORD processId, const DWORD threadId);
static size_t lt_write_system_info(OUT uint8_t *pData);

inline uint64_t lt_time_100ns()
{
  FILETIME time;
  GetSystemTimePreciseAsFileTime(&time);

  return ((uint64_t)time.dwLowDateTime | ((uint64_t)time.dwHighDateTime << 32));
}

constexpr size_t lt_stack_trace_depth = 32;

//////////////////////////////////////////////////////////////////////////

static struct lt_context
{
  volatile bool initialized;
  volatile bool failed;
  HANDLE mutex;
  HANDLE file;
} _lt_context = {};

//////////////////////////////////////////////////////////////////////////

void lt_crash_ex(const DWORD processId, const DWORD threadId, const uint64_t errorCode, IN OPTIONAL const char *description)
{
  if (!lt_init())
    return;

  const uint64_t timestamp = lt_time_100ns();

  uint8_t data[1024 * 10];
  static_assert(sizeof(data) > 1 + 4 + 8 + 8 + 1 + 255 + 1 + 2 + 8 + lt_stack_trace_depth * (1 + 1 + 255 + 8) + 1, "data size may be insufficient.");

  uint8_t *pData = data;

  *pData = lt_t_crash;
  pData += sizeof(uint8_t) + sizeof(uint16_t); // size of this block.

  *reinterpret_cast<uint64_t *>(pData) = timestamp;
  pData += sizeof(uint64_t);

  *reinterpret_cast<uint64_t *>(pData) = errorCode;
  pData += sizeof(uint64_t);

  if (description != nullptr)
  {
    const uint8_t size = (uint8_t)min(0xFF, strlen(description));

    *pData = size;
    pData++;

    memcpy(pData, description, size);
    pData += size;
  }
  else
  {
    *pData = 0;
    pData++;
  }

  const uint16_t stackTraceSize = (uint16_t)lt_write_foreign_stack_trace(pData + 2, lt_get_crash_stack_trace_include_data(), processId, threadId);
  *reinterpret_cast<uint16_t *>(pData) = stackTraceSize;
  pData += 2ULL + stackTraceSize;

  *reinterpret_cast<uint16_t *>(&data[1]) = (uint16_t)(pData - data);

  lt_write_block(data, pData - data);
}

void lt_crash(const uint64_t errorCode, IN OPTIONAL const char *description)
{
  if (!lt_init())
    return;

  const uint64_t timestamp = lt_time_100ns();

  uint8_t data[1024 * 10];
  static_assert(sizeof(data) > 1 + 4 + 8 + 8 + 1 + 255 + 1 + 2 + 8 + lt_stack_trace_depth * (1 + 1 + 255 + 8) + 1, "data size may be insufficient.");

  uint8_t *pData = data;

  *pData = lt_t_crash;
  pData += sizeof(uint8_t) + sizeof(uint16_t); // size of this block.

  *reinterpret_cast<uint64_t *>(pData) = timestamp;
  pData += sizeof(uint64_t);

  *reinterpret_cast<uint64_t *>(pData) = errorCode;
  pData += sizeof(uint64_t);
  
  if (description != nullptr)
  {
    const uint8_t size = (uint8_t)min(0xFF, strlen(description));
  
    *pData = size;
    pData++;
  
    memcpy(pData, description, size);
    pData += size;
  }
  else
  {
    *pData = 0;
    pData++;
  }
  
  const uint16_t stackTraceSize = (uint16_t)lt_write_stack_trace(pData + 2, lt_get_crash_stack_trace_include_data());
  *reinterpret_cast<uint16_t *>(pData) = stackTraceSize;
  pData += 2ULL + stackTraceSize;
  
  *reinterpret_cast<uint16_t *>(&data[1]) = (uint16_t)(pData - data);

  lt_write_block(data, pData - data);
}

void lt_error(const uint64_t subSystem, const uint64_t errorCode, IN OPTIONAL const char *description)
{
  if (!lt_init())
    return;

  const uint64_t timestamp = lt_time_100ns();

  uint8_t data[1024 * 10];
  static_assert(sizeof(data) > 1 + 4 + 8 + 8 + 8 + 1 + 255 + 1 + 2 + 8 + lt_stack_trace_depth * (1 + 1 + 255 + 8) + 1, "data size may be insufficient.");

  uint8_t *pData = data;

  *pData = lt_t_error;
  pData += sizeof(uint8_t) + sizeof(uint16_t); // size of this block.

  *reinterpret_cast<uint64_t *>(pData) = timestamp;
  pData += sizeof(uint64_t);

  *reinterpret_cast<uint64_t *>(pData) = subSystem;
  pData += sizeof(uint64_t);

  *reinterpret_cast<uint64_t *>(pData) = errorCode;
  pData += sizeof(uint64_t);

  if (description != nullptr)
  {
    const uint8_t size = (uint8_t)min(0xFF, strlen(description));

    *pData = size;
    pData++;

    memcpy(pData, description, size);
    pData += size;
  }
  else
  {
    *pData = 0;
    pData++;
  }

  if (lt_error_include_stack_trace())
  {
    const uint16_t stackTraceSize = (uint16_t)lt_write_stack_trace(pData + 2, false);
    *reinterpret_cast<uint16_t *>(pData) = stackTraceSize;
    pData += 2ULL + stackTraceSize;
  }
  else
  {
    *reinterpret_cast<uint16_t *>(pData) = 0;
    pData += sizeof(uint16_t);
  }

  *reinterpret_cast<uint16_t *>(&data[1]) = (uint16_t)(pData - data);

  lt_write_block(data, pData - data);
}

void lt_warn(const uint64_t subSystem, const uint64_t errorCode, IN OPTIONAL const char *description)
{
  if (!lt_init())
    return;

  const uint64_t timestamp = lt_time_100ns();

  uint8_t data[1024 * 10];
  static_assert(sizeof(data) > 1 + 4 + 8 + 8 + 8 + 1 + 255 + 1 + 2 + 8 + lt_stack_trace_depth * (1 + 1 + 255 + 8) + 1, "data size may be insufficient.");

  uint8_t *pData = data;

  *pData = lt_t_warning;
  pData += sizeof(uint8_t) + sizeof(uint16_t); // size of this block.

  *reinterpret_cast<uint64_t *>(pData) = timestamp;
  pData += sizeof(uint64_t);

  *reinterpret_cast<uint64_t *>(pData) = subSystem;
  pData += sizeof(uint64_t);

  *reinterpret_cast<uint64_t *>(pData) = errorCode;
  pData += sizeof(uint64_t);

  if (description != nullptr)
  {
    const uint8_t size = (uint8_t)min(0xFF, strlen(description));

    *pData = size;
    pData++;

    memcpy(pData, description, size);
    pData += size;
  }
  else
  {
    *pData = 0;
    pData++;
  }

  if (lt_warn_include_stack_trace())
  {
    const uint16_t stackTraceSize = (uint16_t)lt_write_stack_trace(pData + 2, false);
    *reinterpret_cast<uint16_t *>(pData) = stackTraceSize;
    pData += 2ULL + stackTraceSize;
  }
  else
  {
    *reinterpret_cast<uint16_t *>(pData) = 0;
    pData += sizeof(uint16_t);
  }

  *reinterpret_cast<uint16_t *>(&data[1]) = (uint16_t)(pData - data);

  lt_write_block(data, pData - data);
}

void lt_log(const uint64_t subSystem, IN OPTIONAL const char *description)
{
  if (!lt_init())
    return;

  const uint64_t timestamp = lt_time_100ns();

  uint8_t data[1024 * 10];
  static_assert(sizeof(data) > 1 + 4 + 8 + 8 + 1 + 255, "data size may be insufficient.");

  uint8_t *pData = data;

  *pData = lt_t_log;
  pData += sizeof(uint8_t) + sizeof(uint16_t); // size of this block.

  *reinterpret_cast<uint64_t *>(pData) = timestamp;
  pData += sizeof(uint64_t);

  *reinterpret_cast<uint64_t *>(pData) = subSystem;
  pData += sizeof(uint64_t);

  if (description != nullptr)
  {
    const uint8_t size = (uint8_t)min(0xFF, strlen(description));

    *pData = size;
    pData++;

    memcpy(pData, description, size);
    pData += size;
  }
  else
  {
    *pData = 0;
    pData++;
  }

  *reinterpret_cast<uint16_t *>(&data[1]) = (uint16_t)(pData - data);

  lt_write_block(data, pData - data);
}

void lt_set_state(const uint64_t subSystem, const uint64_t stateIndex, const uint64_t subStateIndex)
{
  if (!lt_init())
    return;

  const uint64_t timestamp = lt_time_100ns();
  uint8_t data[_lt_state_length];

  data[0] = lt_t_state;
  *reinterpret_cast<uint64_t *>(&data[1]) = subSystem;
  *reinterpret_cast<uint64_t *>(&data[9]) = stateIndex;
  *reinterpret_cast<uint64_t *>(&data[9 + 8]) = subStateIndex;
  *reinterpret_cast<uint64_t *>(&data[9 + 16]) = timestamp;

  static_assert(sizeof(data) == 9 + 24, "invalid data size.");

  lt_write_block(data, sizeof(data));
}

void lt_perf_data(const uint64_t subSystem, IN const double *pDataMs, const uint8_t count)
{
  if (pDataMs == nullptr || count == 0)
    return;

  if (!lt_init())
    return;

  const uint64_t timestamp = lt_time_100ns();

  uint8_t data[1024 * 3];
  static_assert(sizeof(data) > 1 + 2 + 8 + 8 + 1 + 8 * 256, "data size may be insufficient.");

  uint8_t *pData = data;

  *pData = lt_t_perf_data;
  pData += sizeof(uint8_t) + sizeof(uint16_t); // size of this block.

  *reinterpret_cast<uint64_t *>(pData) = timestamp;
  pData += sizeof(uint64_t);

  *reinterpret_cast<uint64_t *>(pData) = subSystem;
  pData += sizeof(uint64_t);

  *pData = count;
  pData++;

  const size_t dataSize = sizeof(double) * (size_t)count;
  memcpy(pData, pDataMs, dataSize);
  pData += dataSize;

  *reinterpret_cast<uint16_t *>(&data[1]) = (uint16_t)(pData - data);

  lt_write_block(data, pData - data);
}

void lt_operation(const uint64_t subSystem, const uint64_t operationType, const uint64_t operationIndex)
{
  if (!lt_init())
    return;

  const uint64_t timestamp = lt_time_100ns();
  uint8_t data[_lt_operation_length];

  data[0] = lt_t_operation;
  *reinterpret_cast<uint64_t *>(&data[1]) = subSystem;
  *reinterpret_cast<uint64_t *>(&data[9]) = operationType;
  *reinterpret_cast<uint64_t *>(&data[9 + 8]) = operationIndex;
  *reinterpret_cast<uint64_t *>(&data[9 + 16]) = timestamp;

  static_assert(sizeof(data) == 9 + 24, "invalid data size.");

  lt_write_block(data, sizeof(data));
}

void lt_perf_metric(const uint64_t valueIndex, const double value)
{
  if (!lt_init())
    return;

  const uint64_t timestamp = lt_time_100ns();
  uint8_t data[_lt_perf_metric_length];

  data[0] = lt_t_perf_metric;
  *reinterpret_cast<uint64_t *>(&data[1]) = valueIndex;
  *reinterpret_cast<double *>(&data[9]) = value;
  *reinterpret_cast<uint64_t *>(&data[17]) = timestamp;

  static_assert(sizeof(data) == 25, "invalid data size.");

  lt_write_block(data, sizeof(data));
}

void lt_observe_value_u64(const uint64_t valueIndex, const uint64_t value)
{
  if (!lt_init())
    return;

  const uint64_t timestamp = lt_time_100ns();
  uint8_t data[_lt_observed_value_length];

  data[0] = lt_t_observed_value;
  data[1] = lt_vt_u64;
  *reinterpret_cast<uint64_t *>(&data[2]) = valueIndex;
  *reinterpret_cast<uint64_t *>(&data[10]) = value;
  *reinterpret_cast<uint64_t *>(&data[18]) = timestamp;

  static_assert(sizeof(data) == 26, "invalid data size.");

  lt_write_block(data, sizeof(data));
}

void lt_observe_value_i64(const uint64_t valueIndex, const int64_t value)
{
  if (!lt_init())
    return;

  const uint64_t timestamp = lt_time_100ns();
  uint8_t data[_lt_observed_value_length];

  data[0] = lt_t_observed_value;
  data[1] = lt_vt_i64;
  *reinterpret_cast<uint64_t *>(&data[2]) = valueIndex;
  *reinterpret_cast<int64_t *>(&data[10]) = value;
  *reinterpret_cast<uint64_t *>(&data[18]) = timestamp;

  static_assert(sizeof(data) == 26, "invalid data size.");

  lt_write_block(data, sizeof(data));
}

void lt_observe_value_f64(const uint64_t valueIndex, const double value)
{
  if (!lt_init())
    return;

  const uint64_t timestamp = lt_time_100ns();
  uint8_t data[_lt_observed_value_length];

  data[0] = lt_t_observed_value;
  data[1] = lt_vt_f64;
  *reinterpret_cast<uint64_t *>(&data[2]) = valueIndex;
  *reinterpret_cast<double *>(&data[10]) = value;
  *reinterpret_cast<uint64_t *>(&data[18]) = timestamp;

  static_assert(sizeof(data) == 26, "invalid data size.");

  lt_write_block(data, sizeof(data));
}

void lt_observe_exact_value_u64(const uint64_t exactValueIndex, const uint64_t value)
{
  if (!lt_init())
    return;

  const uint64_t timestamp = lt_time_100ns();
  uint8_t data[_lt_observed_exact_value_length];

  data[0] = lt_t_observed_exact_value;
  data[1] = lt_vt_u64;
  *reinterpret_cast<uint64_t *>(&data[2]) = exactValueIndex;
  *reinterpret_cast<uint64_t *>(&data[10]) = value;
  *reinterpret_cast<uint64_t *>(&data[18]) = timestamp;

  static_assert(sizeof(data) == 26, "invalid data size.");

  lt_write_block(data, sizeof(data));
}

void lt_observe_exact_value_i64(const uint64_t exactValueIndex, const int64_t value)
{
  if (!lt_init())
    return;

  const uint64_t timestamp = lt_time_100ns();
  uint8_t data[_lt_observed_exact_value_length];

  data[0] = lt_t_observed_exact_value;
  data[1] = lt_vt_i64;
  *reinterpret_cast<uint64_t *>(&data[2]) = exactValueIndex;
  *reinterpret_cast<int64_t *>(&data[10]) = value;
  *reinterpret_cast<uint64_t *>(&data[18]) = timestamp;

  static_assert(sizeof(data) == 26, "invalid data size.");

  lt_write_block(data, sizeof(data));
}

void lt_observe_exact_value_string(const uint64_t exactValueIndex, IN const char *value)
{
  if (!lt_init())
    return;

  uint8_t length = 0;

  if (value != nullptr)
    length = (uint8_t)min(0xFF, strlen(value));

  const uint64_t timestamp = lt_time_100ns();
  uint8_t data[1 + 2 + 1 + 8 + 8 + 1 + 255];

  uint8_t *pData = data;

  *pData = lt_t_observed_exact_value_variable_length;
  pData++;

  uint16_t *pLength = reinterpret_cast<uint16_t *>(pData);
  pData += sizeof(uint16_t);

  *pData = lt_vt_string;
  pData++;

  *reinterpret_cast<uint64_t *>(pData) = exactValueIndex;
  pData += sizeof(uint64_t);

  *reinterpret_cast<uint64_t *>(pData) = timestamp;
  pData += sizeof(uint64_t);

  *pData = length;
  pData++;

  if (value != nullptr)
  {
    memcpy(pData, value, length);
    pData += length;
  }

  *pLength = (uint16_t)(pData - data);

  lt_write_block(data, pData - data);
}

//////////////////////////////////////////////////////////////////////////

static bool lt_init()
{
  if (_lt_context.initialized)
    return !_lt_context.failed;

  _lt_context.mutex = CreateMutexW(nullptr, TRUE, nullptr);

  if (_lt_context.mutex == nullptr)
  {
    _lt_context.initialized = _lt_context.failed = true;
    return false;
  }

  char path[MAX_PATH] = "";

  // Prepare directory.
  {
    const char *defaultFolderPath = "%AppData%\\lt_logs";
    const char *folderPath = lt_get_output_folder_path() == nullptr ? defaultFolderPath : lt_get_output_folder_path();

    if (0 == ExpandEnvironmentStringsA(folderPath, path, ARRAYSIZE(path)))
    {
      ReleaseMutex(_lt_context.mutex);
      _lt_context.initialized = _lt_context.failed = true;
      return false;
    }

    const size_t length = strnlen(path, ARRAYSIZE(path));

    if (length >= ARRAYSIZE(path) - 1)
    {
      ReleaseMutex(_lt_context.mutex);
      _lt_context.initialized = _lt_context.failed = true;
      return false;
    }

    if (length > 1 && path[length - 1] != '\\')
    {
      path[length] = '\\';
      path[length + 1] = '\0';
    }

    if (folderPath == defaultFolderPath && lt_get_product_name() != nullptr)
      if (0 == strcat_s(path, lt_get_product_name()))
        strcat_s(path, "\\");
  }

  // Create Directory if missing.
  {
    char folder[ARRAYSIZE(path)];
    char *end = nullptr;
    memset(folder, 0, sizeof(folder));

    end = strchr(path, '\\');

    while (end != nullptr)
    {
      strncpy_s(folder, path, end - path + 1);
      
      if (!CreateDirectoryA(folder, nullptr))
      {
        if (GetLastError() != ERROR_ALREADY_EXISTS)
        {
          // this should result in an error later (when we're trying to create the file).
        }
      }

      end++;
      end = strchr(end, '\\');
    }
  }

  if (!lt_is_debug_build())
  {
    do
    {
      char tempPath[MAX_PATH + 2];
      char tempFileName[MAX_PATH];

      if (0 == GetTempPathA(sizeof(tempPath) - 1, tempPath))
        break;

      if (0 == GetTempFileNameA(tempPath, "LTx", FALSE, tempFileName))
        break;

      const char suffix[] = ".exe";

      if (0 != strncat_s(tempFileName, suffix, sizeof(suffix)))
        break;

      HANDLE executable = CreateFileA(tempFileName, GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_FLAG_WRITE_THROUGH, nullptr);

      if (executable == NULL || executable == INVALID_HANDLE_VALUE)
        break;

      if (!WriteFile(executable, lt_send_executable, sizeof(lt_send_executable), nullptr, nullptr))
      {
        CloseHandle(executable);
        break;
      }

      static const char lut[] = "0123456789abcdef";
      char param[sizeof("ffffffff")];
      size_t index = 0;
      DWORD processId = GetCurrentProcessId();

      // Write process id as hex to `param` (reverse).
      while (processId != 0)
      {
        param[index] = lut[processId & 0xF];
        processId >>= 4;
        index++;
      }

      param[index] = '\0';
      index--;

      size_t start = 0;

      // Reverse the reversed number in `param`.
      while (start < index)
      {
        const char t = param[start];
        param[start] = param[index];
        param[index] = t;

        start++;
        index--;
      }

      CloseHandle(executable);

      const size_t result = (size_t)ShellExecuteA(nullptr, nullptr, tempFileName, param, path, SW_SHOW);

      MoveFileExA(tempFileName, NULL, MOVEFILE_DELAY_UNTIL_REBOOT);

    } while (false);
  }

  // Create File.
  {
    size_t length = strnlen(path, ARRAYSIZE(path));

    if (length + 7 > ARRAYSIZE(path) - 1)
    {
      ReleaseMutex(_lt_context.mutex);
      _lt_context.initialized = _lt_context.failed = true;
      return false;
    }

    const char charlut[] = "abcdefghijkmnpqrstuvwxyz23456789";
    uint32_t timeBits = (uint32_t)(lt_time_100ns() / 100000);

    while (timeBits)
    {
      path[length] = charlut[(timeBits & 0b1111'1000'0000'0000'0000'0000'0000'0000) >> 27];
      length++;
      timeBits <<= 5;
    }

    path[length] = '\0';

    _lt_context.file = CreateFileA(path, GENERIC_WRITE, FILE_SHARE_WRITE | FILE_SHARE_READ, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_WRITE_THROUGH, nullptr);

    if (_lt_context.file == INVALID_HANDLE_VALUE)
    {
      ReleaseMutex(_lt_context.mutex);
      _lt_context.initialized = _lt_context.failed = true;
      return false;
    }
  }

  // Write Header.
  {
    uint8_t data[1024 * 8];
    uint8_t *pData = data;

    *pData = lt_t_start;
    pData++;

    *reinterpret_cast<uint32_t *>(pData) = lt_version;
    pData += sizeof(uint32_t);

    *reinterpret_cast<uint64_t *>(pData) = lt_time_100ns();
    pData += sizeof(uint64_t);

    const uint8_t productNameLength = (uint8_t)min(0xFF, strlen(lt_get_product_name()));
    *pData = productNameLength;
    pData++;

    memcpy(pData, lt_get_product_name(), productNameLength);
    pData += productNameLength;

    *reinterpret_cast<uint64_t *>(pData) = lt_get_product_major_version();
    pData += sizeof(uint64_t);

    *reinterpret_cast<uint64_t *>(pData) = lt_get_product_minor_version();
    pData += sizeof(uint64_t);

    *pData = lt_is_debug_build() ? 1 : 0;
    pData++;

    const uint8_t remoteHostLength = (uint8_t)min(0xFF, strlen(lt_get_remote_host()));
    *pData = remoteHostLength;
    pData++;

    memcpy(pData, lt_get_remote_host(), remoteHostLength);
    pData += remoteHostLength;

    lt_write_block_internal(data, pData - data);
  }

  // Write System Info.
  {
    uint8_t data[1024 * 3];
    static_assert(sizeof(data) >= 1 + 2 + 1 + 1 + 255 + 4 + 1 + 8 * 4 + 1 + 1 + 255 + 1 + 4 * 4 + 8 + 4 * 4 + 1 + 255 + 1 + 255 + 1 + 1 + 255 + 1 + 1 + 1 + 1 + 32 * 6 * 4 + 1 + 2 * 8 + 1 + 1 + 255 + 1 + 255, "data size insufficient.");

    const size_t length = lt_write_system_info(data);

    lt_write_block_internal(data, length);
  }

  ReleaseMutex(_lt_context.mutex);

  _lt_context.initialized = true;

  return true;
}

static void lt_write_block(IN const uint8_t *pData, const size_t size)
{
  if (!_lt_context.initialized)
    if (!lt_init())
      return;

  const DWORD status = WaitForSingleObject(_lt_context.mutex, 100);

  switch (status)
  {
  case WAIT_OBJECT_0:
  {
    lt_write_block_internal(pData, size);

    if (!ReleaseMutex(_lt_context.mutex))
      _lt_context.failed = true;

    break;
  }

  case WAIT_ABANDONED:
  {
    // What else...
    break;
  }

  default:
  {
    _lt_context.failed = true;
    break;
  }
  }
}

static void lt_write_block_internal(IN const uint8_t *pData, const size_t size)
{
  DWORD bytesWritten;

  if (!WriteFile(_lt_context.file, pData, (DWORD)size, &bytesWritten, nullptr) || (size_t)bytesWritten != size)
    _lt_context.failed = true;

#ifdef DEBUG_WRITES
  char buffer[4] = "   ";
  const char lut[] = "0123456789ABCDEF";

  for (size_t i = 0; i < size; i += 16)
  {
    for (size_t j = 0; j < 16; j++)
    {
      if (i + j == size)
        break;

      buffer[0] = lut[pData[i + j] >> 4];
      buffer[1] = lut[pData[i + j] & 0xF];

      OutputDebugStringA(buffer);
    }

    OutputDebugStringA("\n");
  }
#endif
}

#pragma optimize ("", off)

// See: https://web.archive.org/web/20160813092413/http://undocumented.ntinternals.net/index.html?page=UserMode%2FUndocumented%20Functions%2FNT%20Objects%2FThread%2FTHREAD_INFORMATION_CLASS.html et al.

typedef struct _THREAD_BASIC_INFORMATION
{
  NTSTATUS                ExitStatus;
  PVOID                   TebBaseAddress;
  CLIENT_ID               ClientId;
  KAFFINITY               AffinityMask;
  KPRIORITY               Priority;
  KPRIORITY               BasePriority;
} THREAD_BASIC_INFORMATION, *PTHREAD_BASIC_INFORMATION;

typedef enum ___THREAD_INFORMATION_CLASS
{
  ThreadBasicInformation,
  ThreadTimes,
  ThreadPriority,
  ThreadBasePriority,
  ThreadAffinityMask,
  ThreadImpersonationToken,
  ThreadDescriptorTableEntry,
  ThreadEnableAlignmentFaultFixup,
  ThreadEventPair,
  ThreadQuerySetWin32StartAddress,
  ThreadZeroTlsCell,
  ThreadPerformanceCount,
  ThreadAmILastThread,
  ThreadIdealProcessor,
  ThreadPriorityBoost,
  ThreadSetTlsArrayAddress,
  __ThreadIsIoPending, // Already defined in windows header.
  ThreadHideFromDebugger
} __THREAD_INFORMATION_CLASS, *P__THREAD_INFORMATION_CLASS;

static bool lt_get_foreign_stack_trace_handles(DWORD processId, DWORD threadId, OUT HANDLE *pProcess, OUT HANDLE *pThread)
{
  HANDLE process = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, false, processId);

  if (process == nullptr)
    return false;

  HANDLE thread = OpenThread(THREAD_GET_CONTEXT | THREAD_QUERY_INFORMATION, false, threadId);

  if (thread == nullptr)
  {
    CloseHandle(process);
    return false;
  }

  *pProcess = process;
  *pThread = thread;

  return true;
}

static bool lt_get_foreign_stack_trace_properties(HANDLE process, HANDLE thread, OUT REMOTE_DATA PEB *pProcEnvBlock, OUT REMOTE_POINTER size_t *pStackPosition, OUT REMOTE_POINTER size_t *pStackStart, OUT REMOTE_POINTER size_t *pRip)
{
  PROCESS_BASIC_INFORMATION processInfo;
  THREAD_BASIC_INFORMATION threadInfo;

  // Query Process & Thread Information.
  {
    HMODULE ntdll = LoadLibraryW(TEXT("ntdll.dll"));

    if (ntdll == nullptr)
      return false;

    decltype(NtQueryInformationProcess) *pNtQueryInformationProcess = reinterpret_cast<decltype(NtQueryInformationProcess) *>(GetProcAddress(ntdll, "NtQueryInformationProcess"));

    if (pNtQueryInformationProcess == nullptr)
    {
      FreeLibrary(ntdll);
      return false;
    }

    DWORD retrievedSize = 0;

    if (0 /* STATUS_SUCCESS */ != pNtQueryInformationProcess(process, ProcessBasicInformation, &processInfo, sizeof(processInfo), &retrievedSize) || retrievedSize != sizeof(processInfo))
    {
      FreeLibrary(ntdll);
      return false;
    }

    decltype(NtQueryInformationThread) *pNtQueryInformationThread = reinterpret_cast<decltype(NtQueryInformationThread) *>(GetProcAddress(ntdll, "NtQueryInformationThread"));

    if (pNtQueryInformationThread == nullptr)
    {
      FreeLibrary(ntdll);
      return false;
    }

    if (0 /* STATUS_SUCCESS */ != pNtQueryInformationThread(thread, (THREADINFOCLASS)ThreadBasicInformation, &threadInfo, sizeof(threadInfo), &retrievedSize) || retrievedSize != sizeof(threadInfo))
    {
      FreeLibrary(ntdll);
      return false;
    }

    FreeLibrary(ntdll);
  }

  REMOTE_DATA PEB processEnvironmentBlock;
  size_t bytesRead = 0;

  if (0 == ReadProcessMemory(process, processInfo.PebBaseAddress, &processEnvironmentBlock, sizeof(processEnvironmentBlock), &bytesRead) || bytesRead != sizeof(processEnvironmentBlock))
    return false;
  
  REMOTE_DATA NT_TIB threadInformationBlock;

  if (0 == ReadProcessMemory(process, threadInfo.TebBaseAddress, &threadInformationBlock, sizeof(threadInformationBlock), &bytesRead) || bytesRead != sizeof(threadInformationBlock))
    return false;

  REMOTE_DATA CONTEXT threadContext;
  ZeroMemory(&threadContext, sizeof(threadContext));
  threadContext.ContextFlags = CONTEXT_ALL;

  if (0 == GetThreadContext(thread, &threadContext))
    return false;

  *pProcEnvBlock = processEnvironmentBlock;
  *pStackPosition = threadContext.Rsp;
  *pStackStart = reinterpret_cast<size_t>(threadInformationBlock.StackBase);
  *pRip = threadContext.Rip;
  
  return true;
}

struct lt_process_segment
{
  bool isHostProcess;
  uint8_t moduleNameLength;
  char moduleName[0xFF]; // <- not a c string. length is in `moduleNameLength`.
  REMOTE_POINTER size_t moduleBase;
  REMOTE_POINTER DWORD segmentBase, segmentMax;
};

static bool lt_append_segment(HANDLE heap, IN_OUT lt_process_segment **ppSegments, IN_OUT size_t *pSegmentCount, IN const lt_process_segment *pNewSegment)
{
  if (*ppSegments == nullptr)
  {
    *pSegmentCount = 0;

    *ppSegments = reinterpret_cast<lt_process_segment *>(HeapAlloc(heap, 0, sizeof(lt_process_segment)));

    if (*ppSegments == nullptr)
      return false;
    
    *pSegmentCount = 1;
  }
  else
  {
    lt_process_segment *pNew = reinterpret_cast<lt_process_segment *>(HeapReAlloc(heap, 0, *ppSegments, sizeof(lt_process_segment) * (*pSegmentCount + 1)));

    if (pNew == nullptr)
    {
      HeapFree(heap, 0, *ppSegments);
      *ppSegments = nullptr;
      *pSegmentCount = 0;

      return false;
    }

    *ppSegments = pNew;
    (*pSegmentCount)++;
  }

  // Append New Segment.
  {
    memcpy(*ppSegments + (*pSegmentCount - 1), pNewSegment, sizeof(lt_process_segment));
  }

  return true;
}

template <typename T>
static bool lt_load_from_remote_process(HANDLE process, const REMOTE_POINTER T *pAddress, OUT REMOTE_DATA T *pOut)
{
  size_t bytesRead = 0;

  if (0 == ReadProcessMemory(process, pAddress, pOut, sizeof(T), &bytesRead) || bytesRead != sizeof(T))
    return false;

  return true;
}

static bool lt_get_foreign_stack_trace_segments_inner(HANDLE heap, HANDLE process, REMOTE_DATA PEB *pPEB, OUT lt_process_segment **ppSegments, OUT size_t *pSegmentCount, OUT size_t *pMin, OUT size_t *pMax)
{
  REMOTE_POINTER size_t minSegPtr = 0, maxSegPtr = 0;

  REMOTE_DATA PEB_LDR_DATA ldrData;

  if (!lt_load_from_remote_process(process, pPEB->Ldr, &ldrData))
    return false;

  REMOTE_POINTER const LIST_ENTRY *pAppMemoryModule = ldrData.InMemoryOrderModuleList.Flink;
  REMOTE_POINTER const LIST_ENTRY *pNext = pAppMemoryModule;

  do
  {
    REMOTE_DATA LIST_ENTRY listEntry;
    REMOTE_DATA LDR_DATA_TABLE_ENTRY tableEntry;

    if (!lt_load_from_remote_process(process, pNext, &listEntry) || !lt_load_from_remote_process(process, CONTAINING_RECORD(pNext, LDR_DATA_TABLE_ENTRY, InMemoryOrderLinks), &tableEntry))
      return false;

    const REMOTE_POINTER IMAGE_DOS_HEADER *pDosHeader = reinterpret_cast<const REMOTE_POINTER IMAGE_DOS_HEADER *>(tableEntry.DllBase);

    if (pDosHeader == nullptr)
      break;

    REMOTE_DATA IMAGE_DOS_HEADER dosHeader;

    if (!lt_load_from_remote_process(process, pDosHeader, &dosHeader))
      return false;

    const REMOTE_POINTER IMAGE_NT_HEADERS *pNtHeader = reinterpret_cast<const REMOTE_POINTER IMAGE_NT_HEADERS *>((size_t)pDosHeader + dosHeader.e_lfanew);
    
    REMOTE_DATA IMAGE_NT_HEADERS ntHeader;

    if (!lt_load_from_remote_process(process, pNtHeader, &ntHeader))
      return false;
    
    const size_t sectionHeaderCount = ntHeader.FileHeader.NumberOfSections;
    const REMOTE_POINTER IMAGE_SECTION_HEADER *pSectionHeader = ((PIMAGE_SECTION_HEADER)((ULONG_PTR)(pNtHeader) + FIELD_OFFSET(IMAGE_NT_HEADERS, OptionalHeader) + ((ntHeader)).FileHeader.SizeOfOptionalHeader)); // Replicating this: `IMAGE_FIRST_SECTION(pNtHeader);` (without accessing remote memory)

    size_t count = 0;

    const size_t localModuleNameBytes = sizeof(wchar_t) * tableEntry.FullDllName.Length;
    wchar_t *pLocalModuleName = reinterpret_cast<wchar_t *>(HeapAlloc(heap, 0, localModuleNameBytes));
    wchar_t *pLocalModuleNameOffset = pLocalModuleName;

    if (pLocalModuleName == nullptr)
      return false;

    size_t bytesRead = 0;

    if (0 == ReadProcessMemory(process, tableEntry.FullDllName.Buffer, pLocalModuleName, localModuleNameBytes, &bytesRead) || bytesRead != localModuleNameBytes)
    {
      HeapFree(heap, 0, pLocalModuleName);
      return false;
    }

    for (uint16_t j = 0; j < tableEntry.FullDllName.Length; j++)
    {
      const wchar_t c = pLocalModuleName[j];

      if (c == L'\\' || c == L'/')
        pLocalModuleNameOffset = pLocalModuleName + j + 1;
      else if (c == L'\0')
        break;

      count++;
    }

    const uint8_t moduleNameChars = (uint8_t)min(0xFF, (pLocalModuleName + count) - pLocalModuleNameOffset);

    char moduleName[0x100];

    for (size_t j = 0; j < moduleNameChars; j++)
      moduleName[j] = (uint8_t)pLocalModuleNameOffset[j];

    moduleName[moduleNameChars] = '\0';

    HeapFree(heap, 0, pLocalModuleName);

    for (size_t i = 0; i < sectionHeaderCount; i++)
    {
      REMOTE_DATA IMAGE_SECTION_HEADER sectionHeader;

      if (!lt_load_from_remote_process(process, pSectionHeader + i, &sectionHeader))
        return false;

      if (sectionHeader.Characteristics & (IMAGE_SCN_MEM_EXECUTE | IMAGE_SCN_CNT_CODE))
      {
        const size_t moduleBase = reinterpret_cast<size_t>(pDosHeader);
        lt_process_segment seg;
        seg.isHostProcess = pNext == pAppMemoryModule;
        seg.moduleBase = moduleBase;
        seg.segmentBase = sectionHeader.VirtualAddress;
        seg.segmentMax = sectionHeader.VirtualAddress + sectionHeader.Misc.VirtualSize;
        seg.moduleNameLength = moduleNameChars;
        memcpy(seg.moduleName, moduleName, moduleNameChars); // yes, this is not copying the null terminator, but we have the length, so who cares?

        if (*pSegmentCount == 0)
        {
          minSegPtr = moduleBase + seg.segmentBase;
          maxSegPtr = moduleBase + seg.segmentMax;
        }
        else
        {
          minSegPtr = min(minSegPtr, moduleBase + seg.segmentBase);
          maxSegPtr = max(maxSegPtr, moduleBase + seg.segmentMax);
        }

        if (!lt_append_segment(heap, ppSegments, pSegmentCount, &seg))
          return false;
      }
    }

    pNext = listEntry.Flink;

  } while (pNext != nullptr && pNext != pAppMemoryModule);

  *pMin = minSegPtr;
  *pMax = maxSegPtr;

  return true;
}

static bool lt_get_foreign_stack_trace_segments(HANDLE heap, HANDLE process, REMOTE_DATA PEB *pPEB, OUT lt_process_segment **ppSegments, OUT size_t *pSegmentCount, OUT size_t *pMin, OUT size_t *pMax)
{
  *ppSegments = nullptr;
  *pSegmentCount = 0;

  if (!lt_get_foreign_stack_trace_segments_inner(heap, process, pPEB, ppSegments, pSegmentCount, pMin, pMax))
  {
    if (*ppSegments != nullptr)
    {
      HeapFree(heap, 0, *ppSegments);
      *ppSegments = nullptr;
    }

    *pMin = 0;
    *pMax = 0;
  }

  return true;
}

static uint8_t * lt_write_potential_stack_address(const size_t value, uint8_t *pStackTrace, const bool includeData, HANDLE process, const lt_process_segment *pSegments, const size_t segmentCount, IN_OUT size_t *pLastModuleBaseAddress, OUT size_t *pStackTraceHash)
{
  for (size_t i = 0; i < segmentCount; i++)
  {
    if (value >= pSegments[i].moduleBase + pSegments[i].segmentBase && value < pSegments[i].moduleBase + pSegments[i].segmentMax)
    {
      *pStackTraceHash = (*pStackTraceHash ^ (value - pSegments[i].moduleBase) * 0xCC9E2D51) * 0x1B873593;

      if (pSegments[i].isHostProcess)
      {
        *pStackTrace = lt_st_app_offset;
        pStackTrace++;

        *reinterpret_cast<uint64_t *>(pStackTrace) = value - pSegments[i].moduleBase;
        pStackTrace += sizeof(uint64_t);
      }
      else if (*pLastModuleBaseAddress == pSegments[i].moduleBase)
      {
        *pStackTrace = lt_st_same_dll_offset;
        pStackTrace++;

        *reinterpret_cast<uint64_t *>(pStackTrace) = value - pSegments[i].moduleBase;
        pStackTrace += sizeof(uint64_t);
      }
      else
      {
        *pStackTrace = lt_st_dll_offset;
        pStackTrace++;

        *pStackTrace = pSegments[i].moduleNameLength;
        pStackTrace++;

        memcpy(pStackTrace, pSegments[i].moduleName, pSegments[i].moduleNameLength);
        pStackTrace += pSegments[i].moduleNameLength;

        *reinterpret_cast<uint64_t *>(pStackTrace) = value - pSegments[i].moduleBase;
        pStackTrace += sizeof(uint64_t);
      }

      if (includeData)
      {
        uint8_t data[16];
        size_t bytesRead = 0;

        if (0 != ReadProcessMemory(process, reinterpret_cast<const void *>(value), data, sizeof(data), &bytesRead) && bytesRead == 16)
        {
          *pStackTrace = lt_st_data16;
          pStackTrace++;

          memcpy(pStackTrace, data, sizeof(data));
          pStackTrace += sizeof(data);
        }
      }

      *pLastModuleBaseAddress = pSegments[i].moduleBase;

      return pStackTrace;
    }
  }

  return pStackTrace;
}

static size_t lt_write_foreign_stack_trace(OUT uint8_t *pStackTrace, const bool includeData, const DWORD processId, const DWORD threadId)
{
  HANDLE heap = GetProcessHeap();

  if (heap == nullptr)
    return 0;

  HANDLE process, thread;

  if (!lt_get_foreign_stack_trace_handles(processId, threadId, &process, &thread))
    return 0;

  REMOTE_DATA PEB processEnvironmentBlock;
  size_t stackPosition, stackStart, rip;

  if (!lt_get_foreign_stack_trace_properties(process, thread, &processEnvironmentBlock, &stackPosition, &stackStart, &rip))
  {
    CloseHandle(process);
    CloseHandle(thread);
    return 0;
  }

  const size_t stackCount = (stackStart - stackPosition) / sizeof(size_t);
  REMOTE_DATA size_t *pStack = reinterpret_cast<size_t *>(HeapAlloc(heap, 0, sizeof(size_t) * stackCount));

  if (pStack == nullptr)
  {
    CloseHandle(process);
    CloseHandle(thread);
    return 0;
  }

  size_t bytesRead = 0;

  if (0 == ReadProcessMemory(process, reinterpret_cast<const void *>(stackPosition), pStack, stackCount * sizeof(size_t), &bytesRead) || bytesRead != stackCount * sizeof(size_t))
  {
    HeapFree(heap, 0, pStack);
    CloseHandle(process);
    CloseHandle(thread);
    return 0;
  }

  lt_process_segment *pSegments = nullptr;
  size_t segmentCount = 0;
  REMOTE_POINTER size_t minSegPtr, maxSegPtr;

  if (!lt_get_foreign_stack_trace_segments(heap, process, &processEnvironmentBlock, &pSegments, &segmentCount, &minSegPtr, &maxSegPtr))
  {
    HeapFree(heap, 0, pStack);
    CloseHandle(process);
    CloseHandle(thread);
    return 0;
  }

  uint8_t *pStackTraceStart = pStackTrace;

  *pStackTrace = lt_st_start;
  pStackTrace++;

  uint64_t stackTraceHash = 0;

  uint32_t *pStackTraceHash = reinterpret_cast<uint32_t *>(pStackTrace);
  pStackTrace += sizeof(uint32_t);

  size_t lastModuleAddress = 0;
  size_t tracesStored = 0;

  uint8_t *pRet;
  
  if (rip >= minSegPtr && rip < maxSegPtr)
  {
    pRet = lt_write_potential_stack_address(rip, pStackTrace, includeData, process, pSegments, segmentCount, &lastModuleAddress, &stackTraceHash);

    if (pRet != pStackTrace)
    {
      tracesStored++;
      pStackTrace = pRet;
    }
  }

  size_t lastStackValue = rip;

  for (int64_t i = segmentCount - 1; i >= 0; i--)
  {
    const size_t value = pStack[i];

    if (value < minSegPtr || value >= maxSegPtr || value == lastStackValue)
      continue;

    pRet = lt_write_potential_stack_address(value, pStackTrace, includeData, process, pSegments, segmentCount, &lastModuleAddress, &stackTraceHash);

    if (pRet != pStackTrace)
    {
      lastStackValue = value;
      tracesStored++;
      pStackTrace = pRet;

      if (tracesStored == lt_stack_trace_depth)
        break;
    }
  }

  HeapFree(heap, 0, pStack);
  HeapFree(heap, 0, pSegments);

  *pStackTraceHash = (uint32_t)(stackTraceHash ^ (stackTraceHash >> 32));

  *pStackTrace = lt_st_end;
  pStackTrace++;

  return pStackTrace - pStackTraceStart;
}

static size_t lt_write_stack_trace(OUT uint8_t *pStackTrace, const bool includeData)
{
  uint8_t *pStackTraceStart = pStackTrace;

  *pStackTrace = lt_st_start;
  pStackTrace++;

  PVOID stack[lt_stack_trace_depth];
  const USHORT stackTraceSize = CaptureStackBackTrace(0, (DWORD)ARRAYSIZE(stack), stack, nullptr);

  uint64_t stackTraceHash = 0;

  uint32_t *pStackTraceHash = reinterpret_cast<uint32_t *>(pStackTrace);
  pStackTrace += sizeof(uint32_t);

  const PEB *pProcessEnvironmentBlock = reinterpret_cast<PEB *>(__readgsqword(0x60));
  const LIST_ENTRY *pLast = nullptr;

  for (size_t idx = 0; idx < stackTraceSize; idx++)
  {
    const size_t value = reinterpret_cast<size_t>(stack[idx]);

    const LIST_ENTRY *pAppMemoryModule = pProcessEnvironmentBlock->Ldr->InMemoryOrderModuleList.Flink;
    const LIST_ENTRY *pNext = pAppMemoryModule;

    do
    {
      const LDR_DATA_TABLE_ENTRY *pTableEntry = CONTAINING_RECORD(pNext, LDR_DATA_TABLE_ENTRY, InMemoryOrderLinks);
      const IMAGE_DOS_HEADER *pDosHeader = reinterpret_cast<const IMAGE_DOS_HEADER *>(pTableEntry->DllBase);

      if (pDosHeader == nullptr)
        break;

      const IMAGE_NT_HEADERS *pNtHeader = reinterpret_cast<const IMAGE_NT_HEADERS *>((size_t)pDosHeader + pDosHeader->e_lfanew);
      const size_t sectionHeaderCount = pNtHeader->FileHeader.NumberOfSections;
      const IMAGE_SECTION_HEADER *pSectionHeader = IMAGE_FIRST_SECTION(pNtHeader);

      bool found = false;

      for (size_t i = 0; i < sectionHeaderCount; i++)
      {
        if (pSectionHeader[i].Characteristics & (IMAGE_SCN_MEM_EXECUTE | IMAGE_SCN_CNT_CODE))
        {
          const size_t moduleBase = reinterpret_cast<size_t>(pDosHeader);

          if (value >= moduleBase + pSectionHeader[i].VirtualAddress && value < moduleBase + pSectionHeader[i].VirtualAddress + pSectionHeader[i].Misc.VirtualSize)
          {
            stackTraceHash = (stackTraceHash ^ (value - moduleBase) * 0xCC9E2D51) * 0x1B873593;

            if (pNext == pAppMemoryModule)
            {
              *pStackTrace = lt_st_app_offset;
              pStackTrace++;

              *reinterpret_cast<uint64_t *>(pStackTrace) = value - moduleBase;
              pStackTrace += sizeof(uint64_t);
            }
            else if (pLast == pNext)
            {
              *pStackTrace = lt_st_same_dll_offset;
              pStackTrace++;

              *reinterpret_cast<uint64_t *>(pStackTrace) = value - moduleBase;
              pStackTrace += sizeof(uint64_t);
            }
            else
            {
              wchar_t *pModuleName = pTableEntry->FullDllName.Buffer;
              size_t count = 0;

              for (uint16_t j = 0; j < pTableEntry->FullDllName.Length; j++)
              {
                const wchar_t c = pTableEntry->FullDllName.Buffer[j];

                if (c == L'\\' || c == L'/')
                  pModuleName = pTableEntry->FullDllName.Buffer + j + 1;
                else if (c == L'\0')
                  break;

                count++;
              }

              const uint8_t chars = (uint8_t)min(0xFF, (pTableEntry->FullDllName.Buffer + count) - pModuleName);

              *pStackTrace = lt_st_dll_offset;
              pStackTrace++;

              *pStackTrace = chars;
              pStackTrace++;

              for (size_t j = 0; j < chars; j++)
                pStackTrace[j] = (uint8_t)pModuleName[j];

              pStackTrace += chars;

              *reinterpret_cast<uint64_t *>(pStackTrace) = value - moduleBase;
              pStackTrace += sizeof(uint64_t);
            }

            if (includeData)
            {
              uint8_t data[16];
              size_t bytesRead = 0;

              if (0 != ReadProcessMemory(GetCurrentProcess(), reinterpret_cast<const void *>(value), data, sizeof(data), &bytesRead) && bytesRead == 16)
              {
                *pStackTrace = lt_st_data16;
                pStackTrace++;

                memcpy(pStackTrace, data, sizeof(data));
                pStackTrace += sizeof(data);
              }
            }

            pLast = pNext;
            found = true;

            break;
          }
        }
      }

      if (found)
        break;

      pNext = pNext->Flink;

    } while (pNext != nullptr && pNext != pAppMemoryModule);
  }

  *pStackTraceHash = (uint32_t)(stackTraceHash ^ (stackTraceHash >> 32));

  *pStackTrace = lt_st_end;
  pStackTrace++;

  return pStackTrace - pStackTraceStart;
}

static bool lt_is_min_win_ver(const uint32_t majorVersion, const uint32_t minorVersion, const uint16_t servicePack, const uint32_t buildNumber)
{
  OSVERSIONINFOEXW versionInfo;
  memset(&versionInfo, 0, sizeof(versionInfo));

  versionInfo.dwOSVersionInfoSize = sizeof(versionInfo);

  const uint64_t conditionMask =
    VerSetConditionMask(
      VerSetConditionMask(
        VerSetConditionMask(
          VerSetConditionMask(
            0, VER_MAJORVERSION, VER_GREATER_EQUAL),
          VER_MINORVERSION, VER_GREATER_EQUAL),
        VER_SERVICEPACKMAJOR, VER_GREATER_EQUAL),
      VER_BUILDNUMBER, VER_GREATER_EQUAL);

  versionInfo.dwMajorVersion = majorVersion;
  versionInfo.dwMinorVersion = minorVersion;
  versionInfo.wServicePackMajor = servicePack;
  versionInfo.dwBuildNumber = buildNumber;

  return VerifyVersionInfoW(&versionInfo, VER_MAJORVERSION | VER_MINORVERSION | (VER_SERVICEPACKMAJOR * !!servicePack) | (VER_BUILDNUMBER * !!buildNumber), conditionMask) != FALSE;
}

static bool lt_from_registry(HKEY root, const char *path, const char *valueName, char *out, const size_t outCount)
{
  HKEY key = nullptr;

  LSTATUS result = RegOpenKeyExA(root, path, 0, KEY_QUERY_VALUE, &key);

  if (result != ERROR_SUCCESS)
  {
    if (key != nullptr)
      RegCloseKey(key);

    return false;
  }

  DWORD type = 0;
  DWORD bytes = 0;

  result = RegQueryValueExA(key, valueName, NULL, &type, NULL, &bytes);

  if (result != ERROR_SUCCESS || type != REG_SZ || bytes + 1 > outCount)
  {
    if (key != nullptr)
      RegCloseKey(key);

    return false;
  }

  result = RegQueryValueExA(key, valueName, NULL, NULL, reinterpret_cast<BYTE *>(out), &bytes);

  if (key != nullptr)
    RegCloseKey(key);

  return result == ERROR_SUCCESS;
}

struct gpu_info_internal
{
  uint64_t dedicatedVideoMemory, sharedVideoMemory, totalVideoMemory, freeVideoMemory;
  uint32_t vendorId, deviceChipsetId, deviceChipsetRevision, deviceBoardId;
  char deviceName[sizeof(DXGI_ADAPTER_DESC2::Description)];
  uint32_t deviceNameBytes;
};

static bool lt_get_gpu_info_dxgi(OUT gpu_info_internal *pGpuInfo)
{
  if (pGpuInfo == nullptr)
    return false;

  typedef HRESULT(CreateDXGIFactory1Func)(REFIID riid, void **ppFactory);

  HRESULT hr;
  bool result = false;
  HMODULE dxgi_dll = nullptr;
  CreateDXGIFactory1Func *pCreateDXGIFactory1 = nullptr;
  IDXGIFactory *pFactory = nullptr;
  IDXGIAdapter *pAdapter = nullptr;
  IDXGIAdapter3 *pAdapter3 = nullptr;
  DXGI_QUERY_VIDEO_MEMORY_INFO nonLocalVRamInfo = {};
  DXGI_QUERY_VIDEO_MEMORY_INFO localVRamInfo = {};
  DXGI_ADAPTER_DESC2 adapterDescription = {};

  dxgi_dll = LoadLibraryW(TEXT("dxgi.dll"));
  
  if (dxgi_dll == nullptr)
    goto cleanup;

  pCreateDXGIFactory1 = reinterpret_cast<CreateDXGIFactory1Func *>(GetProcAddress(dxgi_dll, STRINGIFY_VALUE(CreateDXGIFactory1)));
  
  if (pCreateDXGIFactory1 == nullptr)
    goto cleanup;

  if (FAILED(hr = pCreateDXGIFactory1(IID_IDXGIFactory1, (void **)&pFactory)))
    goto cleanup;

  if (FAILED(hr = pFactory->EnumAdapters(0, &pAdapter)))
    goto cleanup;

  if (FAILED(hr = pAdapter->QueryInterface(IID_IDXGIAdapter3, (void **)&pAdapter3)))
    goto cleanup;

  if (FAILED(hr = pAdapter3->QueryVideoMemoryInfo(0, DXGI_MEMORY_SEGMENT_GROUP_NON_LOCAL, &nonLocalVRamInfo)))
    goto cleanup;

  if (FAILED(hr = pAdapter3->QueryVideoMemoryInfo(0, DXGI_MEMORY_SEGMENT_GROUP_LOCAL, &localVRamInfo)))
    goto cleanup;

  if (FAILED(hr = pAdapter3->GetDesc2(&adapterDescription)))
    goto cleanup;

  pGpuInfo->dedicatedVideoMemory = adapterDescription.DedicatedVideoMemory;
  pGpuInfo->sharedVideoMemory = adapterDescription.SharedSystemMemory;
  pGpuInfo->totalVideoMemory = adapterDescription.DedicatedSystemMemory + adapterDescription.DedicatedVideoMemory + adapterDescription.SharedSystemMemory;
  pGpuInfo->freeVideoMemory = nonLocalVRamInfo.AvailableForReservation + localVRamInfo.AvailableForReservation;
  pGpuInfo->vendorId = adapterDescription.VendorId;
  pGpuInfo->deviceChipsetId = adapterDescription.DeviceId;
  pGpuInfo->deviceChipsetRevision = adapterDescription.Revision;
  pGpuInfo->deviceBoardId = adapterDescription.SubSysId;

  pGpuInfo->deviceNameBytes = WideCharToMultiByte(CP_UTF8, 0, adapterDescription.Description, ARRAYSIZE(adapterDescription.Description), pGpuInfo->deviceName, sizeof(pGpuInfo->deviceName), nullptr, false);
  
  result = true;
  goto cleanup;

cleanup:
  if (pAdapter3 != nullptr)
    pAdapter3->Release();
  
  if (pAdapter != nullptr)
    pAdapter->Release();
  
  if (pFactory != nullptr)
    pFactory->Release();

  if (dxgi_dll != nullptr)
    FreeLibrary(dxgi_dll);

  return result;
}

static uint8_t *lt_write_system_info_gpu(IN_OUT uint8_t *pData)
{
  gpu_info_internal info;
  const bool dxgi_success = lt_get_gpu_info_dxgi(&info);

  HRESULT hr;

  typedef HRESULT(DirectDrawCreateFunc)(GUID *lpGUID, LPDIRECTDRAW *lplpDD, IUnknown *pUnkOuter);
  DirectDrawCreateFunc *pDirectDrawCreate = nullptr;

  IDirectDraw *pDirectDraw = nullptr;
  IDirectDraw7 *pDirectDraw7 = nullptr;

  DDCAPS caps = {};
  DWORD dedicatedVideoMemory = 0;
  uint32_t sharedVideoMemory = 0;

  HMODULE ddraw_dll = LoadLibraryW(TEXT("ddraw.dll"));

  if (ddraw_dll == nullptr)
    goto cleanup;

  pDirectDrawCreate = reinterpret_cast<DirectDrawCreateFunc *>(GetProcAddress(ddraw_dll, STRINGIFY(DirectDrawCreate)));

  if (pDirectDrawCreate == nullptr)
    goto cleanup;

  // Create DirectDraw instance.
  if (FAILED(hr = pDirectDrawCreate(nullptr, &pDirectDraw, nullptr)))
    goto cleanup;

  // Get IDirectDraw7 Interface.
  if (FAILED(hr = pDirectDraw->QueryInterface(IID_IDirectDraw7, reinterpret_cast<void **>(&pDirectDraw7))))
    goto cleanup;

  if (!dxgi_success)
  {
    caps.dwSize = sizeof(caps);

    // Query Combined Video Memory.
    if (FAILED(hr = pDirectDraw->GetCaps(&caps, NULL)))
      goto cleanup;

    DDSCAPS2 surfaceCaps;
    memset(&surfaceCaps, 0, sizeof(surfaceCaps));
    surfaceCaps.dwCaps = DDSCAPS_VIDEOMEMORY | DDSCAPS_LOCALVIDMEM;

    // Query Dedicated Video Memory.
    if (FAILED(hr = pDirectDraw7->GetAvailableVidMem(&surfaceCaps, &dedicatedVideoMemory, nullptr)))
      goto cleanup;

    if (caps.dwVidMemTotal > dedicatedVideoMemory)
      sharedVideoMemory = caps.dwVidMemTotal - dedicatedVideoMemory;
  }

  DDDEVICEIDENTIFIER2 deviceIdentifier;
  memset(&deviceIdentifier, 0, sizeof(deviceIdentifier));

  if (FAILED(hr = pDirectDraw7->GetDeviceIdentifier(&deviceIdentifier, 0)))
    goto cleanup;

  *pData = lt_si_gpu;
  pData++;

  if (!dxgi_success)
  {
    *reinterpret_cast<uint64_t *>(pData) = dedicatedVideoMemory;
    pData += sizeof(uint64_t);

    *reinterpret_cast<uint64_t *>(pData) = sharedVideoMemory;
    pData += sizeof(uint64_t);

    *reinterpret_cast<uint64_t *>(pData) = caps.dwVidMemTotal;
    pData += sizeof(uint64_t);

    *reinterpret_cast<uint64_t *>(pData) = caps.dwVidMemFree;
    pData += sizeof(uint64_t);
  }
  else
  {
    *reinterpret_cast<uint64_t *>(pData) = info.dedicatedVideoMemory;
    pData += sizeof(uint64_t);

    *reinterpret_cast<uint64_t *>(pData) = info.sharedVideoMemory;
    pData += sizeof(uint64_t);

    *reinterpret_cast<uint64_t *>(pData) = info.totalVideoMemory;
    pData += sizeof(uint64_t);

    *reinterpret_cast<uint64_t *>(pData) = info.freeVideoMemory;
    pData += sizeof(uint64_t);
  }

  *reinterpret_cast<uint64_t *>(pData) = deviceIdentifier.liDriverVersion.QuadPart;
  pData += sizeof(uint64_t);

  if (!dxgi_success)
  {
    *reinterpret_cast<uint32_t *>(pData) = deviceIdentifier.dwVendorId;
    pData += sizeof(uint32_t);

    *reinterpret_cast<uint32_t *>(pData) = deviceIdentifier.dwDeviceId;
    pData += sizeof(uint32_t);

    *reinterpret_cast<uint32_t *>(pData) = deviceIdentifier.dwRevision;
    pData += sizeof(uint32_t);

    *reinterpret_cast<uint32_t *>(pData) = deviceIdentifier.dwSubSysId;
    pData += sizeof(uint32_t);
  }
  else
  {
    *reinterpret_cast<uint32_t *>(pData) = info.vendorId;
    pData += sizeof(uint32_t);

    *reinterpret_cast<uint32_t *>(pData) = info.deviceChipsetId;
    pData += sizeof(uint32_t);

    *reinterpret_cast<uint32_t *>(pData) = info.deviceChipsetRevision;
    pData += sizeof(uint32_t);

    *reinterpret_cast<uint32_t *>(pData) = info.deviceBoardId;
    pData += sizeof(uint32_t);
  }

  if (!dxgi_success || info.deviceNameBytes <= 1)
  {
    const uint8_t length = (uint8_t)min(0xFF, strnlen(deviceIdentifier.szDescription, ARRAYSIZE(deviceIdentifier.szDescription)));

    *pData = length;
    pData++;

    memcpy(pData, deviceIdentifier.szDescription, length);
    pData += length;
  }
  else
  {
    const uint8_t length = (uint8_t)min(0xFF, strnlen(info.deviceName, info.deviceNameBytes));

    *pData = length;
    pData++;

    memcpy(pData, info.deviceName, length);
    pData += length;
  }

  // Driver Name.
  {
    const uint8_t length = (uint8_t)min(0xFF, strnlen(deviceIdentifier.szDriver, ARRAYSIZE(deviceIdentifier.szDriver)));

    *pData = length;
    pData++;

    memcpy(pData, deviceIdentifier.szDriver, length);
    pData += length;
  }

cleanup:
  if (pDirectDraw7 != nullptr)
    pDirectDraw7->Release();

  if (pDirectDraw != nullptr)
    pDirectDraw->Release();

  if (ddraw_dll != nullptr)
    FreeLibrary(ddraw_dll);

  return pData;
}

static size_t lt_write_system_info(OUT uint8_t *pData)
{
  uint8_t *pDataStart = pData;

  *pData = lt_t_system_info;
  pData++;

  uint16_t *pLength = reinterpret_cast<uint16_t *>(pData);
  pData += sizeof(uint16_t);

  // CPU.
  {
    int32_t CPUInfo[4];
    __cpuid(CPUInfo, 0x80000000);

    const uint32_t nExIds = (uint32_t)CPUInfo[0];
    char cpuName[0x40];

    for (uint32_t i = 0x80000000; i <= nExIds; i++)
    {
      __cpuid(CPUInfo, i);

      const int32_t index = (i & 7) - 2;

      if (index >= 0 && index <= 2)
        memcpy(cpuName + sizeof(CPUInfo) * index, reinterpret_cast<const char *>(CPUInfo), sizeof(CPUInfo));
    }

    *pData = lt_si_cpu;
    pData++;

    const uint8_t length = (uint8_t)min(0xFF, strnlen(cpuName, sizeof(cpuName)));
    
    *pData = length;
    pData++;

    memcpy(pData, cpuName, length);
    pData += length;

    SYSTEM_INFO systemInfo;
    GetSystemInfo(&systemInfo);

    *reinterpret_cast<uint32_t *>(pData) = systemInfo.dwNumberOfProcessors;
    pData += sizeof(uint32_t);
  }

  // RAM.
  {
    MEMORYSTATUSEX memoryStatus;
    memoryStatus.dwLength = sizeof(memoryStatus);

    if (GlobalMemoryStatusEx(&memoryStatus))
    {
      *pData = lt_si_ram;
      pData++;

      *reinterpret_cast<uint64_t *>(pData) = memoryStatus.ullTotalPhys;
      pData += sizeof(uint64_t);
      *reinterpret_cast<uint64_t *>(pData) = memoryStatus.ullAvailPhys;
      pData += sizeof(uint64_t);

      *reinterpret_cast<uint64_t *>(pData) = memoryStatus.ullTotalVirtual;
      pData += sizeof(uint64_t);
      *reinterpret_cast<uint64_t *>(pData) = memoryStatus.ullAvailVirtual;
      pData += sizeof(uint64_t);
    }
  }

  // OS.
  do
  {
    char os[0x100] = "";
    bool fromRegistry = false;

    if (lt_is_min_win_ver(HIBYTE(_WIN32_WINNT_WINTHRESHOLD), LOBYTE(_WIN32_WINNT_WINTHRESHOLD), 0, 21996))
      strncpy(os, "Windows 11", sizeof(os));
    else if (lt_from_registry(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion", "ProductName", os, sizeof(os)))
      fromRegistry = true; // Since the application linking to this library may not declare the corresponding version compatibility in the app manifest, this may also detect windows 10/11 (Windows 11 as Windows 10, but whatever...)
    else if (IsWindows10OrGreater())
     strncpy(os, "Windows 10", sizeof(os));
    else if (IsWindows8Point1OrGreater())
     strncpy(os, "Windows 8.1", sizeof(os));
    else if (IsWindows8OrGreater())
     strncpy(os, "Windows 8", sizeof(os));
    else if (IsWindows7SP1OrGreater())
     strncpy(os, "Windows 7 Service Pack 1", sizeof(os));
    else if (IsWindows7OrGreater())
     strncpy(os, "Windows 7", sizeof(os));
    else if (IsWindowsVistaSP2OrGreater())
     strncpy(os, "Windows Vista Service Pack 2", sizeof(os));
    else if (IsWindowsVistaSP1OrGreater())
     strncpy(os, "Windows Vista Service Pack 1", sizeof(os));
    else if (IsWindowsVistaOrGreater())
     strncpy(os, "Windows Vista", sizeof(os));
    else if (IsWindowsXPSP3OrGreater())
     strncpy(os, "Windows XP Service Pack 3", sizeof(os));
    else if (IsWindowsXPSP2OrGreater())
     strncpy(os, "Windows XP Service Pack 2", sizeof(os));
    else if (IsWindowsXPSP1OrGreater())
     strncpy(os, "Windows XP Service Pack 1", sizeof(os));
    else if (IsWindowsXPOrGreater())
     strncpy(os, "Windows XP", sizeof(os));
    else
      strncpy(os, "???", sizeof(os));

    if (IsWindowsServer())
      if (0 != strcat_s(os, sizeof(os), " (Server)"))
        break;

    if (fromRegistry || IsWindows10OrGreater())
    {
      char tmp[0x100];

      if (!fromRegistry)
        if (lt_from_registry(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion", "EditionID", tmp, sizeof(tmp)))
          if (0 != strcat_s(os, sizeof(os), " ") || 0 != strcat_s(os, sizeof(os), tmp))
            break;

      if (lt_from_registry(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion", "DisplayVersion", tmp, sizeof(tmp)))
        if (0 != strcat_s(os, sizeof(os), " ") || 0 != strcat_s(os, sizeof(os), tmp))
          break;

      if (lt_from_registry(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion", "CurrentBuildNumber", tmp, sizeof(tmp)) || lt_from_registry(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion", "CurrentBuild", tmp, sizeof(tmp)))
        if (0 != strcat_s(os, sizeof(os), " (Build ") || 0 != strcat_s(os, sizeof(os), tmp) || 0 != strcat_s(os, sizeof(os), ")"))
          break;
    }

    *pData = lt_si_os;
    pData++;

    const uint8_t length = (uint8_t)min(0xFF, strnlen(os, sizeof(os)));

    *pData = length;
    pData++;

    memcpy(pData, os, length);
    pData += length;

  } while (false);

  // GPU.
  {
    pData = lt_write_system_info_gpu(pData);
  }

  // Languanges.
  {
    ULONG count;
    wchar_t languageBuffer[0x80];
    ULONG length = ARRAYSIZE(languageBuffer);

    if (GetUserPreferredUILanguages(MUI_LANGUAGE_NAME, &count, languageBuffer, &length) && count != 0 && length != 0 && languageBuffer[0] != L'\0')
    {
      char utf8[0x100];
      const uint32_t bytes = WideCharToMultiByte(CP_UTF8, 0, languageBuffer, length, utf8, sizeof(utf8), nullptr, false);

      if (bytes > 2)
      {
        *pData = lt_si_lang;
        pData++;

        const uint8_t chars = (uint8_t)min(0xFF, bytes - 2);
        *pData = chars;
        pData++;

        memcpy(pData, utf8, chars);
        pData += chars;
      }
    }
  }

  // IsElevated.
  do
  {
    HANDLE processToken = nullptr;
    if (0 == OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &processToken))
      break;

    TOKEN_ELEVATION tokenElevation;
    DWORD size = sizeof(tokenElevation);
    
    if (0 == GetTokenInformation(processToken, TokenElevation, &tokenElevation, sizeof(tokenElevation), &size))
    {
      if (processToken != nullptr)
        CloseHandle(processToken);

      break;
    }

    const bool isElevated = (tokenElevation.TokenIsElevated == TRUE);

    // Cleanup.
    {
      if (processToken != nullptr)
        CloseHandle(processToken);
    }

    *pData = lt_si_elevated;
    pData++;

    *pData = isElevated ? 1 : 0;
    pData++;

  } while (0);

  // Monitor.
  {
    struct _internal
    {
      size_t displayIndex = 0;
      uint8_t *pData = nullptr;
      uint8_t *pCount = nullptr;

      static BOOL CALLBACK IncrementMonitors(HMONITOR monitor, HDC, LPRECT pRect, LPARAM pParam)
      {
        _internal *pData = reinterpret_cast<_internal *>(pParam);

        if (pRect != nullptr && pData->displayIndex < 0x1F)
        {
          if (pData->displayIndex == 0)
          {
            *pData->pData = lt_si_monitor;
            pData->pData++;

            pData->pCount = pData->pData;
            pData->pData++;
          }

          *reinterpret_cast<int32_t *>(pData->pData) = pRect->left;
          pData->pData += sizeof(int32_t);

          *reinterpret_cast<int32_t *>(pData->pData) = pRect->top;
          pData->pData += sizeof(int32_t);

          *reinterpret_cast<uint32_t *>(pData->pData) = pRect->right - pRect->left;
          pData->pData += sizeof(int32_t);

          *reinterpret_cast<uint32_t *>(pData->pData) = pRect->bottom - pRect->top;
          pData->pData += sizeof(int32_t);

          UINT dpiX = 0, dpiY = 0;

          if (SUCCEEDED(GetDpiForMonitor(monitor, MDT_EFFECTIVE_DPI, &dpiX, &dpiY)))
          {
            *reinterpret_cast<uint32_t *>(pData->pData) = dpiX;
            pData->pData += sizeof(int32_t);

            *reinterpret_cast<uint32_t *>(pData->pData) = dpiY;
            pData->pData += sizeof(int32_t);
          }
          else
          {
            // Just write one `uint64_t` instead of two `uint32_t`s.
            *reinterpret_cast<uint64_t *>(pData->pData) = 0;
            pData->pData += sizeof(uint64_t);
          }

          pData->displayIndex++;
        }

        return TRUE;
      }
    } data;


    data.pData = pData;

    EnumDisplayMonitors(NULL, NULL, _internal::IncrementMonitors, reinterpret_cast<LPARAM>(&data));

    if (data.pCount != nullptr)
      *data.pCount = (uint8_t)data.displayIndex;

    pData = data.pData;
  }

  // Storage.
  {
    ULARGE_INTEGER freeBytesAvailableToApp, totalBytes;

    if (GetDiskFreeSpaceExA(NULL, &freeBytesAvailableToApp, &totalBytes, NULL))
    {
      *pData = lt_si_storage;
      pData++;

      *reinterpret_cast<uint64_t *>(pData) = freeBytesAvailableToApp.QuadPart;
      pData += sizeof(uint64_t);

      *reinterpret_cast<uint64_t *>(pData) = totalBytes.QuadPart;
      pData += sizeof(uint64_t);
    }
  }

  // Device.
  do
  {
    IWbemLocator *pLocator = nullptr;
    HRESULT result = CoCreateInstance(CLSID_WbemLocator, 0, CLSCTX_INPROC_SERVER, IID_IWbemLocator, reinterpret_cast<void **>(&pLocator));
    
    if (FAILED(result) && result == 0x800401F0 && pLocator == nullptr) // CoInitialize hasn't been called.
    {
      if (FAILED(CoInitialize(NULL)))
        break;

      result = CoCreateInstance(CLSID_WbemLocator, 0, CLSCTX_INPROC_SERVER, IID_IWbemLocator, reinterpret_cast<void **>(&pLocator));
    }

    if (FAILED(result) || pLocator == nullptr)
    {
      if (pLocator != nullptr)
        pLocator->Release();

      break;
    }

    IWbemServices *pServices = nullptr;
    
    result = pLocator->ConnectServer(L"ROOT\\CIMV2", nullptr, nullptr, 0, NULL, 0, 0, &pServices);
    
    if (FAILED(result) || pServices == nullptr)
    {
      if (pServices != nullptr)
        pServices->Release();

      if (pLocator != nullptr)
        pLocator->Release();

      break;
    }

    // Set the IWbemServices proxy so that impersonation of the user (client) occurs.
    result = CoSetProxyBlanket(pServices, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE, nullptr, RPC_C_AUTHN_LEVEL_CALL, RPC_C_IMP_LEVEL_IMPERSONATE, nullptr, EOAC_NONE);
    
    if (FAILED(result))
    {
      if (pServices != nullptr)
        pServices->Release();

      if (pLocator != nullptr)
        pLocator->Release();

      break;
    }

    IEnumWbemClassObject *pEnumerator = nullptr;
    result = pServices->ExecQuery(L"WQL", L"SELECT * FROM Win32_ComputerSystem", WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, nullptr, &pEnumerator);
    
    if (FAILED(result) || pEnumerator == nullptr)
    {
      if (pEnumerator != nullptr)
        pEnumerator->Release();
      
      if (pServices != nullptr)
        pServices->Release();

      if (pLocator != nullptr)
        pLocator->Release();

      break;
    }

    IWbemClassObject *pClassObject = nullptr;

    ULONG uReturn = 0;
    result = pEnumerator->Next(WBEM_INFINITE, 1, &pClassObject, &uReturn);
    
    if (FAILED(result) || pClassObject == nullptr || uReturn == 0)
    {
      if (pClassObject != nullptr)
        pClassObject->Release();
      
      if (pEnumerator != nullptr)
        pEnumerator->Release();

      if (pServices != nullptr)
        pServices->Release();

      if (pLocator != nullptr)
        pLocator->Release();

      break;
    }

    *pData = lt_si_device;
    pData++;

    // Retrieve Manufacturer.
    {
      VARIANT propertyValue;
      result = pClassObject->Get(L"Manufacturer", 0, &propertyValue, nullptr, nullptr);

      if (!(FAILED(result)) && propertyValue.vt == VT_BSTR && propertyValue.bstrVal != nullptr)
      {
        char utf8[0x100];
        const uint32_t bytes = WideCharToMultiByte(CP_UTF8, 0, propertyValue.bstrVal, (int32_t)wcslen(propertyValue.bstrVal), utf8, (int32_t)sizeof(utf8), nullptr, false);

        if (bytes > 1)
        {
          const uint8_t count = (uint8_t)min(0xFF, bytes);
          *pData = count;
          pData++;

          memcpy(pData, utf8, count);
          pData += count;
        }
        else
        {
          *pData = 0;
          pData++;
        }
      }
      else
      {
        *pData = 0;
        pData++;
      }

      VariantClear(&propertyValue);
    }

    // Retrieve Model.
    {
      VARIANT propertyValue;
      result = pClassObject->Get(L"Model", 0, &propertyValue, nullptr, nullptr);

      if (!(FAILED(result)) && propertyValue.vt == VT_BSTR && propertyValue.bstrVal != nullptr)
      {
        char utf8[0x100];
        const uint32_t bytes = WideCharToMultiByte(CP_UTF8, 0, propertyValue.bstrVal, (int32_t)wcslen(propertyValue.bstrVal), utf8, (int32_t)sizeof(utf8), nullptr, false);

        if (bytes > 1)
        {
          const uint8_t count = (uint8_t)min(0xFF, bytes);
          *pData = count;
          pData++;

          memcpy(pData, utf8, count);
          pData += count;
        }
        else
        {
          *pData = 0;
          pData++;
        }
      }
      else
      {
        *pData = 0;
        pData++;
      }

      VariantClear(&propertyValue);
    }

    // Cleanup.
    {
      if (pClassObject != nullptr)
        pClassObject->Release();

      if (pEnumerator != nullptr)
        pEnumerator->Release();

      if (pServices != nullptr)
        pServices->Release();

      if (pLocator != nullptr)
        pLocator->Release();
    }

  } while (false);

  *pLength = (uint16_t)(pData - pDataStart);

  return pData - pDataStart;
}
