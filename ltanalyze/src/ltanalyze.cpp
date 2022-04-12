#include "lta.h"
#include "lta_types.h"
#include "lta_io.h"
#include "lta_op.h"

#include <windows.h>
#include <debugapi.h>
#include <psapi.h>
#include <atlutil.h>
#include <dia2.h>
#include <diacreate.h>
#include <cvconst.h>
#include <initguid.h>
#include <fcntl.h>

extern "C"
{
#define ZYCORE_STATIC_DEFINE
#include <Zydis/Zydis.h>
}

//////////////////////////////////////////////////////////////////////////

struct lt_sub_system
{
  uint64_t subSystem;
  bool hasLastState = false;
  bool hasLastOperation = false;

  lt_state_identifier lastState;
  lt_operation_identifier lastOperation;
  uint64_t lastStateTimestamp;
  uint64_t lastOperationTimestamp;

  std::vector<std::pair<lt_state_identifier, uint64_t>> previousStates;

  inline lt_sub_system(const uint64_t subSystem) : subSystem(subSystem) {}
};

struct lt_analyze_state
{
  std::vector<lt_sub_system> subSystems;
};

lt_sub_system *get_sub_system(IN lt_analyze_state *pState, const uint64_t subSystem)
{
  for (auto &_subState : pState->subSystems)
    if (_subState.subSystem == subSystem)
      return &_subState;

  // Add SubSystem.
  {
    pState->subSystems.emplace_back(subSystem);
    
    return &pState->subSystems.back();
  }
}

void update_previous_state_timing(lt_sub_system *pSubSystem, lt_state_identifier *pId, const uint64_t timestamp)
{
  for (auto &_item : pSubSystem->previousStates)
  {
    if (_item.first.stateIndex == pId->stateIndex && _item.first.subStateIndex == pId->subStateIndex)
    {
      _item.second = timestamp;
      return;
    }
  }

  // Add Pair.
  {
    pSubSystem->previousStates.emplace_back(*pId, timestamp);
  }
}

//////////////////////////////////////////////////////////////////////////

struct lt_pdb_context
{
  IDiaSession *pPdbSession = nullptr;
  bool disasmAvailable = false;
  ZydisDecoder disasmDecoder;
  ZydisFormatter disasmFormatter;
};

