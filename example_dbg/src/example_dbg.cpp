#include <stdio.h>
#include <inttypes.h>

#include <windows.h>

#include <shlwapi.h>
#pragma comment(lib, "Shlwapi.lib")

#include "liblt.h"

extern const char *g_lt_product_name = "lt_example";
extern const uint64_t g_lt_product_major_version = 0x1020304050607080;
extern const uint64_t g_lt_product_minor_version = 0xFEDCBA9876543210;
extern const bool g_lt_is_debug_build = false;
extern const char *g_lt_remote_host = "localhost";
extern const char *g_lt_folder_path = nullptr;
extern const bool g_lt_crash_stack_trace_include_data = true;
extern const bool g_lt_error_include_stack_trace = true;
extern const bool g_lt_warn_include_stack_trace = false;

#define FATAL(s, ...) do { printf("ERROR: " s "\n", __VA_ARGS__); ExitProcess((UINT)-1); } while (0)

int32_t main(const int32_t argc, const char **pArgv)
{
  (void)argc;
  (void)pArgv;

  wchar_t appPath[MAX_PATH];

  // Set application path.
  do
  {
    if (0 == GetModuleFileNameW(nullptr, appPath, ARRAYSIZE(appPath)))
      FATAL("Failed to get application binary path.");

    if (!PathRemoveFileSpecW(appPath))
      FATAL("Failed to retrieve folder path.");

    if (!SetCurrentDirectoryW(appPath))
      FATAL("Failed to set working directory.");

  } while (0);

  DWORD processId, threadId;

  // Launch Process.
  {
    PROCESS_INFORMATION processInfo;
    ZeroMemory(&processInfo, sizeof(processInfo));

    STARTUPINFO startupInfo;
    ZeroMemory(&startupInfo, sizeof(startupInfo));
    startupInfo.cb = sizeof(startupInfo);

    if (!CreateProcessW(TEXT("example.exe"), nullptr, nullptr, nullptr, FALSE, DEBUG_PROCESS | CREATE_NEW_CONSOLE, nullptr, appPath, &startupInfo, &processInfo))
      FATAL("Failed to create process. (0x%" PRIX32 ")", GetLastError());

    processId = processInfo.dwProcessId;
    threadId = processInfo.dwThreadId;

    printf("Created process %" PRIi32 ", thread %" PRIi32 ".\n", processId, threadId);

    CloseHandle(processInfo.hThread);
    CloseHandle(processInfo.hProcess);
  }

  lt_log(0, "Start");

  // Debug Process.
  {
    bool keepRunning = true;

    do
    {
      DEBUG_EVENT debugEvent;

      if (!WaitForDebugEvent(&debugEvent, 2000))
      {
        puts("Failed to wait for debug event.");
        break;
      }

      DWORD continueStatus = DBG_CONTINUE;

      switch (debugEvent.dwDebugEventCode)
      {
      case EXIT_PROCESS_DEBUG_EVENT:
        keepRunning = false;
        break;

      case EXCEPTION_DEBUG_EVENT:
        continueStatus = DBG_EXCEPTION_NOT_HANDLED;
        
        if (debugEvent.dwProcessId == processId && debugEvent.dwThreadId == threadId)
        {
          lt_crash_ex(debugEvent.dwProcessId, debugEvent.dwThreadId, debugEvent.u.Exception.ExceptionRecord.ExceptionCode, "Crash in child process.");
          printf("Wrote excaption as crash log. (exception 0x%" PRIX32 ")\n", debugEvent.u.Exception.ExceptionRecord.ExceptionCode);
        }

        break;

      default:
        break;
      }

      if (!ContinueDebugEvent(debugEvent.dwProcessId, debugEvent.dwThreadId, continueStatus))
      {
        puts("Failed to continue debugged process.");
        break;
      }

    } while (keepRunning);
  }

  lt_log(0, "End");
  puts("Exit.");

  return 0;
}
