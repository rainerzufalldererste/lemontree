#include "lta.h"
#include "lta_types.h"
#include "lta_io.h"
#include "lta_op.h"

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

struct analyze_options
{
  bool ignoreMinorVersionDiff = false;
};

//////////////////////////////////////////////////////////////////////////

bool analyze_file(const wchar_t *inputFileName, lt_analyze *pAnalyze, bool isNewFile, IN const analyze_options *pOptions)
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
        //uint16_t size = 0;
        //FATAL_IF(!stream.read(&size), "Insufficient data stream.");
        //
        //SKIP_X64(",", "timestamp", stream);
        //SKIP_X64(",", "subSystem", stream);
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

int32_t main(void)
{
  const wchar_t *commandLine = GetCommandLineW();

  int32_t argc = 0;
  wchar_t **pArgv = CommandLineToArgvW(commandLine, &argc);
  FATAL_IF(argc < 4, "Invalid Parameter.\nUsage: [-io <LT Analyze File> | -o <New LT Analyze File>] [%s] <LT Log File> ... ", _Option_IgnoreMinorVersionDiff);

  const wchar_t *outputFileName = pArgv[2];
  bool isNewFile = true;

  lt_analyze analyze;
  analyze_options options;

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
    else
    {
      FATAL_IF(!analyze_file(pArgv[i], &analyze, isNewFile, &options), "Failed to analyze file %ws", pArgv[i]);

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
    JsonWriter writer(stdout);

    FATAL_IF(!jsonify(&analyze, &writer), "Failed to jsonify analyze data.");
  }

  return 0;
}