bool get_tracktrace_from_pdb(IN_OUT ByteStream *pStream, IN lt_pdb_context *pPdbContext, OUT std::vector<lt_stack_trace> *pStackTrace)
{
  pStackTrace->clear();

  // Print Stacktraces.
  {
    uint8_t u8;
    uint32_t u32;

    if (!pStream->read(&u8))
      RETURN_ERROR;

    if (u8 != lt_st_start) // invalid header.
      RETURN_ERROR;

    if (!pStream->read(&u32)) // hash.
      RETURN_ERROR;

    char moduleName[256] = "<Invalid Module>";

    while (true)
    {
      lt_stack_trace trace = {};

      uint8_t type;

      if (!pStream->read(&type))
        RETURN_ERROR;

      bool end = false;

      switch (type)
      {
      default:
      {
        RETURN_ERROR; // Invalid Type.
        break;
      }

      case lt_st_end:
      {
        end = true;
        break;
      }

      case lt_st_app_offset:
      {
        trace.stackTraceType = lt_stt_internal_offset;

        if (!pStream->read(&trace.offset))
          RETURN_ERROR;

        CComPtr<IDiaEnumLineNumbers> lineNumEnum;
        CComPtr<IDiaSymbol> symbol;

        wchar_t *symbolName = nullptr;

        if (SUCCEEDED(pPdbContext->pPdbSession->findSymbolByVA(trace.offset, SymTagFunction, &symbol)) && symbol != nullptr && ((SUCCEEDED(symbol->get_undecoratedName(&symbolName)) && symbolName != nullptr) || SUCCEEDED(symbol->get_name(&symbolName))))
        {
          if (0 < WideCharToMultiByte(CP_UTF8, 0, symbolName, (int32_t)wcslen(symbolName), trace.info.functionName.functionName, (int32_t)sizeof(trace.info.functionName.functionName), nullptr, false))
          {
            trace.stackTraceType = lt_stt_function_name;
          }
          else
          {
            trace.info.functionName.functionName[0] = '\0';
          }
        }

        if (symbolName != nullptr)
          SysFreeString(symbolName);

        if (SUCCEEDED(pPdbContext->pPdbSession->findLinesByVA(trace.offset, 1, &lineNumEnum)))
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
                  if (0 < WideCharToMultiByte(CP_UTF8, 0, sourceFileName, (int32_t)wcslen(sourceFileName), trace.info.functionName.file, (int32_t)sizeof(trace.info.functionName.file), nullptr, false))
                  {
                    trace.stackTraceType = lt_stt_function_name;
                  }
                  else
                  {
                    trace.info.functionName.file[0] = '\0';
                  }

                  DWORD line;

                  if (SUCCEEDED(lineNumber->get_lineNumber(&line)))
                    trace.info.functionName.line = (uint32_t)line;
                  else
                    trace.info.functionName.line = 0;
                }

                if (sourceFileName != nullptr)
                  SysFreeString(sourceFileName);
              }
            }
          }
        }

        pStackTrace->emplace_back(std::move(trace));

        break;
      }

      case lt_st_same_dll_offset:
      {
        trace.stackTraceType = lt_stt_external_module;

        if (!pStream->read(&trace.offset))
          RETURN_ERROR;

        strncpy_s(trace.info.externalModule.moduleName, moduleName, sizeof(moduleName));

        pStackTrace->emplace_back(std::move(trace));

        break;
      }

      case lt_st_dll_offset:
      {
        trace.stackTraceType = lt_stt_external_module;

        READ_STRING(pStream, moduleName);

        if (!pStream->read(&trace.offset))
          RETURN_ERROR;

        strncpy_s(trace.info.externalModule.moduleName, moduleName, sizeof(moduleName));

        pStackTrace->emplace_back(std::move(trace));

        break;
      }

      case lt_st_data16:
      {
        uint8_t data[16];

        if (!pStream->read(data, 16))
          RETURN_ERROR;

        if (pStackTrace->size() > 0 && pPdbContext->disasmAvailable)
        {
          lt_stack_trace *pTrace = &pStackTrace->back();

          ZydisDecodedInstruction instruction;

          if (ZYAN_SUCCESS(ZydisDecoderDecodeBuffer(&pPdbContext->disasmDecoder, data, 16, &instruction)) && ZYAN_SUCCESS(ZydisFormatterFormatInstruction(&pPdbContext->disasmFormatter, &instruction, pTrace->disasm, sizeof(pTrace->disasm), pTrace->offset)))
          {
            pTrace->hasDisasm = true;
          }
          else
          {
            char lut[] = "0123456789ABCDEF";

            for (size_t i = 0; i < 16; i++)
            {
              pTrace->disasm[i * 3 + 0] = lut[data[i] >> 4];
              pTrace->disasm[i * 3 + 1] = lut[data[i] & 0xF];
              pTrace->disasm[i * 3 + 2] = ' ';
            }

            pTrace->disasm[15 * 3 + 2] = '\0';
            pTrace->hasDisasm = true;
          }
        }

        break;
      }
      }

      if (end)
        break;
    }
  }

  return true;
}

//////////////////////////////////////////////////////////////////////////

struct lt_analyze_options
{
  bool ignoreMinorVersionDiff = false;
};

//////////////////////////////////////////////////////////////////////////

