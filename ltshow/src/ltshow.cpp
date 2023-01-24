#include "lt_common.h"

#include <stdio.h>
#include <inttypes.h>

#include <memory>
#include <vector>

#include <windows.h>

//////////////////////////////////////////////////////////////////////////

#ifdef _DEBUG
 #define DBG_BREAK() __debugbreak()
#else
 #define DBG_BREAK() do { } while (0)
#endif

#define FATAL(x, ...) do { printf(x "\n", __VA_ARGS__); DBG_BREAK(); ExitProcess((UINT)-1); } while (0)
#define FATAL_IF(conditional, x, ...) do { if (conditional) { FATAL(x, __VA_ARGS__); } } while (0)
#define RECOVERABLE_ERROR(x, ...) FATAL(x, __VA_ARGS__)
#define RECOVERABLE_ERROR_IF(conditional, x, ...) do { if (conditional) { RECOVERABLE_ERROR(x, __VA_ARGS__); } } while (0)

#define PRINT_X64(prefix, name, stream) do { uint64_t v = 0; FATAL_IF(!stream.read(&v), "Insufficient data stream"); printf(prefix "\"" name "\":\"0x%" PRIX64 "\"", v); } while (0);
#define PRINT_U64(prefix, name, stream) do { uint64_t v = 0; FATAL_IF(!stream.read(&v), "Insufficient data stream"); printf(prefix "\"" name "\":\"%" PRIu64 "\"", v); } while (0);
#define PRINT_I64(prefix, name, stream) do { int64_t v = 0; FATAL_IF(!stream.read(&v), "Insufficient data stream"); printf(prefix "\"" name "\":\"%" PRIi64 "\"", v); } while (0);
#define PRINT_F64(prefix, name, stream) do { double v = 0; FATAL_IF(!stream.read(&v), "Insufficient data stream"); printf(prefix "\"" name "\":%e", v); } while (0);
#define PRINT_F32_2(prefix, name, stream) do { float_t v0 = 0, v1 = 0; FATAL_IF(!stream.read(&v0), "Insufficient data stream"); FATAL_IF(!stream.read(&v1), "Insufficient data stream"); printf(prefix "\"" name "\":[%e,%e]", v0, v1); } while (0);
#define PRINT_U32(prefix, name, stream) do { uint32_t v = 0; FATAL_IF(!stream.read(&v), "Insufficient data stream"); printf(prefix "\"" name "\":%" PRIu32 "", v); } while (0);
#define PRINT_I32(prefix, name, stream) do { int32_t v = 0; FATAL_IF(!stream.read(&v), "Insufficient data stream"); printf(prefix "\"" name "\":%" PRIi32 "", v); } while (0);
#define PRINT_U8(prefix, name, stream) do { uint8_t v = 0; FATAL_IF(!stream.read(&v), "Insufficient data stream"); printf(prefix "\"" name "\":%" PRIu8 "", v); } while (0);
#define PRINT_BOOL(prefix, name, stream) do { uint8_t v = 0; FATAL_IF(!stream.read(&v), "Insufficient data stream"); v = !!v; printf(prefix "\"" name "\":%" PRIu8 "", v); } while (0);

#define PRINT_STRING(prefix, name, stream) do { \
  char buffer256[0x100]; \
  uint8_t length = 0; \
  FATAL_IF(!stream.read(&length), "Insufficient data stream"); \
  if (length > 0) \
  { FATAL_IF(!stream.read(buffer256, length), "Insufficient data stream"); \
    buffer256[length] = '\0'; \
    fputs(prefix "\"" name "\":", stdout); \
    print_string_as_json(buffer256); \
  } } while (0);

//////////////////////////////////////////////////////////////////////////

void print_string_as_json(const char *string);
void print_string_as_json(const wchar_t *string);
void print_bytes_as_base64string(const uint8_t *pData, const size_t size);

//////////////////////////////////////////////////////////////////////////

