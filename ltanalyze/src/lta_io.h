#ifndef lta_io_h__
#define lta_io_h__

#include "lta_types.h"

//////////////////////////////////////////////////////////////////////////

#define SKIP_X64(prefix, name, stream) do { uint64_t v = 0; FATAL_IF(!stream.read(&v), "Insufficient data stream"); } while (0);
#define SKIP_U64(prefix, name, stream) do { uint64_t v = 0; FATAL_IF(!stream.read(&v), "Insufficient data stream"); } while (0);
#define SKIP_I64(prefix, name, stream) do { int64_t v = 0; FATAL_IF(!stream.read(&v), "Insufficient data stream"); } while (0);
#define SKIP_F64(prefix, name, stream) do { double v = 0; FATAL_IF(!stream.read(&v), "Insufficient data stream"); } while (0);
#define SKIP_U32(prefix, name, stream) do { uint32_t v = 0; FATAL_IF(!stream.read(&v), "Insufficient data stream"); } while (0);
#define SKIP_I32(prefix, name, stream) do { int32_t v = 0; FATAL_IF(!stream.read(&v), "Insufficient data stream"); } while (0);
#define SKIP_U8(prefix, name, stream) do { uint8_t v = 0; FATAL_IF(!stream.read(&v), "Insufficient data stream"); } while (0);
#define SKIP_BOOL(prefix, name, stream) do { uint8_t v = 0; FATAL_IF(!stream.read(&v), "Insufficient data stream"); } while (0);

#define SKIP_STRING(prefix, name, stream) do { \
  char buffer256[0x100]; \
  uint8_t length = 0; \
  FATAL_IF(!stream.read(&length), "Insufficient data stream"); \
  if (length > 0) \
  { FATAL_IF(!stream.read(buffer256, length), "Insufficient data stream"); \
  } } while (0);

#define READ(pWriter, value) do { if (!(pWriter)->read(&(value))) return false; } while (0)
#define WRITE(pWriter, value) do { if (!(pWriter)->write(&(value))) return false; } while (0)
#define READ_STRING(pWriter, value) do { uint8_t __len__; if (!(pWriter)->read(&__len__)) return false; if (__len__ >= sizeof(value)) return false; if (!(pWriter)->read((value), __len__)) return false; (value)[__len__] = '\0'; } while (0)
#define WRITE_STRING(pWriter, value) do { \
  const uint8_t __len__ = (uint8_t)min(0xFF, strlen(value)); \
  if (!(pWriter)->write(&__len__)) \
    return false; \
  if (__len__ > 0 && !(pWriter)->write((value), __len__)) \
    return false; \
  } while (0)

//////////////////////////////////////////////////////////////////////////

enum
{
  lt_analyze_file_version = 5,
};

inline bool deserialize(OUT uint8_t *pValue, IN ByteStream *pStream, const uint32_t /* version */)
{
  READ(pStream, *pValue);

  return true;
}

inline bool serialize(IN const uint8_t *pValue, IN StreamWriter *pStream)
{
  WRITE(pStream, *pValue);

  return true;
}

inline bool jsonify(IN const uint8_t *pValue, IN JsonWriter *pWriter)
{
  pWriter->write_value(*pValue);

  return true;
}

template <typename T>
inline bool deserialize(OUT std::vector<T> *pVector, IN ByteStream *pStream, const uint32_t version)
{
  uint64_t count;
  READ(pStream, count);

  for (size_t i = 0; i < count; i++)
  {
    T item;

    if (!deserialize(&item, pStream, version))
      return false;

    pVector->push_back(std::move(item));
  }

  return true;
}

template <typename T>
inline bool serialize(IN const std::vector<T> *pVector, IN StreamWriter *pStream)
{
  const uint64_t count = pVector->size();
  WRITE(pStream, count);

  for (const auto &_item : *pVector)
    if (!serialize(&_item, pStream))
      return false;

  return true;
}

template <typename T>
inline bool jsonify(IN const std::vector<T> *pVector, IN JsonWriter *pWriter)
{
  pWriter->begin_array();

  for (const auto &_item : *pVector)
    if (!jsonify(&_item, pWriter))
      return false;

  pWriter->end();

  return true;
}

template <typename T, typename T2>
inline bool deserialize(OUT SoaList<T, T2> *pList, IN ByteStream *pStream, const uint32_t version)
{
  uint64_t count;
  READ(pStream, count);

  for (size_t i = 0; i < count; i++)
  {
    T index;
    T2 value;

    if (!deserialize(&index, pStream, version))
      return false;

    if (!deserialize(&value, pStream, version))
      return false;

    pList->push_back(std::move(index), std::move(value));
  }

  return true;
}

template <typename T, typename T2>
inline bool serialize(IN const SoaList<T, T2> *pList, IN StreamWriter *pStream)
{
  const uint64_t count = pList->size();
  WRITE(pStream, count);

  for (size_t i = 0; i < pList->size(); i++)
  {
    if (!serialize(&pList->index[i], pStream))
      return false;

    if (!serialize(&pList->value[i], pStream))
      return false;
  }

  return true;
}

template <typename T, typename T2>
inline bool jsonify(IN const SoaList<T, T2> *pList, IN JsonWriter *pWriter)
{
  pWriter->begin_array();

  for (size_t i = 0; i < pList->size(); i++)
  {
    pWriter->begin_body();

    pWriter->write_name("index");

    if (!jsonify(&pList->index[i], pWriter))
      return false;

    pWriter->write_name("value");

    if (!jsonify(&pList->value[i], pWriter))
      return false;

    pWriter->end();
  }

  pWriter->end();

  return true;
}

inline bool deserialize(OUT uint64_t *pValue, IN ByteStream *pStream, const uint32_t /* version */)
{
  READ(pStream, *pValue);

  return true;
}

inline bool serialize(IN const uint64_t *pValue, IN StreamWriter *pStream)
{
  WRITE(pStream, *pValue);

  return true;
}

inline bool jsonify(IN const uint64_t *pValue, IN JsonWriter *pWriter)
{
  pWriter->write_value(*pValue);

  return true;
}