bool analyze_file(const wchar_t *inputFileName, lt_analyze *pAnalyze, bool isNewFile, IN const lt_analyze_options *pOptions, IN lt_pdb_context *pPdbContext)
{
  lt_analyze_state state;

  size_t fileSize = 0;
  uint8_t *pData = nullptr;

  // Read Telemetry File.
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
  lt_hw_info hwInfo;
  lt_short_hw_info hwInfoShort;
  bool hwInfoStored = false;

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

  // Parse Sections.
  {
    if (!isNewFile)
    {
      FATAL_IF(0 != strncmp(productName, pAnalyze->productName, sizeof(productName)), "Error! Incompatible Product Name. '%s' != '%s'.", productName, pAnalyze->productName);
      FATAL_IF(majorVersion != pAnalyze->majorVersion, "Error! Incompatible Major Version. 0x%" PRIX64 " != 0x%" PRIX64 ".", majorVersion, pAnalyze->majorVersion);

      if (!pOptions->ignoreMinorVersionDiff)
        FATAL_IF(minorVersion != pAnalyze->minorVersion, "Error! Incompatible Major Version. 0x%" PRIX64 " != 0x%" PRIX64 ".", minorVersion, pAnalyze->minorVersion);
    }
    else
    {
      memcpy(pAnalyze->productName, productName, sizeof(productName));
      pAnalyze->majorVersion = majorVersion;
      pAnalyze->minorVersion = minorVersion;
    }

    for (const auto &_item : headers)
    {
      ByteStream stream(_item);

      uint8_t type = 0;
      FATAL_IF(!stream.read(&type), "Insufficient Data");

      switch (type)
      {
      case lt_t_state:
      {
        lt_state_identifier id;
        uint64_t subSystem, timestamp;

        FATAL_IF(!stream.read(&subSystem), "Insufficient Data");
        FATAL_IF(!stream.read(&id.stateIndex), "Insufficient Data");
        FATAL_IF(!stream.read(&id.subStateIndex), "Insufficient Data");
        FATAL_IF(!stream.read(&timestamp), "Insufficient Data");

        lt_sub_system *pSubSystem = get_sub_system(&state, subSystem);
        lt_state *pSelf = get_state(pAnalyze, subSystem, id.stateIndex, id.subStateIndex);

        pSelf->avgTimeSinceStartS = adjust(pSelf->avgTimeSinceStartS, to_seconds(timestamp - startTimestamp), pSelf->data.count);

        if (pSubSystem->hasLastState)
        {
          lt_state *pLastState = get_state(pAnalyze, subSystem, pSubSystem->lastState.stateIndex, pSubSystem->lastState.subStateIndex);

          const uint64_t lastStateDelay = timestamp - pSubSystem->lastStateTimestamp;

          update_transition_data(&pLastState->data, lastStateDelay);
          update_transition_data(get_transition_data(&pLastState->nextState, &id), lastStateDelay);
          update_transition_data(get_transition_data(&pSelf->previousState, &pSubSystem->lastState), lastStateDelay);
        }

        for (auto &_states : pSubSystem->previousStates)
        {
          lt_state *pState = get_state(pAnalyze, subSystem, _states.first.stateIndex, _states.first.subStateIndex);

          const uint64_t stateDelay = timestamp - _states.second;

          update_transition_data(get_transition_data(&pState->stateReach, &id), stateDelay);
        }

        if (pSubSystem->hasLastOperation)
        {
          lt_operation *pOp = get_operation(pAnalyze, subSystem, pSubSystem->lastOperation.operationType);

          const uint64_t lastOperationDelay = timestamp - pSubSystem->lastOperationTimestamp;

          update_transition_data(get_transition_data(&pOp->nextState, &id), lastOperationDelay);
          update_transition_data(get_transition_data(&pSelf->previousOperation, &pSubSystem->lastOperation), lastOperationDelay);
        }

        pSubSystem->hasLastState = true;
        pSubSystem->lastState = id;
        pSubSystem->lastStateTimestamp = timestamp;

        update_previous_state_timing(pSubSystem, &id, timestamp);

        break;
      }

      case lt_t_operation:
      {
        uint64_t subSystem, operationIndex, timestamp;
        lt_operation_identifier id;

        FATAL_IF(!stream.read(&subSystem), "Insufficient Data");
        FATAL_IF(!stream.read(&id.operationType), "Insufficient Data");
        FATAL_IF(!stream.read(&operationIndex), "Insufficient Data");
        FATAL_IF(!stream.read(&timestamp), "Insufficient Data");

        lt_sub_system *pSubSystem = get_sub_system(&state, subSystem);
        lt_operation *pSelf = get_operation(pAnalyze, subSystem, id.operationType);

        pSelf->avgTimeSinceStartS = adjust(pSelf->avgTimeSinceStartS, to_seconds(timestamp - startTimestamp), pSelf->data.count);
        update_operation_index_counts(&pSelf->operationIndexCount, operationIndex);

        if (pSubSystem->hasLastState)
        {
          lt_state *pLastState = get_state(pAnalyze, subSystem, pSubSystem->lastState.stateIndex, pSubSystem->lastState.subStateIndex);

          const uint64_t lastStateDelay = timestamp - pSubSystem->lastStateTimestamp;

          update_operation_transition_data(get_operation_transition_data(&pLastState->operations, &id), lastStateDelay, operationIndex);
          update_transition_data(get_transition_data(&pSelf->parentState, &pSubSystem->lastState), lastStateDelay);
        }

        for (auto &_states : pSubSystem->previousStates)
        {
          lt_state *pState = get_state(pAnalyze, subSystem, _states.first.stateIndex, _states.first.subStateIndex);

          const uint64_t stateDelay = timestamp - _states.second;

          update_transition_data(get_transition_data(&pState->operationReach, &id), stateDelay);
        }

        if (pSubSystem->hasLastOperation)
        {
          lt_operation *pOp = get_operation(pAnalyze, subSystem, pSubSystem->lastOperation.operationType);

          pOp->avgTimeSinceStartS = adjust(pOp->avgTimeSinceStartS, to_seconds(timestamp - startTimestamp), pOp->data.count);

          const uint64_t lastOperationDelay = timestamp - pSubSystem->lastOperationTimestamp;

          update_transition_data(&pOp->data, lastOperationDelay);
          update_operation_transition_data(get_operation_transition_data(&pOp->nextOperation, &id), lastOperationDelay, operationIndex);
          update_transition_data(get_transition_data(&pSelf->lastOperation, &id), lastOperationDelay);
        }

        pSubSystem->hasLastOperation = true;
        pSubSystem->lastOperation = id;
        pSubSystem->lastOperationTimestamp = timestamp;

        break;
      }

      case lt_t_perf_metric:
      {
        uint64_t valueIndex;
        FATAL_IF(!stream.read(&valueIndex), "Insufficient data steam.");

        double value = 0;
        FATAL_IF(!stream.read(&value), "Insufficient data stream.");

        uint64_t timestamp;
        FATAL_IF(!stream.read(&timestamp), "Insufficient data stream.");

        lt_perf_value_range<double> *pValue = get_perf_value_range_data(&pAnalyze->perfMetrics, valueIndex);
        update_perf_value_range(pValue, value, timestamp - startTimestamp, &hwInfoShort);

        break;
      }

      case lt_t_observed_value:
      {
        uint8_t dataType = 0;
        FATAL_IF(!stream.read(&dataType), "Insufficient data steam.");

        uint64_t valueIndex;
        FATAL_IF(!stream.read(&valueIndex), "Insufficient data steam.");
        
        uint64_t u64 = 0;
        int64_t i64 = 0;
        double f64 = 0;

        switch (dataType)
        {
        case lt_vt_u64:
          FATAL_IF(!stream.read(&u64), "Insufficient data stream.");
          break;
        
        case lt_vt_i64:
          FATAL_IF(!stream.read(&i64), "Insufficient data stream.");
          break;
        
        case lt_vt_f64:
          FATAL_IF(!stream.read(&f64), "Insufficient data stream.");
          break;
        
        default:
          RECOVERABLE_ERROR("Invalid data type.");
          break;
        }

        uint64_t timestamp;
        FATAL_IF(!stream.read(&timestamp), "Insufficient data stream.");

        switch (dataType)
        {
        case lt_vt_u64:
        {
          lt_value_range<uint64_t> *pValue = get_value_range_data(&pAnalyze->observedRangeU64, valueIndex);
          update_transition_data(&pValue->data, timestamp - startTimestamp);
          update_value_range(pValue, u64);

          break;
        }

        case lt_vt_i64:
        {
          lt_value_range<int64_t> *pValue = get_value_range_data(&pAnalyze->observedRangeI64, valueIndex);
          update_transition_data(&pValue->data, timestamp - startTimestamp);
          update_value_range(pValue, i64);

          break;
        }

        case lt_vt_f64:
        {
          lt_value_range<double> *pValue = get_value_range_data(&pAnalyze->observedRangeF64, valueIndex);
          update_transition_data(&pValue->data, timestamp - startTimestamp);
          update_value_range(pValue, f64);

          break;
        }

        default:
          RECOVERABLE_ERROR("Invalid data type.");
          break;
        }

        break;
      }

      case lt_t_observed_exact_value:
      {
        uint8_t dataType = 0;
        FATAL_IF(!stream.read(&dataType), "Insufficient data steam.");

        uint64_t valueIndex;
        FATAL_IF(!stream.read(&valueIndex), "Insufficient data steam.");

        uint64_t u64 = 0;
        int64_t i64 = 0;
        
        switch (dataType)
        {
        case lt_vt_u64:
          FATAL_IF(!stream.read(&u64), "Insufficient data stream.");
          break;
        
        case lt_vt_i64:
          FATAL_IF(!stream.read(&i64), "Insufficient data stream.");
          break;
        
        //case lt_vt_f64:
        //  SKIP_F64(",", "value", stream);
        //  break;
        
        default:
          RECOVERABLE_ERROR("Invalid data type.");
          break;
        }
        
        uint64_t timestamp;
        FATAL_IF(!stream.read(&timestamp), "Insufficient data stream.");

        switch (dataType)
        {
        case lt_vt_u64:
        {
          lt_values_exact<uint64_t> *pValue = get_exact_value_data(&pAnalyze->observedU64, valueIndex);
          update_transition_data(&pValue->data, timestamp - startTimestamp);
          update_exact_value(pValue, u64);
          
          break;
        }

        case lt_vt_i64:
        {
          lt_values_exact<int64_t> *pValue = get_exact_value_data(&pAnalyze->observedI64, valueIndex);
          update_transition_data(&pValue->data, timestamp - startTimestamp);
          update_exact_value(pValue, i64);

          break;
        }

        default:
          RECOVERABLE_ERROR("Invalid data type.");
          break;
        }
        
        break;
      }

      case lt_t_crash:
      {
        //uint16_t size = 0;
        //FATAL_IF(!stream.read(&size), "Insufficient data stream.");
        //
        //SKIP_X64(",", "timestamp", stream);
        //SKIP_X64(",", "errorCode", stream);
        //SKIP_STRING(",", "description", stream);
        //
        //uint16_t stackTraceLength = 0;
        //FATAL_IF(!stream.read(&stackTraceLength), "Insufficient data stream.");
        //
        //if (stackTraceLength > 0)
        //{
        //  const uint8_t *pStackTraceData = stream.pData;
        //  FATAL_IF(!stream.read<uint8_t >(nullptr, stackTraceLength), "Insufficient data stream.");
        //  fputs(",\"stacktrace\":", stdout);
        //  print_bytes_as_base64string(pStackTraceData, stackTraceLength);
        //}

        break;
      }

      case lt_t_error:
      case lt_t_warning:
      {
        uint16_t size = 0;
        FATAL_IF(!stream.read(&size), "Insufficient data stream.");

        uint64_t timestamp, subSystem, errorCode;
        char description[0x100];

        READ(&stream, timestamp);
        READ(&stream, subSystem);
        READ(&stream, errorCode);
        READ_STRING(&stream, description);

        uint8_t stackTraceData[1024 * 10];

        uint16_t stackTraceLength = 0;
        FATAL_IF(!stream.read(&stackTraceLength), "Insufficient data stream.");
        FATAL_IF(stackTraceLength > sizeof(stackTraceData), "Stack Trace size exceeds capacity.");

        uint32_t stackTraceHash = 0;

        if (stackTraceLength > 0)
        {
          const uint8_t *pStackTraceData = stream.pData;
          FATAL_IF(!stream.read<uint8_t >(nullptr, stackTraceLength), "Insufficient data stream.");
          memcpy(stackTraceData, pStackTraceData, stackTraceLength);

          FATAL_IF(stackTraceLength < 6, "Insufficient StackTrace Length.");
          stackTraceHash = *reinterpret_cast<uint32_t *>(stackTraceData + 1);
        }

        lt_sub_system *pSubSystem = get_sub_system(&state, subSystem);
        SoaList<uint64_t, lt_error_data> *pErrors = nullptr;
        uint64_t cmpTimestamp = startTimestamp;

        if (pSubSystem->hasLastState)
        {
          lt_state *pState = get_state(pAnalyze, subSystem, pSubSystem->lastState.stateIndex, pSubSystem->lastState.subStateIndex);

          pErrors = type == lt_t_error ? &pState->errors : &pState->warnings;
          cmpTimestamp = pSubSystem->lastStateTimestamp;
        }
        else
        {
          lt_sub_system_data *pSubSystemData = get_sub_system_data(pAnalyze, subSystem);

          pErrors = type == lt_t_error ? &pSubSystemData->noStateErrors : &pSubSystemData->noStateWarnings;
        }

        uint64_t errorIndex;
        bool isNewEntry;

        lt_error_data *pErrorData = get_error_data(pErrors, errorCode, strnlen(description, sizeof(description)) != 0, description, stackTraceLength > 0, stackTraceHash, &isNewEntry, &errorIndex);

        if (pErrorData->error.hasStackTrace && pErrorData->error.stackTrace.size() == 0 && pPdbContext->pPdbSession != nullptr)
        {
          ByteStream stackTraceStream(stackTraceData, stackTraceLength);

          if (!get_tracktrace_from_pdb(&stackTraceStream, pPdbContext, &pErrorData->error.stackTrace))
            RETURN_ERROR;
        }

        update_transition_data(&pErrorData->data, timestamp - cmpTimestamp);

        if (pSubSystem->hasLastOperation)
        {
          lt_operation *pOp = get_operation(pAnalyze, subSystem, pSubSystem->lastOperation.operationType);
          SoaList<lt_error_identifier, lt_transition_data> *pErrorTransitionData = type == lt_t_error ? &pOp->errors : &pOp->warnings;

          lt_error_identifier idx;
          idx.errorIndex = errorIndex;
          idx.hasStateIndex = pSubSystem->hasLastState;

          if (pSubSystem->hasLastState)
            idx.state = pSubSystem->lastState;

          update_transition_data(get_transition_data(pErrorTransitionData, &idx), timestamp - pSubSystem->lastOperationTimestamp);
        }

        break;
      }

      case lt_t_log:
      {
        //uint16_t size = 0;
        //FATAL_IF(!stream.read(&size), "Insufficient data stream.");
        //
        //SKIP_X64(",", "timestamp", stream);
        //SKIP_X64(",", "subSystem", stream);
        //SKIP_STRING(",", "description", stream);

        break;
      }

      case lt_t_perf_data:
      {
        uint16_t size = 0;
        FATAL_IF(!stream.read(&size), "Insufficient data stream.");

        uint64_t subSystem, timestamp;

        FATAL_IF(!stream.read(&timestamp), "Insufficient Data");
        FATAL_IF(!stream.read(&subSystem), "Insufficient Data");

        uint8_t count = 0;
        FATAL_IF(!stream.read(&count), "Insufficient data stream.");

        lt_sub_system *pSubSystem = get_sub_system(&state, subSystem);
        lt_sub_system_data *pSSData = get_sub_system_data(pAnalyze, subSystem);

        const double *pPerfData = reinterpret_cast<const double *>(stream.pData);
        FATAL_IF(!stream.read<double>(nullptr, count), "Insufficient data stream.");

        for (uint8_t i = 0; i < count; i++)
          add_perf_data(&pSSData->profilerData, i, pPerfData[i], &hwInfoShort, pSubSystem->hasLastOperation, &pSubSystem->lastOperation);
        
        if (count > 0 && pSubSystem->hasLastState)
        {
          lt_state *pState = get_state(pAnalyze, subSystem, pSubSystem->lastState.stateIndex, pSubSystem->lastState.subStateIndex);

          for (uint8_t i = 0; i < count; i++)
            add_perf_data(&pState->profilerData, i, pPerfData[i], &hwInfoShort, pSubSystem->hasLastOperation, &pSubSystem->lastOperation);
        }

        break;
      }

      case lt_t_observed_exact_value_variable_length:
      {
        uint16_t size = 0;
        FATAL_IF(!stream.read(&size), "Insufficient data stream.");
        
        uint8_t dataType = 0;
        FATAL_IF(!stream.read(&dataType), "Insufficient data stream.");
        
        uint64_t valueIndex, timestamp;
        FATAL_IF(!stream.read(&valueIndex), "Insufficient data stream.");
        FATAL_IF(!stream.read(&timestamp), "Insufficient data stream.");
        
        RECOVERABLE_ERROR_IF(dataType != lt_vt_string, "Invalid Value Type.");
        
        if (dataType == lt_vt_string)
        {
          char string[0x100];
          READ_STRING(&stream, string);

          lt_values<lt_string_value_entry> *pValue = get_exact_value_data(&pAnalyze->observedString, valueIndex);
          update_transition_data(&pValue->data, timestamp - startTimestamp);
          update_string_value(&pValue->values, string);
        }

        break;
      }

      case lt_t_system_info:
      {
        uint16_t size = 0;
        FATAL_IF(!stream.read(&size), "Insufficient data stream.");
        
        ByteStream *pStream = &stream;

        while (stream.sizeRemaining > 0)
        {
          uint8_t infoType = 0;
          FATAL_IF(!stream.read(&infoType), "Insufficient data stream.");
        
          switch (infoType)
          {
          case lt_si_cpu:
          {
            hwInfo.hasCpuInfo = true;
            READ_STRING(pStream, hwInfo.cpuName);
            READ(pStream, hwInfo.cpuCores);
        
            break;
          }
        
          case lt_si_ram:
          {
            hwInfo.hasRamInfo = true;
            READ(pStream, hwInfo.ramTotalPhysical);
            READ(pStream, hwInfo.ramAvailablePhysical);
            READ(pStream, hwInfo.ramTotalVirtual);
            READ(pStream, hwInfo.ramAvailableVirtual);
        
            break;
          }
        
          case lt_si_os:
          {
            hwInfo.hasOsInfo = true;
            READ_STRING(pStream, hwInfo.osName);
        
            break;
          }
        
          case lt_si_gpu:
          {
            hwInfo.hasGpuInfo = true;
            READ(pStream, hwInfo.gpuDedicatedVRam);
            READ(pStream, hwInfo.gpuSharedVRam);
            READ(pStream, hwInfo.gpuTotalVRam);
            READ(pStream, hwInfo.gpuFreeVRam);
            READ(pStream, hwInfo.gpuDriverVersion);
            READ(pStream, hwInfo.gpuVendorId);
            READ(pStream, hwInfo.gpuDeviceId);
            READ(pStream, hwInfo.gpuRevisionId);
            READ(pStream, hwInfo.gpuBoardId);
            READ_STRING(pStream, hwInfo.gpuName);
            READ_STRING(pStream, hwInfo.gpuDriverName);
        
            break;
          }
        
          case lt_si_lang:
          {
            hwInfo.hasLangInfo = true;
            READ_STRING(pStream, hwInfo.langPrimaryName);
            
            break;
          }
        
          case lt_si_elevated:
          {
            hwInfo.hasElevatedInfo = true;
            READ(pStream, hwInfo.elevated);
            
            break;
          }
        
          case lt_si_monitor:
          {
            hwInfo.hasMonitorInfo = true;

            uint8_t count = 0;
            FATAL_IF(!stream.read(&count), "Insufficient data stream.");
        
            int32_t minPosX = 0;
            int32_t maxPosX = 0;
            int32_t minPosY = 0;
            int32_t maxPosY = 0;

            for (uint8_t i = 0; i < count; i++)
            {
              uint32_t sizeX, sizeY, dpiX, dpiY;
              int32_t posX, posY;

              READ(pStream, posX);
              READ(pStream, posY);
              READ(pStream, sizeX);
              READ(pStream, sizeY);
              READ(pStream, dpiX);
              READ(pStream, dpiY);

              if (posX < minPosX)
                minPosX = posX;

              if (posY < minPosY)
                minPosY = posY;

              if ((int32_t)(posX + sizeX) > maxPosX)
                maxPosX = posX + sizeX;

              if ((int32_t)(posY + sizeY) > maxPosY)
                maxPosY = posY + sizeY;

              lt_monitor_info info;
              info.width = sizeX;
              info.height = sizeY;
              info.dpi = (dpiX + dpiY) / 2;

              hwInfo.monitorSizes.push_back(std::move(info));
            }

            hwInfo.monitorTotalWidth = (uint32_t)(maxPosX - minPosX);
            hwInfo.monitorTotalHeight = (uint32_t)(maxPosY - minPosY);
        
            break;
          }
        
          case lt_si_storage:
          {
            hwInfo.hasStorageInfo = true;

            READ(pStream, hwInfo.storageAvailable);
            READ(pStream, hwInfo.storageTotal);
        
            break;
          }
        
          case lt_si_device:
          {
            hwInfo.hasDeviceInfo = true;

            READ_STRING(pStream, hwInfo.deviceManufacturerName);
            READ_STRING(pStream, hwInfo.deviceModelName);
        
            break;
          }
        
          default:
          {
            stream.sizeRemaining = 0;
            RECOVERABLE_ERROR("Unsupported Info (0x%02" PRIX8 ")", infoType);
            break;
          }
          }
        }

        to_short_hw_info(&hwInfo, &hwInfoShort);

        if (!hwInfoStored)
        {
          hwInfoStored = true;

          update_hw_info(&pAnalyze->hwInfo, &hwInfo);
        }

        break;
      }

      default:
      {
        RECOVERABLE_ERROR("Unsupported Type (0x%02" PRIX8 ")", type);
        break;
      }
      }
    }
  }

  // TODO: Store Device Info.

  free(pData);

  return true;
}

