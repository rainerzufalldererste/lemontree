#ifndef lta_op_h__
#define lta_op_h__

#include "lta_types.h"

//////////////////////////////////////////////////////////////////////////

inline void to_short_hw_info(IN const lt_hw_info *pHwInfo, OUT lt_short_hw_info *pShortHwInfo)
{
  if (pHwInfo->hasCpuInfo)
  {
    memcpy(pShortHwInfo->cpuName, pHwInfo->cpuName, sizeof(pHwInfo->cpuName));
    pShortHwInfo->cpuCores = pHwInfo->cpuCores;
  }

  if (pHwInfo->hasRamInfo)
  {
    pShortHwInfo->freeRam = pHwInfo->ramAvailablePhysical;
    pShortHwInfo->totalRam = pHwInfo->ramTotalPhysical;
  }

  if (pHwInfo->hasGpuInfo)
  {
    memcpy(pShortHwInfo->gpuName, pHwInfo->gpuName, sizeof(pHwInfo->gpuName));
    pShortHwInfo->dedicatedVRam = pHwInfo->gpuDedicatedVRam;
    pShortHwInfo->totalVRam = pHwInfo->gpuTotalVRam;
    pShortHwInfo->freeVRam = pHwInfo->gpuFreeVRam;
  }

  if (pHwInfo->hasOsInfo)
  {
    memcpy(pShortHwInfo->osName, pHwInfo->osName, sizeof(pHwInfo->osName));
  }

  if (pHwInfo->hasMonitorInfo)
  {
    pShortHwInfo->monitorCount = pHwInfo->monitorSizes.size();
    pShortHwInfo->monitorTotalWidth = pHwInfo->monitorTotalWidth;
    pShortHwInfo->monitorTotalHeight = pHwInfo->monitorTotalHeight;
  }
}

inline lt_sub_system_data *get_sub_system_data(IN lt_analyze *pAnalyze, const uint64_t subSystem)
{
  if (pAnalyze == nullptr)
    return nullptr;

  for (size_t i = 0; i < pAnalyze->subSystems.size(); i++)
    if (pAnalyze->subSystems.index[i] == subSystem)
      return &pAnalyze->subSystems.value[i];

  // Create New SubSystem.
  {
    pAnalyze->subSystems.push_back(subSystem, lt_sub_system_data());

    return &pAnalyze->subSystems.value.back();
  }
}

inline lt_state *get_state(IN lt_analyze *pAnalyze, const uint64_t subSystem, const uint64_t stateIndex, const uint64_t subStateIndex)
{
  if (pAnalyze == nullptr)
    return nullptr;

  lt_sub_system_data *pSSData = get_sub_system_data(pAnalyze, subSystem);

  auto &list = pSSData->states;

  // Try to find the state.
  for (size_t j = 0; j < list.size(); j++)
    if (list.index[j].stateIndex == stateIndex && list.index[j].subStateIndex == subStateIndex)
      return &list.value[j];

  // Create new state.
  {
    lt_state_identifier index;
    index.stateIndex = stateIndex;
    index.subStateIndex = subStateIndex;

    list.push_back(std::move(index), lt_state());

    return &list.value.back();
  }
}

inline lt_state *get_state(IN lt_analyze *pAnalyze, IN lt_full_state_identifier *pIndex)
{
  if (pIndex == nullptr)
    return nullptr;

  return get_state(pAnalyze, pIndex->subSystem, pIndex->stateIndex, pIndex->subStateIndex);
}

inline lt_operation *get_operation(IN lt_analyze *pAnalyze, const uint64_t subSystem, const uint64_t operationType)
{
  if (pAnalyze == nullptr)
    return nullptr;

  lt_sub_system_data *pSSData = get_sub_system_data(pAnalyze, subSystem);

  auto &list = pSSData->operations;

  // Try to find the state.
  for (size_t j = 0; j < list.size(); j++)
    if (list.index[j].operationType == operationType)
      return &list.value[j];

  // Create new state.
  {
    lt_operation_identifier index;
    index.operationType = operationType;

    list.push_back(std::move(index), lt_operation());

    return &list.value.back();
  }
}

inline lt_operation *get_operation(IN lt_analyze *pAnalyze, IN lt_full_operation_identifier *pIndex)
{
  if (pIndex == nullptr)
    return nullptr;

  return get_operation(pAnalyze, pIndex->subSystem, pIndex->operationType);
}

