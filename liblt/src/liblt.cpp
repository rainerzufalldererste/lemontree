#include "liblt.h"

#include "lt_common.h"

#include <windows.h>
#include <winternl.h>
#include <shlwapi.h>

#pragma comment(lib, "Shlwapi.lib")

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

void lt_crash_ex(IN HANDLE process, IN HANDLE thread, const uint64_t errorCode, IN OPTIONAL const char *description);

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

    if (length > 0 && path[length - 1] != '\\')
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

    end = strchr(path, L'\\');

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
      end = strchr(end, L'\\');
    }
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
      path[length] = charlut[timeBits & 0b11111];
      length++;
      timeBits >>= 5;
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

  *reinterpret_cast<uint64_t *>(pStackTrace) = 1; // quantity of how often this error occured (may be overwritten later).
  pStackTrace += sizeof(uint64_t);

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

  *pStackTraceHash = (uint32_t)(stackTraceHash | (stackTraceHash >> 32));

  *pStackTrace = lt_st_end;
  pStackTrace++;

  return pStackTrace - pStackTraceStart;
}