int32_t main(void)
{
  const wchar_t *commandLine = GetCommandLineW();

  int32_t argc = 0;
  wchar_t **pArgv = CommandLineToArgvW(commandLine, &argc);
  FATAL_IF(argc != 2, "Invalid Parameter.\nUsage: <LT Log File>");

  const wchar_t *inputFileName = pArgv[1];
  size_t fileSize = 0;
  uint8_t *pData = nullptr;

  // Read StackTrace File.
  {
    HANDLE file = CreateFileW(inputFileName, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, 0, nullptr);
    FATAL_IF(file == INVALID_HANDLE_VALUE, "Failed to open log file. (0x%" PRIX32 ")", GetLastError());

    LARGE_INTEGER _fileSize;
    FATAL_IF(!GetFileSizeEx(file, &_fileSize), "Failed to retrieve file size.");

    fileSize = (size_t)_fileSize.QuadPart;
    pData = reinterpret_cast<uint8_t *>(malloc(fileSize));

    FATAL_IF(pData == nullptr, "Failed to allocate memory.");

    size_t bytesRemaining = fileSize;
    size_t offset = 0;

    while (bytesRemaining > 0)
    {
      const DWORD bytesToRead = (DWORD)min(bytesRemaining, MAXDWORD);
      DWORD bytesRead = 0;

      FATAL_IF(!ReadFile(file, pData + offset, bytesToRead, &bytesRead, nullptr), "Failed to read log file. (0x%" PRIX32 ")", GetLastError());
      FATAL_IF(bytesRead == 0, "Failed to read from log file.");

      offset += bytesRead;
      bytesRemaining -= bytesRead;
    }

    CloseHandle(file);
  }

  std::vector<PointerWithSize> headers;
  uint32_t ltVersion = 0;
  uint64_t startTimestamp = 0;
  uint64_t majorVersion = 0;
  uint64_t minorVersion = 0;
  bool isDebugBuild = false;
  char productName[0x100];
  char remoteHost[0x100];

  // Read Section Headers.
  {
    ByteStream stream(pData, fileSize);

    uint8_t u8 = (uint8_t)~lt_t_start;
    FATAL_IF(!stream.read(&u8), "Insufficient Data");
    FATAL_IF(u8 != lt_t_start, "Invalid Header");

    FATAL_IF(!stream.read(&ltVersion), "Insufficient Data");
    FATAL_IF(!stream.read(&startTimestamp), "Insufficient Data");

    uint8_t productNameLength = 0;
    FATAL_IF(!stream.read(&productNameLength), "Insufficient Data");
    FATAL_IF(!stream.read(productName, productNameLength), "Insufficient Data");
    productName[productNameLength] = '\0';

    FATAL_IF(!stream.read(&majorVersion), "Insufficient Data");
    FATAL_IF(!stream.read(&minorVersion), "Insufficient Data");

    FATAL_IF(!stream.read(&u8), "Insufficient Data");
    isDebugBuild = (u8 != 0);

    uint8_t remoteHostLength = 0;
    FATAL_IF(!stream.read(&remoteHostLength), "Insufficient Data");
    FATAL_IF(!stream.read(remoteHost, remoteHostLength), "Insufficient Data");
    remoteHost[remoteHostLength] = '\0';

    while (stream.sizeRemaining > 0)
    {
      const uint8_t *pPtr = stream.pData;

      uint8_t type = 0;
      FATAL_IF(!stream.read(&type), "Insufficient Data");
      FATAL_IF(type == 0, "New Start Detected.");

      if (type < __lt_t_fixed_length)
      {
        uint16_t size;
        FATAL_IF(!stream.read(&size), "Insufficient Data");
        FATAL_IF(size < 3, "Invalid Section Size");
        FATAL_IF(!stream.read<uint8_t>(nullptr, size - 3), "Insufficient Data");

        headers.emplace_back(pPtr, size);
      }
      else
      {
        switch (type)
        {
        case lt_t_state:
          headers.emplace_back(pPtr, _lt_state_length);
          FATAL_IF(!stream.read<uint8_t>(nullptr, _lt_state_length - 1), "Insufficient Data");
          break;

        case lt_t_operation:
          headers.emplace_back(pPtr, _lt_operation_length);
          FATAL_IF(!stream.read<uint8_t>(nullptr, _lt_operation_length - 1), "Insufficient Data");
          break;

        case lt_t_observed_value:
          headers.emplace_back(pPtr, _lt_observed_value_length);
          FATAL_IF(!stream.read<uint8_t>(nullptr, _lt_observed_value_length - 1), "Insufficient Data");
          break;

        case lt_t_observed_exact_value:
          headers.emplace_back(pPtr, _lt_observed_exact_value_length);
          FATAL_IF(!stream.read<uint8_t>(nullptr, _lt_observed_exact_value_length - 1), "Insufficient Data");
          break;

        case lt_t_perf_metric:
          headers.emplace_back(pPtr, _lt_perf_metric_length);
          FATAL_IF(!stream.read<uint8_t>(nullptr, _lt_perf_metric_length - 1), "Insufficient Data");
          break;

        default:
          FATAL("Unsupported Type (0x%02" PRIX8 ")", type);
          break;
        }
      }
    }
  }

  // Display Sections.
  {
    char svalue[0x100];

    fputs("{\"productName\":", stdout);
    print_string_as_json(productName);
    fputs(",\"remoteHost\":", stdout);
    print_string_as_json(remoteHost);
    printf(",\"majorVersion\":\"0x%" PRIX64 "\",\"minorVersion\":\"0x%" PRIX64 "\",\"isDebugBuild\":%" PRIu8 ",\"timestamp\":\"0x%" PRIX64 "\",\"contents\":[", majorVersion, minorVersion, isDebugBuild, startTimestamp);

    bool isFirstItem = true;

    for (const auto &_item : headers)
    {
      ByteStream stream(_item);

      uint8_t type = 0;
      FATAL_IF(!stream.read(&type), "Insufficient Data");

      if (!isFirstItem)
        fputs(",\n", stdout);

      isFirstItem = false;

      printf("{\"type\":%" PRIu8 "", type);

      switch (type)
      {
      case lt_t_state:
      {
        PRINT_X64(",", "subsystem", stream);
        PRINT_X64(",", "stateIndex", stream);
        PRINT_X64(",", "subStateIndex", stream);
        PRINT_X64(",", "timestamp", stream);

        break;
      }

      case lt_t_operation:
      {
        PRINT_X64(",", "subsystem", stream);
        PRINT_X64(",", "operationType", stream);
        PRINT_X64(",", "operationIndex", stream);
        PRINT_X64(",", "timestamp", stream);

        break;
      }

      case lt_t_perf_metric:
      {
        PRINT_X64(",", "valueIndex", stream);
        PRINT_F64(",", "value", stream);
        PRINT_X64(",", "timestamp", stream);

        break;
      }

      case lt_t_observed_value:
      {
        uint8_t dataType = 0;
        FATAL_IF(!stream.read(&dataType), "Insufficient data steam.");
        printf(",\"dataType\":%" PRIu8 "", dataType);

        PRINT_X64(",", "valueIndex", stream);

        switch (dataType)
        {
        case lt_vt_u64:
          PRINT_U64(",", "value", stream);
          break;
        
        case lt_vt_i64:
          PRINT_I64(",", "value", stream);
          break;

        case lt_vt_f64:
          PRINT_F64(",", "value", stream);
          break;

        case lt_vt_f32_2:
          PRINT_F32_2(",", "value", stream);
          break;

        default:
          RECOVERABLE_ERROR("Invalid data type.");
          break;
        }

        PRINT_X64(",", "timestamp", stream);

        break;
      }

      case lt_t_observed_exact_value:
      {
        uint8_t dataType = 0;
        FATAL_IF(!stream.read(&dataType), "Insufficient data steam.");
        printf(",\"dataType\":%" PRIu8 "", dataType);

        PRINT_X64(",", "valueIndex", stream);

        switch (dataType)
        {
        case lt_vt_u64:
          PRINT_U64(",", "value", stream);
          break;

        case lt_vt_i64:
          PRINT_I64(",", "value", stream);
          break;

        case lt_vt_f64:
          PRINT_F64(",", "value", stream);
          break;

        default:
          RECOVERABLE_ERROR("Invalid data type.");
          break;
        }

        PRINT_X64(",", "timestamp", stream);

        break;
      }

      case lt_t_crash:
      {
        uint16_t size = 0;
        FATAL_IF(!stream.read(&size), "Insufficient data stream.");

        PRINT_X64(",", "timestamp", stream);
        PRINT_X64(",", "errorCode", stream);
        PRINT_STRING(",", "description", stream);

        uint16_t stackTraceLength = 0;
        FATAL_IF(!stream.read(&stackTraceLength), "Insufficient data stream.");

        if (stackTraceLength > 0)
        {
          const uint8_t *pStackTraceData = stream.pData;
          FATAL_IF(!stream.read<uint8_t >(nullptr, stackTraceLength), "Insufficient data stream.");
          fputs(",\"stacktrace\":", stdout);
          print_bytes_as_base64string(pStackTraceData, stackTraceLength);
        }

        break;
      }

      case lt_t_error:
      case lt_t_warning:
      {
        uint16_t size = 0;
        FATAL_IF(!stream.read(&size), "Insufficient data stream.");

        PRINT_X64(",", "timestamp", stream);
        PRINT_X64(",", "subSystem", stream);
        PRINT_X64(",", "errorCode", stream);
        PRINT_STRING(",", "description", stream);

        uint16_t stackTraceLength = 0;
        FATAL_IF(!stream.read(&stackTraceLength), "Insufficient data stream.");

        if (stackTraceLength > 0)
        {
          const uint8_t *pStackTraceData = stream.pData;
          FATAL_IF(!stream.read<uint8_t >(nullptr, stackTraceLength), "Insufficient data stream.");
          fputs(",\"stacktrace\":", stdout);
          print_bytes_as_base64string(pStackTraceData, stackTraceLength);
        }

        break;
      }

      case lt_t_log:
      {
        uint16_t size = 0;
        FATAL_IF(!stream.read(&size), "Insufficient data stream.");

        PRINT_X64(",", "timestamp", stream);
        PRINT_X64(",", "subSystem", stream);
        PRINT_STRING(",", "description", stream);

        break;
      }

      case lt_t_perf_data:
      {
        uint16_t size = 0;
        FATAL_IF(!stream.read(&size), "Insufficient data stream.");

        PRINT_X64(",", "timestamp", stream);
        PRINT_X64(",", "subSystem", stream);

        uint8_t count = 0;
        FATAL_IF(!stream.read(&count), "Insufficient data stream.");
        
        if (count > 0)
        {
          const double *pPerfData = reinterpret_cast<const double *>(stream.pData);
          FATAL_IF(!stream.read<double>(nullptr, count), "Insufficient data stream.");

          fputs(",\"data\":[", stdout);

          for (uint8_t i = 0; i < count; i++)
          {
            if (i > 0)
              fputs(",", stdout);

            printf("%e", pPerfData[i]);
          }

          fputs("]", stdout);
        }

        break;
      }

      case lt_t_observed_exact_value_variable_length:
      {
        uint16_t size = 0;
        FATAL_IF(!stream.read(&size), "Insufficient data stream.");

        uint8_t dataType = 0;
        FATAL_IF(!stream.read(&dataType), "Insufficient data stream.");

        PRINT_X64(",", "exactValueIndex", stream);
        PRINT_X64(",", "timestamp", stream);

        RECOVERABLE_ERROR_IF(dataType != lt_vt_string, "Invalid Value Type.");

        if (dataType == lt_vt_string)
          PRINT_STRING(",", "value", stream);

        break;
      }

      case lt_t_system_info:
      {
        fputs(",\"info\":[", stdout);

        uint16_t size = 0;
        FATAL_IF(!stream.read(&size), "Insufficient data stream.");

        bool isFirstInfo = true;

        while (stream.sizeRemaining > 0)
        {
          if (!isFirstInfo)
            fputs(",\n", stdout);

          isFirstInfo = false;

          uint8_t info = 0;
          FATAL_IF(!stream.read(&info), "Insufficient data stream.");

          printf("{\"type\":%" PRIu8 "", info);

          switch (info)
          {
          case lt_si_cpu:
          {
            PRINT_STRING(",", "description", stream);
            PRINT_U32(",", "processorCount", stream);

            break;
          }

          case lt_si_ram:
          {
            PRINT_X64(",", "totalPhysical", stream);
            PRINT_X64(",", "availablePhysical", stream);
            PRINT_X64(",", "totalVirtual", stream);
            PRINT_X64(",", "availableVirtual", stream);

            break;
          }

          case lt_si_os:
          {
            PRINT_STRING(",", "os", stream);

            break;
          }

          case lt_si_gpu:
          {
            PRINT_U64(",", "dedicatedVRAM", stream);
            PRINT_U64(",", "sharedVRAM", stream);
            PRINT_U64(",", "totalVRAM", stream);
            PRINT_U64(",", "freeVRAM", stream);
            PRINT_X64(",", "driverVersion", stream);
            PRINT_U32(",", "vendorId", stream);
            PRINT_U32(",", "deviceId", stream);
            PRINT_U32(",", "revisionId", stream);
            PRINT_U32(",", "boardId", stream);
            PRINT_STRING(",", "deviceDescription", stream);
            PRINT_STRING(",", "driverId", stream);

            break;
          }

          case lt_si_lang:
          {
            uint8_t length = 0;
            FATAL_IF(!stream.read(&length), "Insufficient data stream.");
            FATAL_IF(!stream.read(svalue, length), "Insufficient data stream.");
            svalue[length] = '\0';

            size_t offset = 0;

            fputs(",\"languages\":[", stdout);

            while (offset < length)
            {
              if (offset > 0)
                fputs(",", stdout);
              
              print_string_as_json(svalue + offset);

              offset += strlen(svalue + offset) + 1;
            }
            
            fputs("]", stdout);

            break;
          }

          case lt_si_elevated:
          {
            PRINT_BOOL(",", "isElevated", stream);

            break;
          }

          case lt_si_monitor:
          {
            uint8_t count = 0;
            FATAL_IF(!stream.read(&count), "Insufficient data stream.");

            fputs(",\"monitors\":[", stdout);

            for (uint8_t i = 0; i < count; i++)
            {
              if (i > 0)
                fputs(",\n", stdout);

              fputs("{", stdout);

              PRINT_I32("", "posX", stream);
              PRINT_I32(",", "posY", stream);
              PRINT_U32(",", "sizeX", stream);
              PRINT_U32(",", "sizeY", stream);
              PRINT_U32(",", "dpiX", stream);
              PRINT_U32(",", "dpiY", stream);

              fputs("}", stdout);
            }

            fputs("]", stdout);

            break;
          }

          case lt_si_storage:
          {
            PRINT_X64(",", "freeBytesAvailable", stream);
            PRINT_X64(",", "totalBytes", stream);

            break;
          }

          case lt_si_device:
          {
            PRINT_STRING(",", "manufacturer", stream);
            PRINT_STRING(",", "model", stream);

            break;
          }

          case lt_si_storage_quality:
          {
            PRINT_X64(",", "totalStorageBytes", stream);
            PRINT_X64(",", "ssdStorageBytes", stream);

            break;
          }

          case lt_si_networking:
          {
            PRINT_X64(",", "downLinkSpeed", stream);
            PRINT_X64(",", "upLinkSpeed", stream);
            PRINT_BOOL(",", "isWireless", stream);
            PRINT_U32(",", "identifier", stream);

            break;
          }

          default:
          {
            stream.sizeRemaining = 0;
            RECOVERABLE_ERROR("Unsupported Info (0x%02" PRIX8 ")", info);
            break;
          }
          }

          fputs("}", stdout);
        }

        fputs("]", stdout);

        break;
      }

      default:
      {
        RECOVERABLE_ERROR("Unsupported Type (0x%02" PRIX8 ")", type);
        break;
      }
      }

      fputs("}", stdout);
    }

    fputs("]}", stdout);
  }

  return 0;
}

