#include "lt_common.h"

#include <windows.h>
#include <debugapi.h>
#include <psapi.h>
#include <atlutil.h>
#include <dia2.h>
#include <diacreate.h>
#include <cvconst.h>
#include <initguid.h>

#include <memory>

#include <inttypes.h>

extern "C"
{
#define ZYCORE_STATIC_DEFINE
#include <Zydis/Zydis.h>
}

//////////////////////////////////////////////////////////////////////////

#ifdef _DEBUG
 #define DBG_BREAK() __debugbreak()
#else
 #define DBG_BREAK()
#endif

#define FATAL(x, ...) do { printf(x "\n", __VA_ARGS__); DBG_BREAK(); ExitProcess((UINT)-1); } while (0)
#define FATAL_IF(conditional, x, ...) do { if (conditional) { FATAL(x, __VA_ARGS__); } } while (0)

//////////////////////////////////////////////////////////////////////////

void print_string_as_json(const char *string);
void print_string_as_json(const wchar_t *string);

//////////////////////////////////////////////////////////////////////////

int32_t main(void)
{
  const wchar_t *commandLine = GetCommandLineW();

  int32_t argc = 0;
  wchar_t **pArgv = CommandLineToArgvW(commandLine, &argc);
  FATAL_IF(argc != 3, "Invalid Parameter.\nUsage: [<Input StackTrace File> | :<Base64 Encoded StackTrace>] <Application PDB>");

  const wchar_t *inputFileName = pArgv[1];
  const wchar_t *pdbFileName = pArgv[2];

  CComPtr<IDiaSession> pdbSession;
  size_t stackTracesBytes = 0;
  uint8_t *pStackTraces = nullptr;
  bool disasmAvailable = false;
  ZydisDecoder disasmDecoder;
  ZydisFormatter disasmFormatter;

  // Attempt to read PDB.
  {
    CComPtr<IDiaDataSource> pdbSource;
    HRESULT hr;

    FATAL_IF(FAILED(hr = CoInitialize(nullptr)), "Failed to Initialize. Aborting.");

    if (FAILED(hr = CoCreateInstance(CLSID_DiaSource, nullptr, CLSCTX_INPROC_SERVER, __uuidof(IDiaDataSource), (void **)&pdbSource)) || pdbSource == nullptr)
    {
      // See https://github.com/baldurk/renderdoc/blob/c3ca732ab9d49d710922ce0243e7bd7b404415d1/renderdoc/os/win32/win32_callstack.cpp

      wchar_t *dllPath = L"msdia140.dll";
      HMODULE msdia140dll = LoadLibraryW(dllPath);
      FATAL_IF(msdia140dll == nullptr, "Failed to load '%ws'. Aborting.", dllPath);

      typedef decltype(&DllGetClassObject) DllGetClassObjectFunc;
      DllGetClassObjectFunc pDllGetClassObject = reinterpret_cast<DllGetClassObjectFunc>(GetProcAddress(msdia140dll, "DllGetClassObject"));
      FATAL_IF(pDllGetClassObject == nullptr, "Failed to load symbol from '%ws'. Aborting.", dllPath);

      CComPtr<IClassFactory> classFactory;
      FATAL_IF(FAILED(hr = pDllGetClassObject(__uuidof(DiaSource), IID_IClassFactory, reinterpret_cast<void **>(&classFactory))) || classFactory == nullptr, "Failed to retrieve COM Class Factory. Aborting.");
      
      FATAL_IF(FAILED(hr = classFactory->CreateInstance(nullptr, __uuidof(IDiaDataSource), reinterpret_cast<void **>(&pdbSource))) || pdbSource == nullptr, "Failed to create debug source from class factory. Aborting.");
    }

    if (FAILED(pdbSource->loadDataFromPdb(pdbFileName)))
      FATAL("Failed to load pdb. ('%ws')", pdbFileName);

    FATAL_IF(FAILED(hr = pdbSource->openSession(&pdbSession)), "Failed to Open Session.");
  }

  // Is the input a file name or base64?
  if (inputFileName[0] != TEXT(':'))
  {
    // Read StackTrace File.
    HANDLE file = CreateFileW(inputFileName, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, 0, nullptr);
    FATAL_IF(file == INVALID_HANDLE_VALUE, "Failed to open stack trace file. (0x%" PRIX32 ")", GetLastError());

    LARGE_INTEGER fileSize;
    FATAL_IF(!GetFileSizeEx(file, &fileSize), "Failed to retrieve file size.");

    stackTracesBytes = (size_t)fileSize.QuadPart;
    pStackTraces = reinterpret_cast<uint8_t *>(malloc(stackTracesBytes));

    FATAL_IF(pStackTraces == nullptr, "Failed to allocate memory.");

    size_t bytesRemaining = stackTracesBytes;
    size_t offset = 0;

    while (bytesRemaining > 0)
    {
      const DWORD bytesToRead = (DWORD)min(bytesRemaining, MAXDWORD);
      DWORD bytesRead = 0;

      FATAL_IF(!ReadFile(file, pStackTraces + offset, bytesToRead, &bytesRead, nullptr), "Failed to read stack trace file (0x%" PRIX32 ")", GetLastError());
      FATAL_IF(bytesRead == 0, "Failed to read from stack trace file.");

      offset += bytesRead;
      bytesRemaining -= bytesRead;
    }

    CloseHandle(file);
  }
  else
  {
    // Decode from Base64.
    inputFileName++; // Remove ':'.

    size_t length = wcslen(inputFileName);

    // Remove Padding.
    while (length > 0 && inputFileName[length - 1] == TEXT('='))
      length--;

    FATAL_IF(length % 4 == 1, "Base64 length invalid.");

    stackTracesBytes = length / 4 * 3 + (size_t)max((int64_t)0, ((int64_t)(length % 4) - 1));
    pStackTraces = reinterpret_cast<uint8_t *>(malloc(stackTracesBytes));

    FATAL_IF(pStackTraces == nullptr, "Failed to allocate memory.");

    // Which codepoint maps to which hextet?
    // `0xFF` represents invalid codepoints (including the padding codepoint '=', which is handled separately)
    const uint8_t lut[0x80] =
    {
      0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
      0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
      0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x3E, 0xFF, 0xFF, 0xFF, 0x3F,
      0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
      0xFF, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E,
      0x0F, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
      0xFF, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28,
      0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F, 0x30, 0x31, 0x32, 0x33, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
    };

    size_t i = 0;
    uint8_t *pNext = pStackTraces;
    uint32_t decoded[4];

    for (; i < min(length, length - 3); i += 4)
    {
      FATAL_IF((uint16_t)inputFileName[i] >= 0x80 || (uint16_t)inputFileName[i + 1] >= 0x80 || (uint16_t)inputFileName[i + 2] >= 0x80 || (uint16_t)inputFileName[i + 3] >= 0x80, "Invalid Base64 String.");

      decoded[0] = lut[(uint8_t)inputFileName[i + 0]];
      decoded[1] = lut[(uint8_t)inputFileName[i + 1]];
      decoded[2] = lut[(uint8_t)inputFileName[i + 2]];
      decoded[3] = lut[(uint8_t)inputFileName[i + 3]];

      FATAL_IF(decoded[0] == 0xFF || decoded[1] == 0xFF || decoded[2] == 0xFF || decoded[3] == 0xFF, "Invalid Base64 String.");

      const uint32_t out = (decoded[0] << 3 * 6) | (decoded[1] << 12) | (decoded[2] << 6) | decoded[3];

      pNext[0] = (uint8_t)(out >> 0x10);
      pNext[1] = (uint8_t)(out >> 0x8);
      pNext[2] = (uint8_t)out;

      pNext += 3;
    }

    if (i < length)
    {
      ZeroMemory(decoded, sizeof(decoded));
      const size_t remainingSize = length - i;

      for (size_t j = 0; j < remainingSize; j++)
      {
        FATAL_IF((uint16_t)inputFileName[i + j] >= 0x80, "Invalid Base64 String.");

        decoded[j] = lut[(uint8_t)inputFileName[i + j]];

        FATAL_IF(decoded[j] == 0xFF, "Invalid Base64 String.");
      }

      const uint32_t out = (decoded[0] << 3 * 6) | (decoded[1] << 12) | (decoded[2] << 6) | decoded[3];

      *pNext = (uint8_t)(out >> 0x10);
      pNext++;

      if (remainingSize > 2)
      {
        *pNext = (uint8_t)(out >> 0x8);
        pNext++;

        if (remainingSize > 3)
        {
          *pNext = (uint8_t)out;
          pNext++;
        }
      }
    }

    FATAL_IF(pNext != pStackTraces + stackTracesBytes, "Invalid Base64 String.");
  }

  // Init Disassembler.
  {
    disasmAvailable = ZYAN_SUCCESS(ZydisDecoderInit(&disasmDecoder, ZYDIS_MACHINE_MODE_LONG_64, ZYDIS_ADDRESS_WIDTH_64)) &&
      ZYAN_SUCCESS(ZydisFormatterInit(&disasmFormatter, ZYDIS_FORMATTER_STYLE_INTEL)) &&
      ZYAN_SUCCESS(ZydisFormatterSetProperty(&disasmFormatter, ZYDIS_FORMATTER_PROP_FORCE_SEGMENT, ZYAN_TRUE)) &&
      ZYAN_SUCCESS(ZydisFormatterSetProperty(&disasmFormatter, ZYDIS_FORMATTER_PROP_FORCE_SIZE, ZYAN_TRUE));
  }

  // Print Stacktraces.
  {
    uint8_t *pStackTracesEnd = pStackTraces + stackTracesBytes;

    fputs("{\"stacktraces\":[\n", stdout);
    bool isFirstTrace = true;

    while (pStackTraces < pStackTracesEnd)
    {
      FATAL_IF(*pStackTraces != lt_st_start, "Invalid Stack Trace Header.");
      pStackTraces += sizeof(uint8_t);

      const uint32_t hash = *reinterpret_cast<const uint32_t *>(pStackTraces);
      pStackTraces += sizeof(uint32_t);

      if (!isFirstTrace)
        fputs(",\n", stdout);

      isFirstTrace = false;

      printf("{\"hash\":%" PRIu32 ",\"stack\":[\n", hash);

      char moduleName[256] = "<Invalid Module>";
      uint64_t offset = 0;
      bool isFirstElement = true;

      while (true)
      {
        const uint8_t type = *pStackTraces;
        pStackTraces++;

        bool end = false;

        switch (type)
        {
        default:
        {
          end = true;
          FATAL("Invalid Stack Trace Type.");
          break;
        }

        case lt_st_end:
        {
          end = true;
          fputs("]}", stdout);
          break;
        }

        case lt_st_app_offset:
        {
          if (!isFirstElement)
            fputs(",\n", stdout);

          isFirstElement = false;

          offset = *reinterpret_cast<const uint64_t *>(pStackTraces);
          pStackTraces += sizeof(uint64_t);

          printf("{\"offset\":\"0x%" PRIX64 "\"", offset);

          CComPtr<IDiaEnumLineNumbers> lineNumEnum;
          CComPtr<IDiaSymbol> symbol;

          wchar_t *symbolName = nullptr;

          if (SUCCEEDED(pdbSession->findSymbolByVA(offset, SymTagFunction, &symbol)) && symbol != nullptr && ((SUCCEEDED(symbol->get_undecoratedName(&symbolName)) && symbolName != nullptr) || SUCCEEDED(symbol->get_name(&symbolName))))
          {
            fputs(",\"function\":", stdout);
            print_string_as_json(symbolName);
          }

          if (symbolName != nullptr)
            SysFreeString(symbolName);

          if (SUCCEEDED(pdbSession->findLinesByVA(offset, 1, &lineNumEnum)))
          {
            CComPtr<IDiaLineNumber> lineNumber;
            ULONG fetched;

            if (SUCCEEDED(lineNumEnum->Next(1, &lineNumber, &fetched)) && fetched != 0)
            {
              DWORD sourceFileId;

              if (SUCCEEDED(lineNumber->get_sourceFileId(&sourceFileId)))
              {
                CComPtr<IDiaSourceFile> sourceFile;

                if (SUCCEEDED(lineNumber->get_sourceFile(&sourceFile)))
                {
                  wchar_t *sourceFileName = nullptr;

                  if (SUCCEEDED(sourceFile->get_fileName(&sourceFileName)))
                  {
                    fputs(",\"file\":", stdout);
                    print_string_as_json(sourceFileName);

                    DWORD line = 0;

                    if (SUCCEEDED(lineNumber->get_lineNumber(&line)))
                      printf(",\"line\":%" PRIu32 "", line);
                  }

                  if (sourceFileName != nullptr)
                    SysFreeString(sourceFileName);
                }
              }
            }
          }

          fputs("}", stdout);

          break;
        }

        case lt_st_same_dll_offset:
        {
          offset = *reinterpret_cast<const uint64_t *>(pStackTraces);
          pStackTraces += sizeof(uint64_t);

          if (!isFirstElement)
            fputs(",\n", stdout);

          isFirstElement = false;

          printf("{\"offset\":\"0x%" PRIX64 "\",\"module\":", offset);
          print_string_as_json(moduleName);
          fputs("}", stdout);

          break;
        }

        case lt_st_dll_offset:
        {
          const uint8_t count = *pStackTraces;
          pStackTraces++;

          memcpy(moduleName, pStackTraces, count);
          pStackTraces += count;
          moduleName[(size_t)count] = '\0';

          offset = *reinterpret_cast<const uint64_t *>(pStackTraces);
          pStackTraces += sizeof(uint64_t);

          if (!isFirstElement)
            fputs(",\n", stdout);

          isFirstElement = false;

          printf("{\"offset\":\"0x%" PRIX64 "\",\"module\":", offset);
          print_string_as_json(moduleName);
          fputs("}", stdout);

          break;
        }

        case lt_st_data16:
        {
          if (!isFirstElement)
            fputs(",\n", stdout);

          isFirstElement = false;

          fputs("{\"is_disasm\":1,\"data\":\"", stdout);

          for (size_t i = 0; i < 16; i++)
            printf("%02" PRIX8 "", pStackTraces[i]);

          fputs("\"", stdout);

          if (disasmAvailable)
          {
            ZydisDecodedInstruction instruction;
            char disasmBuffer[256] = {};

            if (ZYAN_SUCCESS(ZydisDecoderDecodeBuffer(&disasmDecoder, pStackTraces, 16, &instruction)) && ZYAN_SUCCESS(ZydisFormatterFormatInstruction(&disasmFormatter, &instruction, disasmBuffer, sizeof(disasmBuffer), offset)))
            {
              fputs(",\"disasm\":", stdout);
              print_string_as_json(disasmBuffer);
              fputs("}", stdout);
            }
            else
            {
              fputs(",\"error\":\"disasm_failed\"}", stdout);
            }
          }
          else
          {
            fputs(",\"error\":\"disasm_not_available\"}", stdout);
          }

          pStackTraces += 16;

          break;
        }
        }

        if (end)
          break;
      }
    }

    fputs("]}", stdout);
  }

  return 0;
}

//////////////////////////////////////////////////////////////////////////

void print_string_as_json(const char *string)
{
  size_t i = 0;

  if (string == nullptr)
  {
    fputs("null", stdout);
    return;
  }

  fputs("\"", stdout);

  char singleChar[2];
  singleChar[1] = '\0';

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
  size_t i = 0;

  if (string == nullptr)
  {
    fputs("null", stdout);
    return;
  }

  fputs("\"", stdout);

  wchar_t singleChar[2];
  singleChar[1] = L'\0';

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