//////////////////////////////////////////////////////////////////////////

#define OPTION_IGNORE_MINOR_VERSION_DIFF "--ignore-minor-version-diff"
static const char _Option_IgnoreMinorVersionDiff[] = OPTION_IGNORE_MINOR_VERSION_DIFF;
static const wchar_t _wOption_IgnoreMinorVersionDiff[] = TEXT(OPTION_IGNORE_MINOR_VERSION_DIFF);

#define OPTION_PDB "--pdb"
static const char _Option_PDB[] = OPTION_PDB;
static const wchar_t _wOption_PDB[] = TEXT(OPTION_PDB);

#define OPTION_DISASM "--disasm"
static const char _Option_Disasm[] = OPTION_DISASM;
static const wchar_t _wOption_Disasm[] = TEXT(OPTION_DISASM);

#define OPTION_OUT "--out"
static const char _Option_Out[] = OPTION_OUT;
static const wchar_t _wOption_Out[] = TEXT(OPTION_OUT);

int32_t main(void)
{
  const wchar_t *commandLine = GetCommandLineW();

  int32_t argc = 0;
  wchar_t **pArgv = CommandLineToArgvW(commandLine, &argc);
  FATAL_IF(argc < 4, "Invalid Parameter.\nUsage: [-io <LT Analyze File> | -o <New LT Analyze File>] [%s] [%s <PDB File> [%s]] <LT Log File> ... [%s <OUT Filename>]", _Option_IgnoreMinorVersionDiff, _Option_PDB, _Option_Disasm, _Option_Out);

  const wchar_t *outputFileName = pArgv[2];
  bool isNewFile = true;
  FILE *pOutFile = stdout;

  lt_analyze analyze;
  lt_analyze_options options;
  lt_pdb_context pdbContext;

  // Read Analyze File (if -o)
  if (0 != wcsncmp(pArgv[1], L"-o", 3))
  {
    isNewFile = false;

    HANDLE file = CreateFileW(outputFileName, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, 0, nullptr);
    FATAL_IF(file == INVALID_HANDLE_VALUE, "Failed to open analyze file. (0x%" PRIX32 ")", GetLastError());

    LARGE_INTEGER _fileSize;
    FATAL_IF(!GetFileSizeEx(file, &_fileSize), "Failed to retrieve analyze file size.");

    const size_t inFileSize = (size_t)_fileSize.QuadPart;
    uint8_t *pInData = reinterpret_cast<uint8_t *>(malloc(inFileSize));

    FATAL_IF(pInData == nullptr, "Failed to allocate memory.");

    size_t bytesRemaining = inFileSize;
    size_t offset = 0;

    while (bytesRemaining > 0)
    {
      const DWORD bytesToRead = (DWORD)min(bytesRemaining, MAXDWORD);
      DWORD bytesRead = 0;

      FATAL_IF(!ReadFile(file, pInData + offset, bytesToRead, &bytesRead, nullptr), "Failed to read log file. (0x%" PRIX32 ")", GetLastError());
      FATAL_IF(bytesRead == 0, "Failed to read from log file.");

      offset += bytesRead;
      bytesRemaining -= bytesRead;
    }

    ByteStream bs(pInData, inFileSize);

    FATAL_IF(!deserialize(&analyze, &bs), "Failed to deserialize input analyze file.");

    free(pInData);
    CloseHandle(file);
  }

  // Analyze Files.
  for (int32_t i = 3; i < argc; i++)
  {
    if (wcsncmp(pArgv[i], _wOption_IgnoreMinorVersionDiff, ARRAYSIZE(_wOption_IgnoreMinorVersionDiff)) == 0)
    {
      options.ignoreMinorVersionDiff = true;
    }
    else if (wcsncmp(pArgv[i], _wOption_PDB, ARRAYSIZE(_wOption_PDB)) == 0)
    {
      FATAL_IF(i + 1 >= argc, "Missing PDB File after argument.");
      FATAL_IF(pdbContext.pPdbSession != nullptr, "PDB Source is already initialized.");

      HRESULT hr;
      FATAL_IF(FAILED(hr = CoInitialize(nullptr)), "Failed to Initialize. Aborting.");

      CComPtr<IDiaDataSource> pdbSource;

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

      if (FAILED(pdbSource->loadDataFromPdb(pArgv[i + 1])))
        FATAL("Failed to load pdb. ('%ws')", pArgv[i + 1]);

      FATAL_IF(FAILED(hr = pdbSource->openSession(&pdbContext.pPdbSession)), "Failed to Open Session.");

      i++;
    }
    else if (wcsncmp(pArgv[i], _wOption_Disasm, ARRAYSIZE(_wOption_Disasm)) == 0)
    {
      FATAL_IF(pdbContext.disasmAvailable, "Disasm is already available.");

      pdbContext.disasmAvailable = ZYAN_SUCCESS(ZydisDecoderInit(&pdbContext.disasmDecoder, ZYDIS_MACHINE_MODE_LONG_64, ZYDIS_ADDRESS_WIDTH_64)) &&
        ZYAN_SUCCESS(ZydisFormatterInit(&pdbContext.disasmFormatter, ZYDIS_FORMATTER_STYLE_INTEL)) &&
        ZYAN_SUCCESS(ZydisFormatterSetProperty(&pdbContext.disasmFormatter, ZYDIS_FORMATTER_PROP_FORCE_SEGMENT, ZYAN_TRUE)) &&
        ZYAN_SUCCESS(ZydisFormatterSetProperty(&pdbContext.disasmFormatter, ZYDIS_FORMATTER_PROP_FORCE_SIZE, ZYAN_TRUE));
    }
    else if (wcsncmp(pArgv[i], _wOption_Out, ARRAYSIZE(_wOption_Out)) == 0)
    {
      FATAL_IF(i + 1 >= argc, "Missing out File path after argument.");

      pOutFile = _wfopen(pArgv[i + 1], L"w");

      FATAL_IF(pOutFile == nullptr, "Failed to open output file '%ws'.", pArgv[i + 1]);

      i++;
    }
    else
    {
      FATAL_IF(!analyze_file(pArgv[i], &analyze, isNewFile, &options, &pdbContext), "Failed to analyze file %ws", pArgv[i]);

      isNewFile = false;
    }
  }

  // Write analyze file.
  {
    StreamWriter writer;

    FATAL_IF(!serialize(&analyze, &writer), "Failed to serialize analyze file.");

    HANDLE file = CreateFileW(outputFileName, GENERIC_WRITE, 0, nullptr, OPEN_ALWAYS, 0, nullptr);
    FATAL_IF(file == INVALID_HANDLE_VALUE, "Failed to open analyze file. (0x%" PRIX32 ")", GetLastError());

    size_t remainingFileSize = writer.size;
    const uint8_t *pBytes = writer.pBytes;

    while (true)
    {
      const DWORD writeSize = (DWORD)min(MAXDWORD, remainingFileSize);
      DWORD writtenSize = 0;

      FATAL_IF(!WriteFile(file, pBytes, writeSize, &writtenSize, nullptr), "Failed to write analyze file with 0x%" PRIX32 ".", GetLastError());
      FATAL_IF(writtenSize == 0, "Failed to write.");

      if (remainingFileSize <= writtenSize)
        break;

      remainingFileSize -= writtenSize;
      pBytes += writtenSize;
    }

    CloseHandle(file);
  }

  // Write analyze json file.
  {
    JsonWriter writer(pOutFile);

    FATAL_IF(!jsonify(&analyze, &writer), "Failed to jsonify analyze data.");
  }

  return 0;
}