inline bool deserialize(OUT int64_t *pValue, IN ByteStream *pStream, const uint32_t /* version */)
{
  READ(pStream, *pValue);

  return true;
}

inline bool serialize(IN const int64_t *pValue, IN StreamWriter *pStream)
{
  WRITE(pStream, *pValue);

  return true;
}

inline bool jsonify(IN const int64_t *pValue, IN JsonWriter *pWriter)
{
  pWriter->write_value(*pValue);

  return true;
}

inline bool deserialize(OUT double *pValue, IN ByteStream *pStream, const uint32_t /* version */)
{
  READ(pStream, *pValue);

  return true;
}

inline bool serialize(IN const double *pValue, IN StreamWriter *pStream)
{
  WRITE(pStream, *pValue);

  return true;
}

inline bool jsonify(IN const double *pValue, IN JsonWriter *pWriter)
{
  pWriter->write_value(*pValue);

  return true;
}

inline bool deserialize(OUT uint32_t *pValue, IN ByteStream *pStream, const uint32_t /* version */)
{
  READ(pStream, *pValue);

  return true;
}

inline bool serialize(IN const uint32_t *pValue, IN StreamWriter *pStream)
{
  WRITE(pStream, *pValue);

  return true;
}

inline bool jsonify(IN const uint32_t *pValue, IN JsonWriter *pWriter)
{
  pWriter->write_value(*pValue);

  return true;
}

inline bool deserialize(OUT int32_t *pValue, IN ByteStream *pStream, const uint32_t /* version */)
{
  READ(pStream, *pValue);

  return true;
}

inline bool serialize(IN const int32_t *pValue, IN StreamWriter *pStream)
{
  WRITE(pStream, *pValue);

  return true;
}

inline bool jsonify(IN const int32_t *pValue, IN JsonWriter *pWriter)
{
  pWriter->write_value(*pValue);

  return true;
}

inline bool deserialize(OUT lt_state_identifier *pId, IN ByteStream *pStream, const uint32_t /* version */)
{
  READ(pStream, pId->stateIndex);
  READ(pStream, pId->subStateIndex);

  return true;
}

inline bool serialize(IN const lt_state_identifier *pId, IN StreamWriter *pStream)
{
  WRITE(pStream, pId->stateIndex);
  WRITE(pStream, pId->subStateIndex);

  return true;
}

inline bool jsonify(IN const lt_state_identifier *pValue, IN JsonWriter *pWriter)
{
  pWriter->begin_body();

  pWriter->write("state", pValue->stateIndex);
  pWriter->write("subState", pValue->subStateIndex);

  pWriter->end();

  return true;
}

inline bool deserialize(OUT lt_transition_data *pData, IN ByteStream *pStream, const uint32_t /* version */)
{
  READ(pStream, pData->avgDelayS);
  READ(pStream, pData->count);
  READ(pStream, pData->minDelay);
  READ(pStream, pData->maxDelay);

  return true;
}

inline bool serialize(IN const lt_transition_data *pData, IN StreamWriter *pStream)
{
  WRITE(pStream, pData->avgDelayS);
  WRITE(pStream, pData->count);
  WRITE(pStream, pData->minDelay);
  WRITE(pStream, pData->maxDelay);

  return true;
}

inline bool jsonify(IN const lt_transition_data *pValue, IN JsonWriter *pWriter)
{
  pWriter->begin_body();

  pWriter->write("avgDelay", pValue->avgDelayS);
  pWriter->write("minDelay", to_seconds(pValue->minDelay));
  pWriter->write("maxDelay", to_seconds(pValue->maxDelay));
  pWriter->write("count", pValue->count);

  pWriter->end();

  return true;
}

inline bool deserialize(OUT lt_operation_identifier *pId, IN ByteStream *pStream, const uint32_t /* version */)
{
  READ(pStream, pId->operationType);

  return true;
}

inline bool serialize(IN const lt_operation_identifier *pId, IN StreamWriter *pStream)
{
  WRITE(pStream, pId->operationType);

  return true;
}

inline bool jsonify(IN const lt_operation_identifier *pValue, IN JsonWriter *pWriter)
{
  pWriter->write_value(pValue->operationType);

  return true;
}

inline bool deserialize(OUT lt_operation_transition_data *pData, IN ByteStream *pStream, const uint32_t version)
{
  if (!deserialize(static_cast<lt_transition_data *>(pData), pStream, version))
    return false;

  if (!deserialize(&pData->operationIndexCount, pStream, version))
    return false;

  return true;
}

inline bool serialize(IN const lt_operation_transition_data *pData, IN StreamWriter *pStream)
{
  if (!serialize(static_cast<const lt_transition_data *>(pData), pStream))
    return false;

  if (!serialize(&pData->operationIndexCount, pStream))
    return false;

  return true;
}

inline bool jsonify(IN const lt_operation_transition_data *pValue, IN JsonWriter *pWriter)
{
  pWriter->begin_body();

  pWriter->write("avgDelay", pValue->avgDelayS);
  pWriter->write("minDelay", to_seconds(pValue->minDelay));
  pWriter->write("maxDelay", to_seconds(pValue->maxDelay));
  pWriter->write("count", pValue->count);

  pWriter->write_name("operations");

  if (!jsonify(&pValue->operationIndexCount, pWriter))
    return false;

  pWriter->end();

  return true;
}

template <typename T>
inline bool deserialize(OUT lt_avg_data<T> *pData, IN ByteStream *pStream, const uint32_t version)
{
  if (version < 2)
    return false;

  READ(pStream, pData->count);
  READ(pStream, pData->value);
  READ(pStream, pData->minValue);
  READ(pStream, pData->maxValue);

  return true;
}

template <typename T>
inline bool serialize(IN const lt_avg_data<T> *pData, IN StreamWriter *pStream)
{
  WRITE(pStream, pData->count);
  WRITE(pStream, pData->value);
  WRITE(pStream, pData->minValue);
  WRITE(pStream, pData->maxValue);

  return true;
}