//////////////////////////////////////////////////////////////////////////

void print_string_as_json(const char *string)
{

  if (string == nullptr)
  {
    fputs("null", stdout);
    return;
  }

  fputs("\"", stdout);

  char singleChar[2];
  singleChar[1] = '\0';
 
  size_t i = 0;

  while (true)
  {
    singleChar[0] = string[i++];

    if (singleChar[0] == '\0')
      break;

    switch (singleChar[0])
    {
    case '\\': fputs("\\\\", stdout); break;
    case '\"': fputs("\\\"", stdout); break;
    case '\b': fputs("\\b", stdout); break;
    case '\f': fputs("\\f", stdout); break;
    case '\n': fputs("\\n", stdout); break;
    case '\r': fputs("\\r", stdout); break;
    case '\t': fputs("\\t", stdout); break;
    default:
    {
      if (singleChar[0] > 0 && singleChar[0] <= 0x1F)
        printf("\\u00%02" PRIX8, singleChar[0]);
      else
        fputs(singleChar, stdout);

      break;
    }
    }
  }

  fputs("\"", stdout);
}

void print_string_as_json(const wchar_t *string)
{
  if (string == nullptr)
  {
    fputs("null", stdout);
    return;
  }

  fputs("\"", stdout);

  wchar_t singleChar[2];
  singleChar[1] = L'\0';

  size_t i = 0;

  while (true)
  {
    singleChar[0] = string[i++];

    if (singleChar[0] == L'\0')
      break;

    switch (singleChar[0])
    {
    case L'\\': fputs("\\\\", stdout); break;
    case L'\"': fputs("\\\"", stdout); break;
    case L'\b': fputs("\\b", stdout); break;
    case L'\f': fputs("\\f", stdout); break;
    case L'\n': fputs("\\n", stdout); break;
    case L'\r': fputs("\\r", stdout); break;
    case L'\t': fputs("\\t", stdout); break;
    default:
    {
      if (singleChar[0] > 0 && singleChar[0] <= 0x1F)
        printf("\\u00%02" PRIX8, singleChar[0]);
      else
        fputws(singleChar, stdout);

      break;
    }
    }
  }

  fputs("\"", stdout);
}