inline lt_transition_data *get_transition_data(SoaList<lt_state_identifier, lt_transition_data> *pList, const lt_state_identifier *pId)
{
  for (size_t i = 0; i < pList->size(); i++)
    if (pList->index[i].stateIndex == pId->stateIndex && pList->index[i].subStateIndex == pId->subStateIndex)
      return &pList->value[i];

  pList->push_back(*pId, lt_transition_data());

  return &pList->value.back();
}

inline lt_transition_data *get_transition_data(SoaList<lt_operation_identifier, lt_transition_data> *pList, const lt_operation_identifier *pId)
{
  for (size_t i = 0; i < pList->size(); i++)
    if (pList->index[i].operationType == pId->operationType)
      return &pList->value[i];

  pList->push_back(*pId, lt_transition_data());

  return &pList->value.back();
}

inline lt_transition_data *get_transition_data(SoaList<lt_error_identifier, lt_transition_data> *pList, const lt_error_identifier *pId)
{
  for (size_t i = 0; i < pList->size(); i++)
    if (pList->index[i].hasStateIndex == pId->hasStateIndex && (!pId->hasStateIndex || (pList->index[i].state.stateIndex == pId->state.stateIndex && pList->index[i].state.subStateIndex == pId->state.subStateIndex)) && pList->index[i].errorIndex == pId->errorIndex)
      return &pList->value[i];

  pList->push_back(*pId, lt_transition_data());

  return &pList->value.back();
}

inline lt_operation_transition_data *get_operation_transition_data(SoaList<lt_operation_identifier, lt_operation_transition_data> *pList, const lt_operation_identifier *pId)
{
  for (size_t i = 0; i < pList->size(); i++)
    if (pList->index[i].operationType == pId->operationType)
      return &pList->value[i];

  pList->push_back(*pId, lt_operation_transition_data());

  return &pList->value.back();
}

template <typename T>
inline lt_values_exact<T> *get_exact_value_data(SoaList<uint64_t, lt_values_exact<T>> *pList, const uint64_t index)
{
  for (size_t i = 0; i < pList->size(); i++)
    if (pList->index[i] == index)
      return &pList->value[i];

  pList->push_back(index, lt_values_exact<T>());

  return &pList->value.back();
}

template <typename T>
inline lt_values<T> *get_exact_value_data(SoaList<uint64_t, lt_values<T>> *pList, const uint64_t index)
{
  for (size_t i = 0; i < pList->size(); i++)
    if (pList->index[i] == index)
      return &pList->value[i];

  pList->push_back(index, lt_values<T>());

  return &pList->value.back();
}

template <typename T>
inline lt_value_range<T> *get_value_range_data(SoaList<uint64_t, lt_value_range<T>> *pList, const uint64_t index)
{
  for (size_t i = 0; i < pList->size(); i++)
    if (pList->index[i] == index)
      return &pList->value[i];

  pList->push_back(index, lt_value_range<T>());

  return &pList->value.back();
}

template <typename T>
inline lt_perf_value_range<T> *get_perf_value_range_data(SoaList<uint64_t, lt_perf_value_range<T>> *pList, const uint64_t index)
{
  for (size_t i = 0; i < pList->size(); i++)
    if (pList->index[i] == index)
      return &pList->value[i];

  pList->push_back(index, lt_perf_value_range<T>());

  return &pList->value.back();
}

inline lt_error_data *get_error_data(SoaList<uint64_t, lt_error_data> *pList, const uint64_t errorCode, const bool hasDescription, const char *description, const bool hasStackTrace, const uint32_t stackTraceHash, OUT bool *pIsNewEntry, OUT uint64_t *pErrorIndex)
{
  for (size_t i = 0; i < pList->size(); i++)
  {
    if (pList->index[i] == errorCode)
    {
      if (pList->value[i].error.hasDescription != hasDescription)
        continue;

      if (hasDescription && strncmp(pList->value[i].error.description, description, sizeof(pList->value[i].error.description)) != 0)
        continue;

      if (pList->value[i].error.hasStackTrace != hasStackTrace)
        continue;

      if (hasStackTrace && pList->value[i].error.stackTraceHash != stackTraceHash)
        continue;

      *pErrorIndex = i;
      *pIsNewEntry = false;

      return &pList->value[i];
    }
  }

  lt_error_data error;
  error.error.errorCode = errorCode;
  error.error.hasDescription = hasDescription;
  error.error.hasStackTrace = hasStackTrace;

  if (hasDescription)
    strcpy_s(error.error.description, description);

  if (hasStackTrace)
    error.error.stackTraceHash = stackTraceHash;

  *pErrorIndex = pList->size();
  *pIsNewEntry = true;

  pList->push_back(errorCode, std::move(error));

  return &pList->value.back();
}

