#include "lt_common.h"

#include <windows.h>
#include <debugapi.h>
#include <psapi.h>
#include <atlutil.h>
#include <dia2.h>
#include <diacreate.h>
#include <cvconst.h>
#include <initguid.h>

#include <vector>
#include <deque>
#include <algorithm>
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

int32_t main(void)
{
  const wchar_t *commandLine = GetCommandLineW();

  int32_t argc = 0;
  wchar_t **pArgv = CommandLineToArgvW(commandLine, &argc);
  FATAL_IF(argc != 3, "Invalid Parameter.\nUsage: <Input StackTrace File> <Application PDB>");

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
      FATAL("Failed to load pdb.");

    FATAL_IF(FAILED(hr = pdbSource->openSession(&pdbSession)), "Failed to Open Session.");
  }

  // Read StackTrace File.
  {
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

      FATAL_IF(!ReadFile(file, pStackTraces + offset, bytesToRead, &bytesRead, nullptr), "Failed to read stack trace file.");
      FATAL_IF(bytesRead == 0, "Failed to read from stack trace file.");

      offset += bytesRead;
      bytesRemaining -= bytesRead;
    }

    CloseHandle(file);
  }

  // Init Disassembler.
  {
    disasmAvailable = ZYAN_SUCCESS(ZydisDecoderInit(&disasmDecoder, ZYDIS_MACHINE_MODE_LONG_64, ZYDIS_ADDRESS_WIDTH_64)) &&
      ZYAN_SUCCESS(ZydisFormatterInit(&disasmFormatter, ZYDIS_FORMATTER_STYLE_INTEL)) &&
      ZYAN_SUCCESS(ZydisFormatterSetProperty(&disasmFormatter, ZYDIS_FORMATTER_PROP_FORCE_SEGMENT, ZYAN_TRUE)) &&
      ZYAN_SUCCESS(ZydisFormatterSetProperty(&disasmFormatter, ZYDIS_FORMATTER_PROP_FORCE_SIZE, ZYAN_TRUE));
  }

  uint8_t *pStackTracesEnd = pStackTraces + stackTracesBytes;

  while (pStackTraces < pStackTracesEnd)
  {
    FATAL_IF(*pStackTraces != lt_st_start, "Invalid Stack Trace Header.");
    pStackTraces += sizeof(uint8_t) + sizeof(DWORD);

    printf("Stacktrace (x %" PRIu64 "):\n", *reinterpret_cast<const uint64_t *>(pStackTraces));
    pStackTraces += sizeof(uint64_t);

    char moduleName[256] = "<Invalid Module>";
    uint64_t offset = 0;

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
        puts("");
        break;
      }

      case lt_st_app_offset:
      {
        offset = *reinterpret_cast<const uint64_t *>(pStackTraces);
        pStackTraces += sizeof(uint64_t);

        CComPtr<IDiaEnumLineNumbers> lineNumEnum;
        CComPtr<IDiaSymbol> symbol;

        wchar_t *symbolName = nullptr;

        if (SUCCEEDED(pdbSession->findSymbolByVA(offset, SymTagFunction, &symbol)) && symbol != nullptr && ((SUCCEEDED(symbol->get_undecoratedName(&symbolName)) && symbolName != nullptr) || SUCCEEDED(symbol->get_name(&symbolName))))
          printf("Function '%ws' @ 0x%" PRIX64 "", symbolName, offset);
        else
          printf("Application Binary @ 0x%" PRIX64 "", offset);

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
                  DWORD line = 0;

                  if (SUCCEEDED(lineNumber->get_lineNumber(&line)))
                    printf(" ('%ws' Line %" PRIu32 ")", sourceFileName, line);
                  else
                    printf(" ('%ws')", sourceFileName);
                }

                if (sourceFileName != nullptr)
                  SysFreeString(sourceFileName);
              }
            }
          }
        }

        puts("");

        break;
      }

      case lt_st_same_dll_offset:
      {
        offset = *reinterpret_cast<const uint64_t *>(pStackTraces);
        pStackTraces += sizeof(uint64_t);

        printf("Module '%s' @ 0x%" PRIX64 "\n", moduleName, offset);

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

        printf("Module '%s' @ 0x%" PRIX64 "\n", moduleName, offset);

        break;
      }

      case lt_st_data16:
      {
        printf("\t\t");

        for (size_t i = 0; i < 16; i++)
          printf("%02" PRIX8 " ", pStackTraces[i]);
        
        if (disasmAvailable)
        {
          ZydisDecodedInstruction instruction;
          char disasmBuffer[256] = {};

          if (ZYAN_SUCCESS(ZydisDecoderDecodeBuffer(&disasmDecoder, pStackTraces, 16, &instruction)) && ZYAN_SUCCESS(ZydisFormatterFormatInstruction(&disasmFormatter, &instruction, disasmBuffer, sizeof(disasmBuffer), offset)))
            printf("\t%s\n", disasmBuffer);
          else
            puts("\t<disasm_failed>");
        }
        else
        {
          puts("\t<disasm_not_available>");
        }

        pStackTraces += 16;

        break;
      }
      }

      if (end)
        break;
    }
  }

  // Cleanup.
  {
    
  }

  return 0;
}