template <typename T>
inline bool jsonify(IN const lt_avg_data<T> *pData, IN JsonWriter *pWriter)
{
  pWriter->begin_body();

  pWriter->write("count", pData->count);
  pWriter->write("value", pData->value);
  pWriter->write("min", pData->minValue);
  pWriter->write("max", pData->maxValue);

  pWriter->end();

  return true;
}

inline bool deserialize(OUT lt_short_hw_info *pData, IN ByteStream *pStream, const uint32_t version)
{
  if (version < 2)
    return false;

  READ_STRING(pStream, pData->cpuName);
  READ(pStream, pData->cpuCores);
  READ(pStream, pData->freeRam);
  READ(pStream, pData->totalRam);
  READ(pStream, pData->freeVRam);
  READ(pStream, pData->dedicatedVRam);
  READ(pStream, pData->totalVRam);
  READ_STRING(pStream, pData->gpuName);
  READ_STRING(pStream, pData->osName);
  READ(pStream, pData->monitorCount);
  READ(pStream, pData->monitorTotalWidth);
  READ(pStream, pData->monitorTotalHeight);

  return true;
}

inline bool serialize(IN const lt_short_hw_info *pData, IN StreamWriter *pStream)
{
  WRITE_STRING(pStream, pData->cpuName);
  WRITE(pStream, pData->cpuCores);
  WRITE(pStream, pData->freeRam);
  WRITE(pStream, pData->totalRam);
  WRITE_STRING(pStream, pData->gpuName);
  WRITE(pStream, pData->freeVRam);
  WRITE(pStream, pData->dedicatedVRam);
  WRITE(pStream, pData->totalVRam);
  WRITE_STRING(pStream, pData->osName);
  WRITE(pStream, pData->monitorCount);
  WRITE(pStream, pData->monitorTotalWidth);
  WRITE(pStream, pData->monitorTotalHeight);

  return true;
}

inline bool jsonify(IN const lt_short_hw_info *pData, IN JsonWriter *pWriter)
{
  pWriter->begin_body();

  pWriter->write("cpu", pData->cpuName);
  pWriter->write("cpuCores", pData->cpuCores);
  pWriter->write("freeRam", pData->freeRam);
  pWriter->write("totalRam", pData->totalRam);
  pWriter->write("gpu", pData->gpuName);
  pWriter->write("freeVRam", pData->freeVRam);
  pWriter->write("dedicatedVRam", pData->dedicatedVRam);
  pWriter->write("totalVRam", pData->totalVRam);
  pWriter->write("os", pData->osName);
  pWriter->write("monitorCount", pData->monitorCount);
  pWriter->write("multiMonitorWidth", pData->monitorTotalWidth);
  pWriter->write("multiMonitorHeight", pData->monitorTotalHeight);

  pWriter->end();

  return true;
}

inline bool deserialize(OUT lt_perf_data *pData, IN ByteStream *pStream, const uint32_t version)
{
  if (version < 2)
    return false;

  if (!pStream->read(pData->hist, ARRAYSIZE(pData->hist)))
    return false;

  if (!deserialize(&pData->timeMs, pStream, version))
    return false;

  if (!deserialize(&pData->minInfo, pStream, version))
    return false;

  if (!deserialize(&pData->maxInfo, pStream, version))
    return false;

  if (version >= 3)
  {
    READ(pStream, pData->hasMinLastOperation);

    if (pData->hasMinLastOperation)
      if (!deserialize(&pData->minLastOperation, pStream, version))
        return false;

    READ(pStream, pData->hasMaxLastOperation);

    if (pData->hasMaxLastOperation)
      if (!deserialize(&pData->maxLastOperation, pStream, version))
        return false;
  }

  return true;
}

inline bool serialize(IN const lt_perf_data *pData, IN StreamWriter *pStream)
{
  if (!pStream->write(pData->hist, ARRAYSIZE(pData->hist)))
    return false;

  if (!serialize(&pData->timeMs, pStream))
    return false;

  if (!serialize(&pData->minInfo, pStream))
    return false;

  if (!serialize(&pData->minInfo, pStream))
    return false;

  WRITE(pStream, pData->hasMinLastOperation);

  if (pData->hasMinLastOperation)
    if (!serialize(&pData->minLastOperation, pStream))
      return false;

  WRITE(pStream, pData->hasMaxLastOperation);

  if (pData->hasMaxLastOperation)
    if (!serialize(&pData->maxLastOperation, pStream))
      return false;

  return true;
}

inline bool jsonify(IN const lt_perf_data *pData, IN JsonWriter *pWriter)
{
  pWriter->begin_body();

  pWriter->begin_array("histogram");

  for (size_t i = 0; i < ARRAYSIZE(pData->hist); i++)
    pWriter->write_value(pData->hist[i]);

  pWriter->end();

  pWriter->write_name("timeMs");

  if (!jsonify(&pData->timeMs, pWriter))
    return false;

  pWriter->write_name("minInfo");

  if (!jsonify(&pData->minInfo, pWriter))
    return false;

  pWriter->write_name("maxInfo");

  if (!jsonify(&pData->maxInfo, pWriter))
    return false;

  pWriter->write_name("minLastOperation");

  if (pData->hasMinLastOperation)
  {
    if (!jsonify(&pData->minLastOperation, pWriter))
      return false;
  }
  else
  {
    pWriter->begin_body();
    pWriter->end();
  }

  pWriter->write_name("maxLastOperation");

  if (pData->hasMaxLastOperation)
  {
    if (!jsonify(&pData->maxLastOperation, pWriter))
      return false;
  }
  else
  {
    pWriter->begin_body();
    pWriter->end();
  }

  pWriter->end();

  return true;
}