inline void update_transition_data(lt_transition_data *pTransition, const uint64_t delay)
{
  pTransition->avgDelayS = adjust(pTransition->avgDelayS, to_seconds(delay), pTransition->count);

  if (pTransition->count > 0)
  {
    pTransition->maxDelay = max(pTransition->maxDelay, delay);
    pTransition->minDelay = min(pTransition->minDelay, delay);
  }
  else
  {
    pTransition->maxDelay = delay;
    pTransition->minDelay = delay;
  }

  pTransition->count++;
}

inline void update_operation_transition_data(lt_operation_transition_data *pTransition, const uint64_t delay, const uint64_t operationIndex)
{
  update_transition_data(pTransition, delay);

  // Update Operation Count.
  {
    for (size_t i = 0; i < pTransition->operationIndexCount.size(); i++)
    {
      if (pTransition->operationIndexCount.index[i] == operationIndex)
      {
        pTransition->operationIndexCount.value[i]++;
        return;
      }
    }

    pTransition->operationIndexCount.emplace_back(operationIndex, 1);
  }
}

inline void update_operation_index_counts(SoaList<uint64_t, uint64_t> *pIndices, const uint64_t operationIndex)
{
  for (size_t i = 0; i < pIndices->size(); i++)
  {
    if (pIndices->index[i] == operationIndex)
    {
      pIndices->value[i]++;
      return;
    }
  }

  pIndices->emplace_back(operationIndex, 1);
}

template <typename T>
inline void update_exact_value(lt_global_values_exact<T> *pValue, const T newValue)
{
  pValue->average = adjust(pValue->average, newValue, pValue->count);

  if (pValue->count == 0)
  {
    pValue->minValue = newValue;
    pValue->maxValue = newValue;
  }
  else
  {
    pValue->minValue = min(pValue->minValue, newValue);
    pValue->maxValue = max(pValue->maxValue, newValue);
  }

  pValue->count++;

  for (size_t i = 0; i < pValue->multiple.size(); i++)
  {
    if (pValue->multiple.index[i] == newValue)
    {
      pValue->multiple.value[i]++;
      return;
    }
  }

  for (size_t i = 0; i < pValue->single.size(); i++)
  {
    if (pValue->single[i] == newValue)
    {
      pValue->single.erase(pValue->single.begin() + i);
      pValue->multiple.emplace_back(newValue, 2);
      return;
    }
  }

  pValue->single.push_back(newValue);
}

inline void update_string_value(SoaList<lt_string_value_entry, uint64_t> *pStringValue, const char value[256])
{
  for (size_t i = 0; i < pStringValue->size(); i++)
  {
    if (strncmp(pStringValue->index[i].value, value, sizeof(pStringValue->index[i].value)) == 0)
    {
      pStringValue->value[i]++;
      return;
    }
  }

  lt_string_value_entry entry;
  strncpy(entry.value, value, sizeof(entry.value));

  pStringValue->emplace_back(std::move(entry), 1);
}

inline void update_vec2u32_value(SoaList<lt_vec2<uint32_t>, uint64_t> *pValue, const uint32_t x, const uint32_t y)
{
  for (size_t i = 0; i < pValue->size(); i++)
  {
    if (pValue->index[i].x == x && pValue->index[i].y == y)
    {
      pValue->value[i]++;
      return;
    }
  }

  lt_vec2<uint32_t> entry = { x, y };

  pValue->emplace_back(std::move(entry), 1);
}

template <typename T>
inline void update_value(SoaList<T, uint64_t> *pValue, const T v)
{
  for (size_t i = 0; i < pValue->size(); i++)
  {
    if (pValue->index[i] == v)
    {
      pValue->value[i]++;
      return;
    }
  }

  pValue->emplace_back(v, 1);
}