void print_bytes_as_base64string(const uint8_t *pData, const size_t size)
{
  if (pData == nullptr)
  {
    fputs("null", stdout);
    return;
  }

  fputs("\"", stdout);

  // Encode to Base64.
  {
    const char lut[] =
    {
      'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
      'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
      'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
      'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
      'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
      'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
      'w', 'x', 'y', 'z', '0', '1', '2', '3',
      '4', '5', '6', '7', '8', '9', '+', '/'
    };

    char next[5];
    next[4] = '\0';

    for (size_t i = 0; i < size; i += 3)
    {
      uint32_t data[3];

      data[0] = (i + 0) < size ? pData[i] : 0;
      data[1] = (i + 1) < size ? pData[i + 1] : 0;
      data[2] = (i + 2) < size ? pData[i + 2] : 0;

      const uint32_t triple = (data[0] << 0x10) + (data[1] << 0x08) + data[2];

      next[0] = lut[(triple >> 18) & 0x3F];
      next[1] = lut[(triple >> 12) & 0x3F];
      next[2] = lut[(triple >> 6) & 0x3F];
      next[3] = lut[triple & 0x3F];

      fputs(next, stdout);
    }
  }

  // Add Padding.
  {
    const size_t bytesMod3 = size % 3;

    if (bytesMod3 != 0)
      for (size_t i = 0; i < 3 - bytesMod3; i++)
        fputs("=", stdout);
  }

  fputs("\"", stdout);
}