inline bool deserialize(OUT lt_state *pValue, IN ByteStream *pStream, const uint32_t version)
{
  if (!deserialize(&pValue->data, pStream, version))
    return false;

  READ(pStream, pValue->avgTimeSinceStartS);

  if (!deserialize(&pValue->previousState, pStream, version))
    return false;

  if (!deserialize(&pValue->nextState, pStream, version))
    return false;

  if (!deserialize(&pValue->operations, pStream, version))
    return false;

  if (!deserialize(&pValue->previousOperation, pStream, version))
    return false;

  if (!deserialize(&pValue->stateReach, pStream, version))
    return false;

  if (!deserialize(&pValue->operationReach, pStream, version))
    return false;

  if (version >= 2)
    if (!deserialize(&pValue->profilerData, pStream, version))
      return false;

  return true;
}

inline bool serialize(IN const lt_state *pValue, IN StreamWriter *pStream)
{
  if (!serialize(&pValue->data, pStream))
    return false;

  WRITE(pStream, pValue->avgTimeSinceStartS);

  if (!serialize(&pValue->previousState, pStream))
    return false;

  if (!serialize(&pValue->nextState, pStream))
    return false;

  if (!serialize(&pValue->operations, pStream))
    return false;

  if (!serialize(&pValue->previousOperation, pStream))
    return false;

  if (!serialize(&pValue->stateReach, pStream))
    return false;

  if (!serialize(&pValue->operationReach, pStream))
    return false;

  if (!serialize(&pValue->profilerData, pStream))
    return false;

  return true;
}

inline bool jsonify(IN const lt_state *pValue, IN JsonWriter *pWriter)
{
  pWriter->begin_body();

  pWriter->write("avgDelay", pValue->data.avgDelayS);
  pWriter->write("minDelay", to_seconds(pValue->data.minDelay));
  pWriter->write("maxDelay", to_seconds(pValue->data.maxDelay));
  pWriter->write("count", pValue->data.count);
  pWriter->write("avgStartDelay", pValue->avgTimeSinceStartS);

  pWriter->write_name("previousState");

  if (!jsonify(&pValue->previousState, pWriter))
    return false;

  pWriter->write_name("nextState");

  if (!jsonify(&pValue->nextState, pWriter))
    return false;

  pWriter->write_name("operations");

  if (!jsonify(&pValue->operations, pWriter))
    return false;

  pWriter->write_name("previousOperation");

  if (!jsonify(&pValue->previousOperation, pWriter))
    return false;

  pWriter->write_name("stateReach");

  if (!jsonify(&pValue->stateReach, pWriter))
    return false;

  pWriter->write_name("operationReach");

  if (!jsonify(&pValue->operationReach, pWriter))
    return false;

  pWriter->write_name("profileData");

  if (!jsonify(&pValue->profilerData, pWriter))
    return false;

  pWriter->end();

  return true;
}

inline bool deserialize(OUT lt_operation *pValue, IN ByteStream *pStream, const uint32_t version)
{
  if (!deserialize(&pValue->data, pStream, version))
    return false;

  READ(pStream, pValue->avgTimeSinceStartS);

  if (!deserialize(&pValue->operationIndexCount, pStream, version))
    return false;

  if (!deserialize(&pValue->parentState, pStream, version))
    return false;

  if (!deserialize(&pValue->nextState, pStream, version))
    return false;

  if (!deserialize(&pValue->lastOperation, pStream, version))
    return false;

  if (!deserialize(&pValue->nextOperation, pStream, version))
    return false;

  return true;
}

inline bool serialize(IN const lt_operation *pValue, IN StreamWriter *pStream)
{
  if (!serialize(&pValue->data, pStream))
    return false;

  WRITE(pStream, pValue->avgTimeSinceStartS);

  if (!serialize(&pValue->operationIndexCount, pStream))
    return false;

  if (!serialize(&pValue->parentState, pStream))
    return false;

  if (!serialize(&pValue->nextState, pStream))
    return false;

  if (!serialize(&pValue->lastOperation, pStream))
    return false;

  if (!serialize(&pValue->nextOperation, pStream))
    return false;

  return true;
}

inline bool jsonify(IN const lt_operation *pValue, IN JsonWriter *pWriter)
{
  pWriter->begin_body();

  pWriter->write("avgDelay", pValue->data.avgDelayS);
  pWriter->write("minDelay", to_seconds(pValue->data.minDelay));
  pWriter->write("maxDelay", to_seconds(pValue->data.maxDelay));
  pWriter->write("count", pValue->data.count);
  pWriter->write("avgStartDelay", pValue->avgTimeSinceStartS);

  pWriter->write_name("operationIndexCount");

  if (!jsonify(&pValue->operationIndexCount, pWriter))
    return false;

  pWriter->write_name("parentState");

  if (!jsonify(&pValue->parentState, pWriter))
    return false;

  pWriter->write_name("nextState");

  if (!jsonify(&pValue->nextState, pWriter))
    return false;

  pWriter->write_name("lastOperation");

  if (!jsonify(&pValue->lastOperation, pWriter))
    return false;

  pWriter->write_name("nextOperation");

  if (!jsonify(&pValue->nextOperation, pWriter))
    return false;

  pWriter->end();

  return true;
}

inline bool deserialize(OUT lt_sub_system_data *pData, IN ByteStream *pStream, const uint32_t version)
{
  if (!deserialize(&pData->states, pStream, version))
    return false;

  if (!deserialize(&pData->operations, pStream, version))
    return false;

  if (!deserialize(&pData->profilerData, pStream, version))
    return false;

  return true;
}

inline bool serialize(IN const lt_sub_system_data *pData, IN StreamWriter *pStream)
{
  if (!serialize(&pData->states, pStream))
    return false;

  if (!serialize(&pData->operations, pStream))
    return false;

  if (!serialize(&pData->profilerData, pStream))
    return false;

  return true;
}

inline bool jsonify(IN const lt_sub_system_data *pValue, IN JsonWriter *pWriter)
{
  pWriter->begin_body();

  pWriter->write_name("states");

  if (!jsonify(&pValue->states, pWriter))
    return false;

  pWriter->write_name("operations");

  if (!jsonify(&pValue->operations, pWriter))
    return false;

  pWriter->write_name("profileData");

  if (!jsonify(&pValue->profilerData, pWriter))
    return false;

  pWriter->end();

  return true;
}