template <typename T>
inline bool update_value_range(lt_global_value_range<T> *pValue, const T newValue)
{
  bool boundsChanged = false;

  pValue->average = adjust(pValue->average, newValue, pValue->count);

  if (pValue->count == 0)
  {
    pValue->minValue = newValue;
    pValue->maxValue = newValue;

    boundsChanged = true;
  }
  else
  {
    if (newValue < pValue->minValue)
    {
      pValue->minValue = newValue;
      boundsChanged = true;
    }

    if (newValue > pValue->maxValue)
    {
      pValue->maxValue = newValue;
      boundsChanged = true;
    }
  }

  pValue->count++;

  const double range = (pValue->maxValue - pValue->minValue) / 128.0;

  size_t bestFit = (size_t)-1;
  double bestFitDiff = 0.0;

  for (size_t i = 0; i < pValue->values.size(); i++)
  {
    double diff;

    if (pValue->values.index[i] >= newValue)
      diff = pValue->values.index[i] - (double)newValue;
    else
      diff = (double)newValue - pValue->values.index[i];

    if ((double)diff <= range && (bestFit == (size_t)-1 || bestFitDiff > diff))
    {
      bestFit = i;
      bestFitDiff = diff;
    }
  }

  if (bestFit != -1)
  {
    pValue->values.index[bestFit] = adjust(pValue->values.index[bestFit], (double)newValue, pValue->values.value[bestFit]);
    pValue->values.value[bestFit]++;
  }
  else
  {
    pValue->values.emplace_back((double)newValue, 1);

    while (pValue->values.size() >= 256)
    {
      size_t closestA = (size_t)-1;
      size_t closestB = (size_t)-1;
      double bestDiff = 0.0;

      for (size_t i = 0; i < pValue->values.size(); i++)
      {
        const double a = pValue->values.index[i];

        for (size_t j = i + 1; j < pValue->values.size(); j++)
        {
          const double b = pValue->values.index[j];
          const double diff = a > b ? (a - b) : (b - a);

          if (closestA == (size_t)-1 || diff < bestDiff)
          {
            bestDiff = diff;
            closestA = i;
            closestB = j;
          }
        }
      }

      // Combine A and B.
      pValue->values.index[closestA] = adjust(pValue->values.index[closestA], pValue->values.index[closestB], pValue->values.value[closestA], pValue->values.value[closestB]);
      pValue->values.value[closestA] += pValue->values.value[closestB];

      // Remove B.
      pValue->values.index.erase(pValue->values.index.begin() + closestB);
      pValue->values.value.erase(pValue->values.value.begin() + closestB);
    }
  }

  return boundsChanged;
}

template <typename T>
inline void update_perf_value_range(lt_perf_value_range<T> *pValue, const T value, const uint64_t delay, IN const lt_short_hw_info *pHwInfo)
{
  if (update_value_range(pValue, value))
  {
    if (pValue->minValue == value)
      pValue->minInfo = *pHwInfo;

    if (pValue->maxValue == value)
      pValue->maxInfo = *pHwInfo;
  }

  update_transition_data(&pValue->data, delay);
}

