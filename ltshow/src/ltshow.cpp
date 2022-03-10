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
 #define DBG_BREAK()
#endif

#define FATAL(x, ...) do { printf(x "\n", __VA_ARGS__); DBG_BREAK(); ExitProcess((UINT)-1); } while (0)
#define FATAL_IF(conditional, x, ...) do { if (conditional) { FATAL(x, __VA_ARGS__); } } while (0)
#define RECOVERABLE_ERROR(x, ...) FATAL(x, __VA_ARGS__)
#define RECOVERABLE_ERROR_IF(conditional, x, ...) do { if (conditional) { RECOVERABLE_ERROR(x, __VA_ARGS__); } } while (0)

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

  std::vector<const uint8_t *> headers;
  uint32_t ltVersion = 0;
  uint64_t startTimestamp = 0;
  uint64_t majorVersion = 0;
  uint64_t minorVersion = 0;
  bool isDebugBuild = false;
  char productName[0x100];
  char remoteHost[0x100];

  // Read Section Headers.
  {
    const uint8_t *pRead = pData;
    const uint8_t *pEnd = pData + fileSize;

    FATAL_IF(*pRead != lt_t_start, "Invalid Header");
    pRead++;

    ltVersion = *reinterpret_cast<const uint32_t *>(pRead);
    pRead += sizeof(uint32_t);

    startTimestamp = *reinterpret_cast<const uint64_t *>(pRead);
    pRead += sizeof(uint64_t);

    const uint8_t productNameLength = *pRead;
    pRead++;

    memcpy(productName, pRead, productNameLength);
    pRead += productNameLength;
    productName[productNameLength] = '\0';

    majorVersion = *reinterpret_cast<const uint64_t *>(pRead);
    pRead += sizeof(uint64_t);

    minorVersion = *reinterpret_cast<const uint64_t *>(pRead);
    pRead += sizeof(uint64_t);

    isDebugBuild = *pRead != 0;
    pRead++;

    const uint8_t remoteHostLength = *pRead;
    pRead++;

    memcpy(remoteHost, pRead, remoteHostLength);
    pRead += remoteHostLength;
    remoteHost[remoteHostLength] = '\0';

    while (pRead < pEnd)
    {
      const uint8_t type = *pRead;

      if (type < __lt_t_fixed_length)
      {
        headers.push_back(pRead);

        const uint16_t size = *reinterpret_cast<const uint16_t *>(pRead + 1);
        FATAL_IF(size < 3, "Invalid Section Size");

        pRead += size;
      }
      else
      {
        switch (type)
        {
        case lt_t_state:
          headers.push_back(pRead);
          pRead += _lt_state_length;
          break;

        case lt_t_operation:
          headers.push_back(pRead);
          pRead += _lt_operation_length;
          break;

        case lt_t_observed_value:
          headers.push_back(pRead);
          pRead += _lt_observed_value_length;
          break;

        case lt_t_observed_exact_value:
          headers.push_back(pRead);
          pRead += _lt_observed_exact_value_length;
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

    bool isFirstTrace = true;

    for (const uint8_t *pRead : headers)
    {
      const uint8_t type = *pRead;
      pRead++;

      if (!isFirstTrace)
        fputs(",\n", stdout);

      isFirstTrace = false;

      printf("{\"type\":%" PRIu8 "", type);

      switch (type)
      {
      case lt_t_state:
      {
        const uint64_t *pRead64 = reinterpret_cast<const uint64_t *>(pRead);

        printf(",\"subsystem\":\"0x%" PRIX64 "\"", pRead64[0]);
        printf(",\"stateIndex\":\"0x%" PRIX64 "\"", pRead64[1]);
        printf(",\"subStateIndex\":\"0x%" PRIX64 "\"", pRead64[2]);
        printf(",\"timestamp\":\"0x%" PRIX64 "\"", pRead64[3]);

        break;
      }

      case lt_t_operation:
      {
        const uint64_t *pRead64 = reinterpret_cast<const uint64_t *>(pRead);

        printf(",\"subsystem\":\"0x%" PRIX64 "\"", pRead64[0]);
        printf(",\"operationType\":\"0x%" PRIX64 "\"", pRead64[1]);
        printf(",\"operationIndex\":\"0x%" PRIX64 "\"", pRead64[2]);
        printf(",\"timestamp\":\"0x%" PRIX64 "\"", pRead64[3]);

        break;
      }

      case lt_t_observed_value:
      {
        const uint8_t dataType = *pRead;
        pRead++;

        printf(",\"dataType\":%" PRIu8 "", dataType);

        const uint64_t *pRead64 = reinterpret_cast<const uint64_t *>(pRead);

        printf(",\"valueIndex\":\"0x%" PRIX64 "\"", pRead64[0]);
        printf(",\"timestamp\":\"0x%" PRIX64 "\"", pRead64[2]);

        switch (dataType)
        {
        case lt_vt_u64:
          printf(",\"value\":\"%" PRIu64 "\"", pRead64[1]);
          break;
        
        case lt_vt_i64:
          printf(",\"value\":\"%" PRIi64 "\"", *reinterpret_cast<const int64_t *>(&pRead64[1]));
          break;

        case lt_vt_f64:
          printf(",\"value\":%e", *reinterpret_cast<const double_t *>(&pRead64[1]));
          break;

        default:
          RECOVERABLE_ERROR("Invalid data type.");
          break;
        }

        break;
      }

      case lt_t_observed_exact_value:
      {
        const uint8_t dataType = *pRead;
        pRead++;

        printf(",\"dataType\":%" PRIu8 "", dataType);

        const uint64_t *pRead64 = reinterpret_cast<const uint64_t *>(pRead);

        printf(",\"valueIndex\":\"0x%" PRIX64 "\"", pRead64[0]);
        printf(",\"timestamp\":\"0x%" PRIX64 "\"", pRead64[2]);

        switch (dataType)
        {
        case lt_vt_u64:
          printf(",\"value\":\"%" PRIu64 "\"", pRead64[1]);
          break;

        case lt_vt_i64:
          printf(",\"value\":\"%" PRIi64 "\"", *reinterpret_cast<const int64_t *>(&pRead64[1]));
          break;

        case lt_vt_f64:
          printf(",\"value\":%e", *reinterpret_cast<const double_t *>(&pRead64[1]));
          break;

        default:
          RECOVERABLE_ERROR("Invalid data type.");
          break;
        }

        break;
      }

      case lt_t_crash:
      {
        const uint16_t size = *reinterpret_cast<const uint16_t *>(pRead);
        pRead += sizeof(uint16_t);
        (void)size;

        printf(",\"timestamp\":\"0x%" PRIX64 "\"", *reinterpret_cast<const uint64_t *>(pRead));
        pRead += sizeof(uint64_t);
        
        printf(",\"errorCode\":\"0x%" PRIX64 "\"", *reinterpret_cast<const uint64_t *>(pRead));
        pRead += sizeof(uint64_t);

        const uint8_t descriptionLength = *pRead;
        pRead++;

        if (descriptionLength > 0)
        {
          memcpy(svalue, pRead, descriptionLength);
          pRead += descriptionLength;
          svalue[descriptionLength] = '\0';

          fputs(",\"description\":", stdout);
          print_string_as_json(svalue);
        }

        const uint16_t stackTraceLength = *reinterpret_cast<const uint16_t *>(pRead);
        pRead += sizeof(uint16_t);

        if (stackTraceLength > 0)
        {
          fputs(",\"stacktrace\":\"", stdout);

          for (uint16_t i = 0; i < stackTraceLength; i++)
            printf("%02" PRIX8, pRead[i]);

          fputs("\"", stdout);
        }

        break;
      }

      case lt_t_error:
      case lt_t_warning:
      {
        const uint16_t size = *reinterpret_cast<const uint16_t *>(pRead);
        pRead += sizeof(uint16_t);
        (void)size;

        printf(",\"timestamp\":\"0x%" PRIX64 "\"", *reinterpret_cast<const uint64_t *>(pRead));
        pRead += sizeof(uint64_t);

        printf(",\"subSystem\":\"0x%" PRIX64 "\"", *reinterpret_cast<const uint64_t *>(pRead));
        pRead += sizeof(uint64_t);

        printf(",\"errorCode\":\"0x%" PRIX64 "\"", *reinterpret_cast<const uint64_t *>(pRead));
        pRead += sizeof(uint64_t);

        const uint8_t descriptionLength = *pRead;
        pRead++;

        if (descriptionLength > 0)
        {
          memcpy(svalue, pRead, descriptionLength);
          pRead += descriptionLength;
          svalue[descriptionLength] = '\0';

          fputs(",\"description\":", stdout);
          print_string_as_json(svalue);
        }

        const uint16_t stackTraceLength = *reinterpret_cast<const uint16_t *>(pRead);
        pRead += sizeof(uint16_t);

        if (stackTraceLength > 0)
        {
          fputs(",\"stacktrace\":", stdout);
          print_bytes_as_base64string(pRead, stackTraceLength);
          pRead += stackTraceLength;
        }

        break;
      }

      case lt_t_log:
      {
        const uint16_t size = *reinterpret_cast<const uint16_t *>(pRead);
        pRead += sizeof(uint16_t);
        (void)size;

        printf(",\"timestamp\":\"0x%" PRIX64 "\"", *reinterpret_cast<const uint64_t *>(pRead));
        pRead += sizeof(uint64_t);

        printf(",\"subSystem\":\"0x%" PRIX64 "\"", *reinterpret_cast<const uint64_t *>(pRead));
        pRead += sizeof(uint64_t);

        const uint8_t descriptionLength = *pRead;
        pRead++;

        if (descriptionLength > 0)
        {
          memcpy(svalue, pRead, descriptionLength);
          pRead += descriptionLength;
          svalue[descriptionLength] = '\0';

          fputs(",\"description\":", stdout);
          print_string_as_json(svalue);
        }

        break;
      }

      case lt_t_perf_data:
      {
        const uint16_t size = *reinterpret_cast<const uint16_t *>(pRead);
        pRead += sizeof(uint16_t);
        (void)size;

        printf(",\"timestamp\":\"0x%" PRIX64 "\"", *reinterpret_cast<const uint64_t *>(pRead));
        pRead += sizeof(uint64_t);

        printf(",\"subSystem\":\"0x%" PRIX64 "\"", *reinterpret_cast<const uint64_t *>(pRead));
        pRead += sizeof(uint64_t);

        const uint8_t count = *pRead;
        pRead++;

        if (count > 0)
        {
          fputs(",\"data\":[", stdout);

          for (uint8_t i = 0; i < count; i++)
          {
            if (i > 0)
              fputs(",", stdout);

            printf("%e", reinterpret_cast<const double *>(pRead)[i]);
          }

          fputs("]", stdout);
        }

        break;
      }

      case lt_t_observed_exact_value_variable_length:
      {
        const uint16_t size = *reinterpret_cast<const uint16_t *>(pRead);
        pRead += sizeof(uint16_t);
        (void)size;

        const uint8_t dataType = *pRead;
        pRead++;

        printf(",\"exactValueIndex\":\"0x%" PRIX64 "\"", *reinterpret_cast<const uint64_t *>(pRead));
        pRead += sizeof(uint64_t);

        printf(",\"timestamp\":\"0x%" PRIX64 "\"", *reinterpret_cast<const uint64_t *>(pRead));
        pRead += sizeof(uint64_t);

        RECOVERABLE_ERROR_IF(dataType != lt_vt_string, "Invalid Value Type.");

        if (dataType == lt_vt_string)
        {
          const uint8_t length = *pRead;
          pRead++;

          if (length > 0)
          {
            memcpy(svalue, pRead, length);
            pRead += length;
            svalue[length] = '\0';

            fputs(",\"value\":", stdout);
            print_string_as_json(svalue);
          }
        }

        break;
      }

      case lt_t_hardware_info:
        break;

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