template <typename T>
inline bool deserialize(OUT lt_global_values_exact<T> *pData, IN ByteStream *pStream, const uint32_t version)
{
  if (version < 4)
    return false;

  READ(pStream, pData->average);
  READ(pStream, pData->minValue);
  READ(pStream, pData->maxValue);
  READ(pStream, pData->count);

  if (!deserialize(&pData->single, pStream, version))
    return false;

  if (!deserialize(&pData->multiple, pStream, version))
    return false;

  return true;
}

template <typename T>
inline bool serialize(IN const lt_global_values_exact<T> *pData, IN StreamWriter *pStream)
{
  WRITE(pStream, pData->average);
  WRITE(pStream, pData->minValue);
  WRITE(pStream, pData->maxValue);
  WRITE(pStream, pData->count);

  if (!serialize(&pData->single, pStream))
    return false;

  if (!serialize(&pData->multiple, pStream))
    return false;

  return true;
}

template <typename T>
inline bool jsonify(IN const lt_global_values_exact<T> *pValue, IN JsonWriter *pWriter)
{
  pWriter->begin_body();

  pWriter->write("average", pValue->average);
  pWriter->write("min", pValue->minValue);
  pWriter->write("max", pValue->maxValue);
  pWriter->write("count", pValue->count);

  SoaList<T, uint64_t> sorted;
  sorted.index = pValue->multiple.index;
  sorted.value = pValue->multiple.value;

  if (sorted.size() < 16)
    for (size_t i = 0; i < pValue->single.size() && sorted.size() < 16; i++)
      sorted.emplace_back(pValue->single[i], 1);

  sort_by_value(&sorted);

  pWriter->begin_array("values");

  for (size_t i = 0; i < sorted.size() && i < 16; i++)
  {
    pWriter->begin_body();
    pWriter->write("value", sorted.index[i]);
    pWriter->write("count", sorted.value[i]);
    pWriter->end();
  }

  pWriter->end();

  pWriter->end();

  return true;
}

template <typename T>
inline bool deserialize(OUT lt_values_exact<T> *pData, IN ByteStream *pStream, const uint32_t version)
{
  if (version < 4)
    return false;

  READ(pStream, pData->average);
  READ(pStream, pData->minValue);
  READ(pStream, pData->maxValue);
  READ(pStream, pData->count);

  if (!deserialize(&pData->single, pStream, version))
    return false;

  if (!deserialize(&pData->multiple, pStream, version))
    return false;

  if (!deserialize(&pData->data, pStream, version))
    return false;

  return true;
}

template <typename T>
inline bool serialize(IN const lt_values_exact<T> *pData, IN StreamWriter *pStream)
{
  WRITE(pStream, pData->average);
  WRITE(pStream, pData->minValue);
  WRITE(pStream, pData->maxValue);
  WRITE(pStream, pData->count);

  if (!serialize(&pData->single, pStream))
    return false;

  if (!serialize(&pData->multiple, pStream))
    return false;

  if (!serialize(&pData->data, pStream))
    return false;

  return true;
}

template <typename T>
inline bool jsonify(IN const lt_values_exact<T> *pValue, IN JsonWriter *pWriter)
{
  pWriter->begin_body();

  pWriter->write_name("data");

  if (!jsonify(&pValue->data, pWriter))
    return false;

  pWriter->write("average", pValue->average);
  pWriter->write("min", pValue->minValue);
  pWriter->write("max", pValue->maxValue);
  pWriter->write("count", pValue->count);

  SoaList<T, uint64_t> sorted;
  sorted.index = pValue->multiple.index;
  sorted.value = pValue->multiple.value;

  if (sorted.size() < 16)
    for (size_t i = 0; i < pValue->single.size() && sorted.size() < 16; i++)
      sorted.emplace_back(pValue->single[i], 1);

  sort_by_value(&sorted);

  pWriter->begin_array("values");

  for (size_t i = 0; i < sorted.size() && i < 16; i++)
  {
    pWriter->begin_body();
    pWriter->write("value", sorted.index[i]);
    pWriter->write("count", sorted.value[i]);
    pWriter->end();
  }

  pWriter->end();

  pWriter->end();

  return true;
}

inline bool deserialize(OUT lt_string_value_entry *pData, IN ByteStream *pStream, const uint32_t version)
{
  if (version < 4)
    return false;

  READ_STRING(pStream, pData->value);

  return true;
}

inline bool serialize(IN const lt_string_value_entry *pData, IN StreamWriter *pStream)
{
  WRITE_STRING(pStream, pData->value);

  return true;
}

inline bool jsonify(IN const lt_string_value_entry *pValue, IN JsonWriter *pWriter)
{
  pWriter->write_value(pValue->value);

  return true;
}

template <typename T>
inline bool deserialize(OUT lt_vec2<T> *pData, IN ByteStream *pStream, const uint32_t version)
{
  if (version < 5)
    return false;

  READ(pStream, pData->x);
  READ(pStream, pData->y);

  return true;
}

template <typename T>
inline bool serialize(IN const lt_vec2<T> *pData, IN StreamWriter *pStream)
{
  WRITE(pStream, pData->x);
  WRITE(pStream, pData->y);

  return true;
}

template <typename T>
inline bool jsonify(IN const lt_vec2<T> *pValue, IN JsonWriter *pWriter)
{
  pWriter->begin_body();

  pWriter->write("x", pValue->x);
  pWriter->write("y", pValue->y);

  pWriter->end();

  return true;
}

template <typename T>
inline bool deserialize(OUT lt_global_value_range<T> *pData, IN ByteStream *pStream, const uint32_t version)
{
  if (version < 5)
    return false;

  READ(pStream, pData->average);
  READ(pStream, pData->minValue);
  READ(pStream, pData->maxValue);
  READ(pStream, pData->count);

  if (!deserialize(&pData->values, pStream, version))
    return false;

  return true;
}