inline void update_hw_info(lt_hw_info_analyze *pAnalyze, IN const lt_hw_info *pInfo)
{
  if (pInfo->hasCpuInfo)
  {
    update_string_value(&pAnalyze->cpuName.values, pInfo->cpuName);
    update_exact_value(&pAnalyze->cpuCores, pInfo->cpuCores);
  }

  if (pInfo->hasRamInfo)
  {
    update_value_range(&pAnalyze->ramTotalPhysical, pInfo->ramTotalPhysical / (1024.0 * 1024.0 * 1024.0));
    update_value_range(&pAnalyze->ramTotalVirtual, pInfo->ramTotalVirtual / (1024.0 * 1024.0 * 1024.0));
    update_value_range(&pAnalyze->ramAvailablePhysical, pInfo->ramAvailablePhysical / (1024.0 * 1024.0 * 1024.0));
    update_value_range(&pAnalyze->ramAvailableVirtual, pInfo->ramAvailableVirtual / (1024.0 * 1024.0 * 1024.0));
  }

  if (pInfo->hasOsInfo)
    update_string_value(&pAnalyze->osName.values, pInfo->osName);

  if (pInfo->hasGpuInfo)
  {
    update_value_range(&pAnalyze->gpuDedicatedVRam, pInfo->gpuDedicatedVRam / (1024.0 * 1024.0 * 1024.0));
    update_value_range(&pAnalyze->gpuSharedVRam, pInfo->gpuSharedVRam / (1024.0 * 1024.0 * 1024.0));
    update_value_range(&pAnalyze->gpuTotalVRam, pInfo->gpuTotalVRam / (1024.0 * 1024.0 * 1024.0));
    update_value_range(&pAnalyze->gpuFreeVRam, pInfo->gpuFreeVRam / (1024.0 * 1024.0 * 1024.0));
    update_value(&pAnalyze->gpuVendorId.values, pInfo->gpuVendorId);
    update_string_value(&pAnalyze->gpuName.values, pInfo->gpuName);
  }

  if (pInfo->hasElevatedInfo)
    update_value(&pAnalyze->elevated.values, (uint8_t)(pInfo->elevated ? 1 : 0));

  if (pInfo->hasLangInfo)
    update_string_value(&pAnalyze->langPrimaryName.values, pInfo->langPrimaryName);

  if (pInfo->hasMonitorInfo)
  {
    update_exact_value(&pAnalyze->monitorCount, pInfo->monitorSizes.size());
    update_vec2u32_value(&pAnalyze->totalMonitorSize.values, pInfo->monitorTotalWidth, pInfo->monitorTotalHeight);

    for (const auto &_item : pInfo->monitorSizes)
    {
      update_vec2u32_value(&pAnalyze->monitorSize.values, _item.width, _item.height);
      update_exact_value(&pAnalyze->monitorDpiAvgXY, _item.dpi);
    }
  }

  if (pInfo->hasStorageInfo)
  {
    update_value_range(&pAnalyze->storageAvailable, pInfo->storageAvailable / (1024.0 * 1024.0 * 1024.0));
    update_value_range(&pAnalyze->storageTotal, pInfo->storageTotal / (1024.0 * 1024.0 * 1024.0));
  }

  if (pInfo->hasDeviceInfo)
  {
    update_string_value(&pAnalyze->deviceManufacturerName.values, pInfo->deviceManufacturerName);

    char model[0x100];
    memcpy(model, pInfo->deviceManufacturerName, sizeof(model));
    strncat_s(model, " - ", 4);
    strncat_s(model, pInfo->deviceModelName, sizeof(pInfo->deviceModelName));
    model[sizeof(model) - 1] = '\0';

    update_string_value(&pAnalyze->deviceManufacturerModelName.values, model);
  }
}

template <typename T>
inline void update_avg_value(lt_avg_data<T> *pAvgData, const T value)
{
  pAvgData->value = adjust(pAvgData->value, value, pAvgData->count);

  if (pAvgData->count > 0)
  {
    pAvgData->maxValue = max(pAvgData->maxValue, value);
    pAvgData->minValue = min(pAvgData->minValue, value);
  }
  else
  {
    pAvgData->maxValue = value;
    pAvgData->minValue = value;
  }

  pAvgData->count++;
}

inline void add_perf_data(std::vector<lt_perf_data> *pPerfData, const size_t index, const double ms, const lt_short_hw_info *pHwInfo, const bool hasLastOperation, const lt_operation_identifier *pLastOperation)
{
  while (pPerfData->size() <= index)
    pPerfData->emplace_back(lt_perf_data());

  lt_perf_data *pData = &(*pPerfData)[index];

  update_avg_value(&pData->timeMs, ms);

  if (pData->timeMs.maxValue == ms)
  {
    pData->maxInfo = *pHwInfo;
    pData->hasMaxLastOperation = hasLastOperation;
    pData->maxLastOperation = *pLastOperation;
  }

  if (pData->timeMs.minValue == ms)
  {
    pData->minInfo = *pHwInfo;
    pData->hasMinLastOperation = hasLastOperation;
    pData->minLastOperation = *pLastOperation;
  }

  size_t i = 0;

  for (; i < ARRAYSIZE(pData->hist) - 1; i++)
    if (ms < lt_perf_data_sizes[i])
      break;

  pData->hist[i]++;
}

#endif lta_op_h__