template <typename T>
inline bool serialize(IN const lt_global_value_range<T> *pData, IN StreamWriter *pStream)
{
  WRITE(pStream, pData->average);
  WRITE(pStream, pData->minValue);
  WRITE(pStream, pData->maxValue);
  WRITE(pStream, pData->count);

  if (!serialize(&pData->values, pStream))
    return false;

  return true;
}

template <typename T>
inline bool jsonify_internal(IN const lt_global_value_range<T> *pData, IN JsonWriter *pWriter)
{
  pWriter->write("average", pData->average);
  pWriter->write("min", pData->minValue);
  pWriter->write("max", pData->maxValue);
  pWriter->write("count", pData->count);

  pWriter->begin_array("histogram");

  if (pData->values.size() <= 1)
  {
    if (pData->values.size() > 0)
      pWriter->write_value(pData->values.value.front());
  }
  else
  {
    uint64_t histogram[64] = {};
    double histval[63];
    const double epsilon = DBL_EPSILON * (pData->maxValue - pData->minValue);

    for (size_t i = 0; i < 63; i++)
      histval[i] = lerp((double)pData->minValue, (double)pData->maxValue, i / (double)(63)) + epsilon;

    for (size_t i = 0; i < pData->values.size(); i++)
    {
      const double v = pData->values.index[i];
      size_t j = 0;

      for (; j < 63; j++)
        if (histval[j] >= v)
          break;

      histogram[j] += pData->values.value[i];
    }

    for (size_t i = 0; i < 64; i++)
      pWriter->write_value(histogram[i]);
  }

  pWriter->end();

  return true;
}

template <typename T>
inline bool jsonify(IN const lt_global_value_range<T> *pData, IN JsonWriter *pWriter)
{
  pWriter->begin_body();

  if (!jsonify_internal(pData, pWriter))
    return false;

  pWriter->end();

  return true;
}

template <typename T>
inline bool deserialize(OUT lt_value_range<T> *pData, IN ByteStream *pStream, const uint32_t version)
{
  if (version < 5)
    return false;

  if (!deserialize(static_cast<lt_global_value_range<T> *>(pData), pStream, version))
    return false;

  if (!deserialize(&pData->data, pStream, version))
    return false;

  return true;
}

template <typename T>
inline bool serialize(IN const lt_value_range<T> *pData, IN StreamWriter *pStream)
{
  if (!serialize(static_cast<const lt_global_value_range<T> *>(pData), pStream))
    return false;

  if (!serialize(&pData->data, pStream))
    return false;

  return true;
}

template <typename T>
inline bool jsonify(IN const lt_value_range<T> *pData, IN JsonWriter *pWriter)
{
  pWriter->begin_body();

  if (!jsonify_internal(static_cast<const lt_global_value_range<T> *>(pData), pWriter))
    return false;

  pWriter->write_name("data");

  if (!jsonify(&pData->data, pWriter))
    return false;

  pWriter->end();

  return true;
}

template <typename T>
inline bool deserialize(OUT lt_perf_value_range<T> *pData, IN ByteStream *pStream, const uint32_t version)
{
  if (version < 5)
    return false;

  if (!deserialize(static_cast<lt_value_range<T> *>(pData), pStream, version))
    return false;

  if (!deserialize(&pData->minInfo, pStream, version))
    return false;

  if (!deserialize(&pData->maxInfo, pStream, version))
    return false;

  return true;
}

template <typename T>
inline bool serialize(IN const lt_perf_value_range<T> *pData, IN StreamWriter *pStream)
{
  if (!serialize(static_cast<const lt_value_range<T> *>(pData), pStream))
    return false;

  if (!serialize(&pData->minInfo, pStream))
    return false;

  if (!serialize(&pData->maxInfo, pStream))
    return false;

  return true;
}

template <typename T>
inline bool jsonify(IN const lt_perf_value_range<T> *pData, IN JsonWriter *pWriter)
{
  pWriter->begin_body();

  if (!jsonify_internal(static_cast<const lt_global_value_range<T> *>(pData), pWriter))
    return false;

  pWriter->write_name("data");

  if (!jsonify(&pData->data, pWriter))
    return false;

  pWriter->write_name("minInfo");

  if (!jsonify(&pData->minInfo, pWriter))
    return false;

  pWriter->write_name("maxInfo");

  if (!jsonify(&pData->maxInfo, pWriter))
    return false;

  pWriter->end();

  return true;
}

template <typename T>
inline bool deserialize(OUT lt_values<T> *pData, IN ByteStream *pStream, const uint32_t version)
{
  if (version < 4)
    return false;

  if (!deserialize(&pData->data, pStream, version))
    return false;

  if (!deserialize(&pData->values, pStream, version))
    return false;

  return true;
}

template <typename T>
inline bool serialize(IN const lt_values<T> *pData, IN StreamWriter *pStream)
{
  if (!serialize(&pData->data, pStream))
    return false;

  if (!serialize(&pData->values, pStream))
    return false;

  return true;
}

template <typename T>
inline bool jsonify(IN const lt_values<T> *pValue, IN JsonWriter *pWriter)
{
  pWriter->begin_body();

  pWriter->write_name("data");

  if (!jsonify(&pValue->data, pWriter))
    return false;

  uint64_t count = 0;

  for (const auto &_item : pValue->values.value)
    count += _item;

  pWriter->write("count", count);

  SoaList<T, uint64_t> sorted;
  sorted.index = pValue->values.index;
  sorted.value = pValue->values.value;

  sort_by_value(&sorted);

  pWriter->begin_array("values");

  for (size_t i = 0; i < sorted.size() && i < 16; i++)
  {
    pWriter->begin_body();
    pWriter->write("value", sorted.index[i].value);
    pWriter->write("count", sorted.value[i]);
    pWriter->end();
  }

  pWriter->end();

  pWriter->end();

  return true;
}

template <typename T>
inline bool deserialize(OUT lt_global_values<T> *pData, IN ByteStream *pStream, const uint32_t version)
{
  if (version < 4)
    return false;

  if (!deserialize(&pData->values, pStream, version))
    return false;

  return true;
}

template <typename T>
inline bool serialize(IN const lt_global_values<T> *pData, IN StreamWriter *pStream)
{
  if (!serialize(&pData->values, pStream))
    return false;

  return true;
}

template <typename T>
inline bool jsonify(IN const lt_global_values<T> *pValue, IN JsonWriter *pWriter)
{
  pWriter->begin_body();

  uint64_t count = 0;

  for (const auto &_item : pValue->values.value)
    count += _item;

  pWriter->write("count", count);

  SoaList<T, uint64_t> sorted;
  sorted.index = pValue->values.index;
  sorted.value = pValue->values.value;

  sort_by_value(&sorted);

  pWriter->begin_array("values");

  for (size_t i = 0; i < sorted.size() && i < 16; i++)
  {
    pWriter->begin_body();
    pWriter->write_name("value");

    const T val = sorted.index[i];

    if (!jsonify(&val, pWriter))
      return false;

    pWriter->write("count", sorted.value[i]);
    pWriter->end();
  }

  pWriter->end();

  pWriter->end();

  return true;
}

inline bool deserialize(OUT lt_hw_info_analyze *pData, IN ByteStream *pStream, const uint32_t version)
{
  if (version < 5)
    return false;

  if (!deserialize(&pData->cpuName, pStream, version))
    return false;

  if (!deserialize(&pData->cpuCores, pStream, version))
    return false;

  if (!deserialize(&pData->ramTotalPhysical, pStream, version))
    return false;

  if (!deserialize(&pData->ramTotalVirtual, pStream, version))
    return false;

  if (!deserialize(&pData->ramAvailablePhysical, pStream, version))
    return false;

  if (!deserialize(&pData->ramAvailableVirtual, pStream, version))
    return false;

  if (!deserialize(&pData->osName, pStream, version))
    return false;

  if (!deserialize(&pData->gpuDedicatedVRam, pStream, version))
    return false;

  if (!deserialize(&pData->gpuSharedVRam, pStream, version))
    return false;

  if (!deserialize(&pData->gpuTotalVRam, pStream, version))
    return false;

  if (!deserialize(&pData->gpuFreeVRam, pStream, version))
    return false;

  if (!deserialize(&pData->gpuVendorId, pStream, version))
    return false;

  if (!deserialize(&pData->gpuName, pStream, version))
    return false;

  if (!deserialize(&pData->langPrimaryName, pStream, version))
    return false;

  if (!deserialize(&pData->elevated, pStream, version))
    return false;

  if (!deserialize(&pData->monitorCount, pStream, version))
    return false;

  if (!deserialize(&pData->monitorSize, pStream, version))
    return false;

  if (!deserialize(&pData->totalMonitorSize, pStream, version))
    return false;

  if (!deserialize(&pData->monitorDpiAvgXY, pStream, version))
    return false;

  if (!deserialize(&pData->storageAvailable, pStream, version))
    return false;

  if (!deserialize(&pData->storageTotal, pStream, version))
    return false;

  if (!deserialize(&pData->deviceManufacturerName, pStream, version))
    return false;

  if (!deserialize(&pData->deviceManufacturerModelName, pStream, version))
    return false;

  return true;
}

inline bool serialize(IN const lt_hw_info_analyze *pData, IN StreamWriter *pStream)
{
  if (!serialize(&pData->cpuName, pStream))
    return false;

  if (!serialize(&pData->cpuCores, pStream))
    return false;

  if (!serialize(&pData->ramTotalPhysical, pStream))
    return false;

  if (!serialize(&pData->ramTotalVirtual, pStream))
    return false;

  if (!serialize(&pData->ramAvailablePhysical, pStream))
    return false;

  if (!serialize(&pData->ramAvailableVirtual, pStream))
    return false;

  if (!serialize(&pData->osName, pStream))
    return false;

  if (!serialize(&pData->gpuDedicatedVRam, pStream))
    return false;

  if (!serialize(&pData->gpuSharedVRam, pStream))
    return false;

  if (!serialize(&pData->gpuTotalVRam, pStream))
    return false;

  if (!serialize(&pData->gpuFreeVRam, pStream))
    return false;

  if (!serialize(&pData->gpuVendorId, pStream))
    return false;

  if (!serialize(&pData->gpuName, pStream))
    return false;

  if (!serialize(&pData->langPrimaryName, pStream))
    return false;

  if (!serialize(&pData->elevated, pStream))
    return false;

  if (!serialize(&pData->monitorCount, pStream))
    return false;

  if (!serialize(&pData->monitorSize, pStream))
    return false;

  if (!serialize(&pData->totalMonitorSize, pStream))
    return false;

  if (!serialize(&pData->monitorDpiAvgXY, pStream))
    return false;

  if (!serialize(&pData->storageAvailable, pStream))
    return false;

  if (!serialize(&pData->storageTotal, pStream))
    return false;

  if (!serialize(&pData->deviceManufacturerName, pStream))
    return false;

  if (!serialize(&pData->deviceManufacturerModelName, pStream))
    return false;

  return true;
}

inline bool jsonify(IN const lt_hw_info_analyze *pData, IN JsonWriter *pWriter)
{
  pWriter->begin_body();

  pWriter->write_name("cpu");

  if (!jsonify(&pData->cpuName, pWriter))
    return false;

  pWriter->write_name("cpuCores");

  if (!jsonify(&pData->cpuCores, pWriter))
    return false;

  pWriter->write_name("totalPhysicalRam");

  if (!jsonify(&pData->ramTotalPhysical, pWriter))
    return false;

  pWriter->write_name("totalVirtualRam");

  if (!jsonify(&pData->ramTotalVirtual, pWriter))
    return false;

  pWriter->write_name("availablePhysicalRam");

  if (!jsonify(&pData->ramAvailablePhysical, pWriter))
    return false;

  pWriter->write_name("availableVirtualRam");

  if (!jsonify(&pData->ramAvailableVirtual, pWriter))
    return false;

  pWriter->write_name("os");

  if (!jsonify(&pData->osName, pWriter))
    return false;

  pWriter->write_name("gpuDedicatedVRam");

  if (!jsonify(&pData->gpuDedicatedVRam, pWriter))
    return false;

  pWriter->write_name("gpuSharedVRam");

  if (!jsonify(&pData->gpuSharedVRam, pWriter))
    return false;

  pWriter->write_name("gpuTotalVRam");

  if (!jsonify(&pData->gpuTotalVRam, pWriter))
    return false;

  pWriter->write_name("gpuFreeVRam");

  if (!jsonify(&pData->gpuFreeVRam, pWriter))
    return false;

  pWriter->write_name("gpuVendorId");

  if (!jsonify(&pData->gpuVendorId, pWriter))
    return false;

  pWriter->write_name("gpu");

  if (!jsonify(&pData->gpuName, pWriter))
    return false;

  pWriter->write_name("primaryLanguage");

  if (!jsonify(&pData->langPrimaryName, pWriter))
    return false;

  pWriter->write_name("isElevated");

  if (!jsonify(&pData->elevated, pWriter))
    return false;

  pWriter->write_name("monitorCount");

  if (!jsonify(&pData->monitorCount, pWriter))
    return false;

  pWriter->write_name("monitorSize");

  if (!jsonify(&pData->monitorSize, pWriter))
    return false;

  pWriter->write_name("totalMonitorSize");

  if (!jsonify(&pData->totalMonitorSize, pWriter))
    return false;

  pWriter->write_name("monitorDpi");

  if (!jsonify(&pData->monitorDpiAvgXY, pWriter))
    return false;

  pWriter->write_name("availableStorage");

  if (!jsonify(&pData->storageAvailable, pWriter))
    return false;

  pWriter->write_name("totalStorage");

  if (!jsonify(&pData->storageTotal, pWriter))
    return false;

  pWriter->write_name("deviceManufacturer");

  if (!jsonify(&pData->deviceManufacturerName, pWriter))
    return false;

  pWriter->write_name("deviceManufacturerModel");

  if (!jsonify(&pData->deviceManufacturerModelName, pWriter))
    return false;

  pWriter->end();

  return true;
}

inline bool deserialize(OUT lt_analyze *pAnalyze, IN ByteStream *pStream)
{
  uint32_t version;
  READ(pStream, version);

  if (version > lt_analyze_file_version)
    return false;

  READ(pStream, pAnalyze->majorVersion);
  READ(pStream, pAnalyze->minorVersion);
  READ_STRING(pStream, pAnalyze->productName);

  if (!deserialize(&pAnalyze->subSystems, pStream, version))
    return false;

  if (version >= 4)
  {
    if (!deserialize(&pAnalyze->observedU64, pStream, version))
      return false;

    if (!deserialize(&pAnalyze->observedI64, pStream, version))
      return false;

    if (!deserialize(&pAnalyze->observedString, pStream, version))
      return false;
  }

  if (version >= 5)
  {
    if (!deserialize(&pAnalyze->hwInfo, pStream, version))
      return false;

    if (!deserialize(&pAnalyze->observedRangeU64, pStream, version))
      return false;

    if (!deserialize(&pAnalyze->observedRangeI64, pStream, version))
      return false;

    if (!deserialize(&pAnalyze->observedRangeF64, pStream, version))
      return false;

    if (!deserialize(&pAnalyze->perfMetrics, pStream, version))
      return false;
  }

  return true;
}

inline bool serialize(IN const lt_analyze *pAnalyze, OUT StreamWriter *pStream)
{
  const uint32_t version = lt_analyze_file_version;
  WRITE(pStream, version);

  WRITE(pStream, pAnalyze->majorVersion);
  WRITE(pStream, pAnalyze->minorVersion);
  WRITE_STRING(pStream, pAnalyze->productName);

  if (!serialize(&pAnalyze->subSystems, pStream))
    return false;

  if (!serialize(&pAnalyze->observedU64, pStream))
    return false;

  if (!serialize(&pAnalyze->observedI64, pStream))
    return false;

  if (!serialize(&pAnalyze->observedString, pStream))
    return false;

  if (!serialize(&pAnalyze->hwInfo, pStream))
    return false;

  if (!serialize(&pAnalyze->observedRangeU64, pStream))
    return false;

  if (!serialize(&pAnalyze->observedRangeI64, pStream))
    return false;

  if (!serialize(&pAnalyze->observedRangeF64, pStream))
    return false;

  if (!serialize(&pAnalyze->perfMetrics, pStream))
    return false;

  return true;
}

inline bool jsonify(IN const lt_analyze *pAnalyze, OUT JsonWriter *pWriter)
{
  pWriter->begin_body();

  pWriter->write("outputVersion", 1);

  pWriter->write("productName", pAnalyze->productName);
  pWriter->write("majorVersion", pAnalyze->majorVersion);
  pWriter->write("minorVersion", pAnalyze->minorVersion);

  pWriter->write_name("subSystems");

  if (!jsonify(&pAnalyze->subSystems, pWriter))
    return false;

  pWriter->write_name("hwInfo");

  if (!jsonify(&pAnalyze->hwInfo, pWriter))
    return false;

  pWriter->write_name("observedU64");

  if (!jsonify(&pAnalyze->observedU64, pWriter))
    return false;

  pWriter->write_name("observedI64");

  if (!jsonify(&pAnalyze->observedI64, pWriter))
    return false;

  pWriter->write_name("observedString");

  if (!jsonify(&pAnalyze->observedString, pWriter))
    return false;

  pWriter->write_name("observedRangeU64");

  if (!jsonify(&pAnalyze->observedRangeU64, pWriter))
    return false;

  pWriter->write_name("observedRangeI64");

  if (!jsonify(&pAnalyze->observedRangeI64, pWriter))
    return false;

  pWriter->write_name("observedRangeF64");

  if (!jsonify(&pAnalyze->observedRangeF64, pWriter))
    return false;

  pWriter->write_name("perfMetrics");

  if (!jsonify(&pAnalyze->perfMetrics, pWriter))
    return false;

  pWriter->end();

  return true;
}

#endif // lta_io_h__
