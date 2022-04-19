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

#define READ(pWriter, value) \
  do { \
    RETURN_ERROR_IF(!(pWriter)->read(&(value)), "Failed to read value '" #value "'."); \
  } while (0)

#define WRITE(pWriter, value) \
  do { \
    RETURN_ERROR_IF(!(pWriter)->write(&(value)), "Failed to write value '" #value "'."); \
  } while (0)

#define READ_STRING(pWriter, value) \
  do { \
    uint8_t __len__; \
    RETURN_ERROR_IF(!(pWriter)->read(&__len__), "Failed to read length for string '" #value "'."); \
    RETURN_ERROR_IF(__len__ >= sizeof(value), "The retrieved length for string '" #value "' is invalid (%" PRIu8 ").", __len__); \
    RETURN_ERROR_IF(!(pWriter)->read((value), __len__), "Failed to read string '" #value "'."); \
    (value)[__len__] = '\0'; \
    for (size_t __i__ = 0; __i__ < __len__; __i__++) \
      RETURN_ERROR_IF((value)[__i__] == '\0', "The deserialized contents of '" #value "' are not valid."); \
  } while (0)

#define WRITE_STRING(pWriter, value) \
  do { \
    const uint8_t __len__ = (uint8_t)min(0xFF, strlen(value)); \
    RETURN_ERROR_IF(!(pWriter)->write(&__len__), "Failed to write length of string '" #value "'."); \
    RETURN_ERROR_IF(__len__ > 0 && !(pWriter)->write((value), __len__), "Failed to write string '" #value "'."); \
  } while (0)

//////////////////////////////////////////////////////////////////////////

enum
{
  lt_analyze_file_version = 9,
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

  RETURN_ERROR_IF(version < 8, "This version is no longer supported.");
  uint64_t size;
  READ(pStream, size);
  RETURN_ERROR_IF(size < 8, "Invalid Parameter.");
  size -= sizeof(size);

  ByteStream bs(pStream->pData, size);
  RETURN_ERROR_IF(!pStream->read<uint8_t>(nullptr, size), "ByteStream insufficient (%s).", typeid(T).name());

  for (size_t i = 0; i < count; i++)
  {
    T item;

    RETURN_ERROR_IF(!deserialize(&item, &bs, version), "Failed to deserialize (%s).", typeid(T).name());

    pVector->push_back(std::move(item));
  }

  RETURN_ERROR_IF(bs.sizeRemaining > 0, "Size Remaining after container (%s).", typeid(T).name());

  return true;
}

template <typename T>
inline bool serialize(IN const std::vector<T> *pVector, IN StreamWriter *pStream)
{
  const uint64_t count = pVector->size();
  WRITE(pStream, count);

  const uint64_t sizeOffset = pStream->size;
  const uint64_t zero = 0;
  WRITE(pStream, zero);

  for (const auto &_item : *pVector)
    RETURN_ERROR_IF(!serialize(&_item, pStream), "Failed to serialize (%s).", typeid(T).name());

  // Write Size of this container.
  *reinterpret_cast<uint64_t *>(pStream->pBytes + sizeOffset) = pStream->size - sizeOffset;

  return true;
}

template <typename T>
inline bool jsonify(IN const std::vector<T> *pVector, IN JsonWriter *pWriter)
{
  pWriter->begin_array();

  for (const auto &_item : *pVector)
    RETURN_ERROR_IF(!jsonify(&_item, pWriter), "Failed to jsonify (%s).", typeid(T).name());

  pWriter->end();

  return true;
}

template <typename T, typename T2>
inline bool deserialize(OUT SoaList<T, T2> *pList, IN ByteStream *pStream, const uint32_t version)
{
  uint64_t count;
  READ(pStream, count);

  RETURN_ERROR_IF(version < 8, "This version is no longer supported.");
  uint64_t size;
  READ(pStream, size);
  RETURN_ERROR_IF(size < 8, "Invalid Parameter.");
  size -= sizeof(size);

  ByteStream bs(pStream->pData, size);
  RETURN_ERROR_IF(!pStream->read<uint8_t>(nullptr, size), "ByteStream insufficient (%s / %s).", typeid(T).name(), typeid(T2).name());

  for (size_t i = 0; i < count; i++)
  {
    T index;
    T2 value;

    RETURN_ERROR_IF(!deserialize(&index, &bs, version), "Failed to deserialize (%s).", typeid(T).name());
    RETURN_ERROR_IF(!deserialize(&value, &bs, version), "Failed to deserialize (%s).", typeid(T2).name());

    pList->push_back(std::move(index), std::move(value));
  }

  RETURN_ERROR_IF(bs.sizeRemaining > 0, "Size Remaining after container (%s / %s).", typeid(T).name(), typeid(T2).name());

  return true;
}

template <typename T, typename T2>
inline bool serialize(IN const SoaList<T, T2> *pList, IN StreamWriter *pStream)
{
  const uint64_t count = pList->size();
  WRITE(pStream, count);

  const uint64_t sizeOffset = pStream->size;
  const uint64_t zero = 0;
  WRITE(pStream, zero);

  for (size_t i = 0; i < pList->size(); i++)
  {
    RETURN_ERROR_IF(!serialize(&pList->index[i], pStream), "Failed to serialize (%s).", typeid(T).name());
    RETURN_ERROR_IF(!serialize(&pList->value[i], pStream), "Failed to serialize (%s).", typeid(T2).name());
  }

  // Write Size of this container.
  *reinterpret_cast<uint64_t *>(pStream->pBytes + sizeOffset) = pStream->size - sizeOffset;

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
    RETURN_ERROR_IF(!jsonify(&pList->index[i], pWriter), "Failed to jsonify (%s).", typeid(T).name());

    pWriter->write_name("value");
    RETURN_ERROR_IF(!jsonify(&pList->value[i], pWriter), "Failed to jsonify (%s).", typeid(T2).name());

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
  RETURN_ERROR_IF(!deserialize(static_cast<lt_transition_data *>(pData), pStream, version), "Failed to deserialize.");
  RETURN_ERROR_IF(!deserialize(&pData->operationIndexCount, pStream, version), "Failed to deserialize.");

  return true;
}

inline bool serialize(IN const lt_operation_transition_data *pData, IN StreamWriter *pStream)
{
  RETURN_ERROR_IF(!serialize(static_cast<const lt_transition_data *>(pData), pStream), "Failed to serialize.");
  RETURN_ERROR_IF(!serialize(&pData->operationIndexCount, pStream), "Failed to serialize.");

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

  RETURN_ERROR_IF(!jsonify(&pValue->operationIndexCount, pWriter), "Failed to jsonify.");

  pWriter->end();

  return true;
}

template <typename T>
inline bool deserialize(OUT lt_avg_data<T> *pData, IN ByteStream *pStream, const uint32_t version)
{
  RETURN_ERROR_IF(version < 2, "Invalid Version");

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
  RETURN_ERROR_IF(version < 2, "Invalid Version");

  READ_STRING(pStream, pData->cpuName);
  READ(pStream, pData->cpuCores);
  READ(pStream, pData->freeRam);
  READ(pStream, pData->totalRam);
  READ_STRING(pStream, pData->gpuName);
  READ(pStream, pData->freeVRam);
  READ(pStream, pData->dedicatedVRam);
  READ(pStream, pData->totalVRam);
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
  RETURN_ERROR_IF(version < 2, "Invalid Version");

  RETURN_ERROR_IF(!pStream->read(pData->hist, ARRAYSIZE(pData->hist)), "Failed to read data.");

  RETURN_ERROR_IF(!deserialize(&pData->timeMs, pStream, version), "Failed to deserialize.");
  RETURN_ERROR_IF(!deserialize(&pData->minInfo, pStream, version), "Failed to deserialize.");
  RETURN_ERROR_IF(!deserialize(&pData->maxInfo, pStream, version), "Failed to deserialize.");

  if (version >= 3)
  {
    READ(pStream, pData->hasMinLastOperation);

    if (pData->hasMinLastOperation)
      RETURN_ERROR_IF(!deserialize(&pData->minLastOperation, pStream, version), "Failed to deserialize.");

    READ(pStream, pData->hasMaxLastOperation);

    if (pData->hasMaxLastOperation)
      RETURN_ERROR_IF(!deserialize(&pData->maxLastOperation, pStream, version), "Failed to deserialize.");
  }

  return true;
}

inline bool serialize(IN const lt_perf_data *pData, IN StreamWriter *pStream)
{
  RETURN_ERROR_IF(!pStream->write(pData->hist, ARRAYSIZE(pData->hist)), "Failed to write data.");

  RETURN_ERROR_IF(!serialize(&pData->timeMs, pStream), "Failed to serialize.");
  RETURN_ERROR_IF(!serialize(&pData->minInfo, pStream), "Failed to serialize.");
  RETURN_ERROR_IF(!serialize(&pData->minInfo, pStream), "Failed to serialize.");

  WRITE(pStream, pData->hasMinLastOperation);

  if (pData->hasMinLastOperation)
    RETURN_ERROR_IF(!serialize(&pData->minLastOperation, pStream), "Failed to serialize.");

  WRITE(pStream, pData->hasMaxLastOperation);

  if (pData->hasMaxLastOperation)
    RETURN_ERROR_IF(!serialize(&pData->maxLastOperation, pStream), "Failed to serialize.");

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
  RETURN_ERROR_IF(!jsonify(&pData->timeMs, pWriter), "Failed to jsonify.");

  pWriter->write_name("minInfo");
  RETURN_ERROR_IF(!jsonify(&pData->minInfo, pWriter), "Failed to jsonify.");

  pWriter->write_name("maxInfo");
  RETURN_ERROR_IF(!jsonify(&pData->maxInfo, pWriter), "Failed to jsonify.");

  pWriter->write_name("minLastOperation");

  if (pData->hasMinLastOperation)
  {
    RETURN_ERROR_IF(!jsonify(&pData->minLastOperation, pWriter), "Failed to jsonify.");
  }
  else
  {
    pWriter->begin_body();
    pWriter->end();
  }

  pWriter->write_name("maxLastOperation");

  if (pData->hasMaxLastOperation)
  {
    RETURN_ERROR_IF(!jsonify(&pData->maxLastOperation, pWriter), "Failed to jsonify.");
  }
  else
  {
    pWriter->begin_body();
    pWriter->end();
  }

  pWriter->end();

  return true;
}

template <typename T>
inline bool deserialize(OUT lt_global_values<T> *pData, IN ByteStream *pStream, const uint32_t version)
{
  RETURN_ERROR_IF(version < 4, "Invalid Version");

  RETURN_ERROR_IF(!deserialize(&pData->values, pStream, version), "Failed to deserialize.");

  return true;
}

template <typename T>
inline bool serialize(IN const lt_global_values<T> *pData, IN StreamWriter *pStream)
{
  RETURN_ERROR_IF(!serialize(&pData->values, pStream), "Failed to serialize.");

  return true;
}

template <typename T>
inline bool jsonify_internal(IN const lt_global_values<T> *pValue, IN JsonWriter *pWriter)
{
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

    RETURN_ERROR_IF(!jsonify(&val, pWriter), "Failed to jsonify.");

    pWriter->write("count", sorted.value[i]);
    pWriter->end();
  }

  pWriter->end();

  return true;
}

template <typename T>
inline bool jsonify(IN const lt_global_values<T> *pValue, IN JsonWriter *pWriter)
{
  pWriter->begin_body();

  RETURN_ERROR_IF(!jsonify_internal(pValue, pWriter), "Failed to jsonify.");

  pWriter->end();

  return true;
}

template <typename T>
inline bool deserialize(OUT lt_values<T> *pData, IN ByteStream *pStream, const uint32_t version)
{
  RETURN_ERROR_IF(version < 4, "Invalid Version");

  RETURN_ERROR_IF(!deserialize(static_cast<lt_global_values<T> *>(pData), pStream, version), "Failed to deserialize.");
  RETURN_ERROR_IF(!deserialize(&pData->data, pStream, version), "Failed to deserialize.");

  return true;
}

template <typename T>
inline bool serialize(IN const lt_values<T> *pData, IN StreamWriter *pStream)
{
  RETURN_ERROR_IF(!serialize(static_cast<const lt_global_values<T> *>(pData), pStream), "Failed to serialize.");
  RETURN_ERROR_IF(!serialize(&pData->data, pStream), "Failed to serialize.");

  return true;
}

template <typename T>
inline bool jsonify(IN const lt_values<T> *pValue, IN JsonWriter *pWriter)
{
  pWriter->begin_body();

  RETURN_ERROR_IF(!jsonify_internal(static_cast<const lt_global_values<T> *>(pValue), pWriter), "Failed to jsonify.");

  pWriter->write_name("data");
  RETURN_ERROR_IF(!jsonify(&pValue->data, pWriter), "Failed to jsonify.");

  pWriter->end();

  return true;
}

inline bool deserialize(OUT lt_state *pValue, IN ByteStream *pStream, const uint32_t version)
{
  RETURN_ERROR_IF(!deserialize(&pValue->data, pStream, version), "Failed to deserialize.");

  READ(pStream, pValue->avgTimeSinceStartS);

  RETURN_ERROR_IF(!deserialize(&pValue->previousState, pStream, version), "Failed to deserialize.");
  RETURN_ERROR_IF(!deserialize(&pValue->nextState, pStream, version), "Failed to deserialize.");
  RETURN_ERROR_IF(!deserialize(&pValue->operations, pStream, version), "Failed to deserialize.");
  RETURN_ERROR_IF(!deserialize(&pValue->previousOperation, pStream, version), "Failed to deserialize.");
  RETURN_ERROR_IF(!deserialize(&pValue->stateReach, pStream, version), "Failed to deserialize.");
  RETURN_ERROR_IF(!deserialize(&pValue->operationReach, pStream, version), "Failed to deserialize.");
  if (version >= 2)
    RETURN_ERROR_IF(!deserialize(&pValue->profilerData, pStream, version), "Failed to deserialize.");

  if (version >= 6)
  {
    RETURN_ERROR_IF(!deserialize(&pValue->errors, pStream, version), "Failed to deserialize.");
    RETURN_ERROR_IF(!deserialize(&pValue->warnings, pStream, version), "Failed to deserialize.");
  }

  if (version >= 7)
    RETURN_ERROR_IF(!deserialize(&pValue->logs, pStream, version), "Failed to deserialize.");

  return true;
}

inline bool serialize(IN const lt_state *pValue, IN StreamWriter *pStream)
{
  RETURN_ERROR_IF(!serialize(&pValue->data, pStream), "Failed to serialize.");

  WRITE(pStream, pValue->avgTimeSinceStartS);

  RETURN_ERROR_IF(!serialize(&pValue->previousState, pStream), "Failed to serialize.");
  RETURN_ERROR_IF(!serialize(&pValue->nextState, pStream), "Failed to serialize.");
  RETURN_ERROR_IF(!serialize(&pValue->operations, pStream), "Failed to serialize.");
  RETURN_ERROR_IF(!serialize(&pValue->previousOperation, pStream), "Failed to serialize.");
  RETURN_ERROR_IF(!serialize(&pValue->stateReach, pStream), "Failed to serialize.");
  RETURN_ERROR_IF(!serialize(&pValue->operationReach, pStream), "Failed to serialize.");
  RETURN_ERROR_IF(!serialize(&pValue->profilerData, pStream), "Failed to serialize.");
  RETURN_ERROR_IF(!serialize(&pValue->errors, pStream), "Failed to serialize.");
  RETURN_ERROR_IF(!serialize(&pValue->warnings, pStream), "Failed to serialize.");
  RETURN_ERROR_IF(!serialize(&pValue->logs, pStream), "Failed to serialize.");

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
  RETURN_ERROR_IF(!jsonify(&pValue->previousState, pWriter), "Failed to jsonify.");

  pWriter->write_name("nextState");
  RETURN_ERROR_IF(!jsonify(&pValue->nextState, pWriter), "Failed to jsonify.");

  pWriter->write_name("operations");
  RETURN_ERROR_IF(!jsonify(&pValue->operations, pWriter), "Failed to jsonify.");

  pWriter->write_name("previousOperation");
  RETURN_ERROR_IF(!jsonify(&pValue->previousOperation, pWriter), "Failed to jsonify.");

  pWriter->write_name("stateReach");
  RETURN_ERROR_IF(!jsonify(&pValue->stateReach, pWriter), "Failed to jsonify.");

  pWriter->write_name("operationReach");
  RETURN_ERROR_IF(!jsonify(&pValue->operationReach, pWriter), "Failed to jsonify.");

  pWriter->write_name("profileData");
  RETURN_ERROR_IF(!jsonify(&pValue->profilerData, pWriter), "Failed to jsonify.");

  pWriter->write_name("errors");
  RETURN_ERROR_IF(!jsonify(&pValue->errors.value, pWriter), "Failed to jsonify.");

  pWriter->write_name("warnings");
  RETURN_ERROR_IF(!jsonify(&pValue->warnings.value, pWriter), "Failed to jsonify.");

  pWriter->write_name("logs");
  RETURN_ERROR_IF(!jsonify(&pValue->logs, pWriter), "Failed to jsonify.");

  pWriter->end();

  return true;
}

inline bool deserialize(OUT lt_operation *pValue, IN ByteStream *pStream, const uint32_t version)
{
  RETURN_ERROR_IF(!deserialize(&pValue->data, pStream, version), "Failed to deserialize.");

  READ(pStream, pValue->avgTimeSinceStartS);

  RETURN_ERROR_IF(!deserialize(&pValue->operationIndexCount, pStream, version), "Failed to deserialize.");
  RETURN_ERROR_IF(!deserialize(&pValue->parentState, pStream, version), "Failed to deserialize.");
  RETURN_ERROR_IF(!deserialize(&pValue->nextState, pStream, version), "Failed to deserialize.");
  RETURN_ERROR_IF(!deserialize(&pValue->lastOperation, pStream, version), "Failed to deserialize.");
  RETURN_ERROR_IF(!deserialize(&pValue->nextOperation, pStream, version), "Failed to deserialize.");

  if (version >= 6)
  {
    RETURN_ERROR_IF(!deserialize(&pValue->errors, pStream, version), "Failed to deserialize.");
    RETURN_ERROR_IF(!deserialize(&pValue->warnings, pStream, version), "Failed to deserialize.");
  }

  if (version >= 7)
    RETURN_ERROR_IF(!deserialize(&pValue->logs, pStream, version), "Failed to deserialize.");

  return true;
}

inline bool serialize(IN const lt_operation *pValue, IN StreamWriter *pStream)
{
  RETURN_ERROR_IF(!serialize(&pValue->data, pStream), "Failed to serialize.");

  WRITE(pStream, pValue->avgTimeSinceStartS);

  RETURN_ERROR_IF(!serialize(&pValue->operationIndexCount, pStream), "Failed to serialize.");
  RETURN_ERROR_IF(!serialize(&pValue->parentState, pStream), "Failed to serialize.");
  RETURN_ERROR_IF(!serialize(&pValue->nextState, pStream), "Failed to serialize.");
  RETURN_ERROR_IF(!serialize(&pValue->lastOperation, pStream), "Failed to serialize.");
  RETURN_ERROR_IF(!serialize(&pValue->nextOperation, pStream), "Failed to serialize.");
  RETURN_ERROR_IF(!serialize(&pValue->errors, pStream), "Failed to serialize.");
  RETURN_ERROR_IF(!serialize(&pValue->warnings, pStream), "Failed to serialize.");
  RETURN_ERROR_IF(!serialize(&pValue->logs, pStream), "Failed to serialize.");

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
  RETURN_ERROR_IF(!jsonify(&pValue->operationIndexCount, pWriter), "Failed to jsonify.");

  pWriter->write_name("parentState");
  RETURN_ERROR_IF(!jsonify(&pValue->parentState, pWriter), "Failed to jsonify.");

  pWriter->write_name("nextState");
  RETURN_ERROR_IF(!jsonify(&pValue->nextState, pWriter), "Failed to jsonify.");

  pWriter->write_name("lastOperation");
  RETURN_ERROR_IF(!jsonify(&pValue->lastOperation, pWriter), "Failed to jsonify.");

  pWriter->write_name("nextOperation");
  RETURN_ERROR_IF(!jsonify(&pValue->nextOperation, pWriter), "Failed to jsonify.");

  pWriter->write_name("errors");
  RETURN_ERROR_IF(!jsonify(&pValue->errors, pWriter), "Failed to jsonify.");

  pWriter->write_name("warnings");
  RETURN_ERROR_IF(!jsonify(&pValue->warnings, pWriter), "Failed to jsonify.");

  pWriter->write_name("logs");
  RETURN_ERROR_IF(!jsonify(&pValue->logs, pWriter), "Failed to jsonify.");

  pWriter->end();

  return true;
}

inline bool deserialize(OUT lt_sub_system_data *pData, IN ByteStream *pStream, const uint32_t version)
{
  RETURN_ERROR_IF(!deserialize(&pData->states, pStream, version), "Failed to deserialize.");
  RETURN_ERROR_IF(!deserialize(&pData->operations, pStream, version), "Failed to deserialize.");
  RETURN_ERROR_IF(!deserialize(&pData->profilerData, pStream, version), "Failed to deserialize.");

  if (version >= 6)
  {
    RETURN_ERROR_IF(!deserialize(&pData->noStateErrors, pStream, version), "Failed to deserialize.");
    RETURN_ERROR_IF(!deserialize(&pData->noStateWarnings, pStream, version), "Failed to deserialize.");
  }

  if (version >= 7)
    RETURN_ERROR_IF(!deserialize(&pData->noStateLogs, pStream, version), "Failed to deserialize.");

  return true;
}

inline bool serialize(IN const lt_sub_system_data *pData, IN StreamWriter *pStream)
{
  RETURN_ERROR_IF(!serialize(&pData->states, pStream), "Failed to serialize.");
  RETURN_ERROR_IF(!serialize(&pData->operations, pStream), "Failed to serialize.");
  RETURN_ERROR_IF(!serialize(&pData->profilerData, pStream), "Failed to serialize.");
  RETURN_ERROR_IF(!serialize(&pData->noStateErrors, pStream), "Failed to serialize.");
  RETURN_ERROR_IF(!serialize(&pData->noStateWarnings, pStream), "Failed to serialize.");
  RETURN_ERROR_IF(!serialize(&pData->noStateLogs, pStream), "Failed to serialize.");

  return true;
}

inline bool jsonify(IN const lt_sub_system_data *pValue, IN JsonWriter *pWriter)
{
  pWriter->begin_body();

  pWriter->write_name("states");
  RETURN_ERROR_IF(!jsonify(&pValue->states, pWriter), "Failed to jsonify.");

  pWriter->write_name("operations");
  RETURN_ERROR_IF(!jsonify(&pValue->operations, pWriter), "Failed to jsonify.");

  pWriter->write_name("profileData");
  RETURN_ERROR_IF(!jsonify(&pValue->profilerData, pWriter), "Failed to jsonify.");

  pWriter->write_name("noStateErrors");
  RETURN_ERROR_IF(!jsonify(&pValue->noStateErrors.value, pWriter), "Failed to jsonify.");

  pWriter->write_name("noStateWarnings");
  RETURN_ERROR_IF(!jsonify(&pValue->noStateWarnings.value, pWriter), "Failed to jsonify.");

  pWriter->write_name("noStateLogs");
  RETURN_ERROR_IF(!jsonify(&pValue->noStateLogs, pWriter), "Failed to jsonify.");

  pWriter->end();

  return true;
}

inline bool deserialize(OUT lt_stack_trace *pData, IN ByteStream *pStream, const uint32_t version)
{
  RETURN_ERROR_IF(version < 6, "Invalid Version");

  READ(pStream, pData->stackTraceType);
  READ(pStream, pData->offset);

  switch (pData->stackTraceType)
  {
  case lt_stt_internal_offset:
  {
    break;
  }

  case lt_stt_external_module:
  {
    READ_STRING(pStream, pData->info.externalModule.moduleName);
    
    break;
  }

  case lt_stt_function_name:
  {
    READ_STRING(pStream, pData->info.functionName.functionName);
    READ_STRING(pStream, pData->info.functionName.file);
    READ(pStream, pData->info.functionName.line);

    break;
  }

  default:
    RETURN_ERROR("Invalid stack trace type.");
  }

  READ(pStream, pData->hasDisasm);

  if (pData->hasDisasm)
    READ_STRING(pStream, pData->disasm);

  return true;
}

inline bool serialize(IN const lt_stack_trace *pData, IN StreamWriter *pStream)
{
  WRITE(pStream, pData->stackTraceType);
  WRITE(pStream, pData->offset);

  switch (pData->stackTraceType)
  {
  case lt_stt_internal_offset:
  {
    break;
  }

  case lt_stt_external_module:
  {
    WRITE_STRING(pStream, pData->info.externalModule.moduleName);

    break;
  }

  case lt_stt_function_name:
  {
    WRITE_STRING(pStream, pData->info.functionName.functionName);
    WRITE_STRING(pStream, pData->info.functionName.file);
    WRITE(pStream, pData->info.functionName.line);

    break;
  }

  default:
    RETURN_ERROR("Invalid stack trace type.");
  }

  WRITE(pStream, pData->hasDisasm);

  if (pData->hasDisasm)
    WRITE_STRING(pStream, pData->disasm);

  return true;
}

inline bool jsonify(IN const lt_stack_trace *pData, IN JsonWriter *pWriter)
{
  pWriter->begin_body();

  pWriter->write("offset", pData->offset);

  switch (pData->stackTraceType)
  {
  case lt_stt_internal_offset:
  {
    break;
  }

  case lt_stt_external_module:
  {
    pWriter->write("module", pData->info.externalModule.moduleName);

    break;
  }

  case lt_stt_function_name:
  {
    if (pData->info.functionName.functionName[0] != '\0')
      pWriter->write("function", pData->info.functionName.functionName);
    
    if (pData->info.functionName.file[0] != '\0')
      pWriter->write("file", pData->info.functionName.file);
    
    if (pData->info.functionName.line != 0)
      pWriter->write("line", pData->info.functionName.line);

    break;
  }

  default:
    RETURN_ERROR("Invalid stack trace type.");
  }

  if (pData->hasDisasm)
    pWriter->write("disassembly", pData->disasm);

  pWriter->end();

  return true;
}

inline bool deserialize(OUT lt_error *pData, IN ByteStream *pStream, const uint32_t version)
{
  RETURN_ERROR_IF(version < 6, "Invalid Version");

  READ(pStream, pData->errorCode);

  READ(pStream, pData->hasDescription);
  
  if (pData->hasDescription)
    READ_STRING(pStream, pData->description);

  READ(pStream, pData->hasStackTrace);

  if (pData->hasStackTrace)
  {
    READ(pStream, pData->stackTraceHash);

    RETURN_ERROR_IF(!deserialize(&pData->stackTrace, pStream, version), "Failed to deserialize.");
  }
  
  return true;
}

inline bool serialize(IN const lt_error *pData, IN StreamWriter *pStream)
{
  WRITE(pStream, pData->errorCode);

  WRITE(pStream, pData->hasDescription);

  if (pData->hasDescription)
    WRITE_STRING(pStream, pData->description);

  WRITE(pStream, pData->hasStackTrace);

  if (pData->hasStackTrace)
  {
    WRITE(pStream, pData->stackTraceHash);

    RETURN_ERROR_IF(!serialize(&pData->stackTrace, pStream), "Failed to serialize.");
  }

  return true;
}

inline bool jsonify_internal(IN const lt_error *pData, IN JsonWriter *pWriter)
{
  pWriter->write("errorCode", pData->errorCode);

  if (pData->hasDescription)
    pWriter->write("description", pData->description);

  if (pData->hasStackTrace && pData->stackTrace.size() > 0)
  {
    pWriter->write_name("stackTrace");

    RETURN_ERROR_IF(!jsonify(&pData->stackTrace, pWriter), "Failed to jsonify.");
  }

  return true;
}

inline bool jsonify(IN const lt_error *pData, IN JsonWriter *pWriter)
{
  pWriter->begin_body();

  RETURN_ERROR_IF(!jsonify_internal(pData, pWriter), "Failed to jsonify.");

  pWriter->end();

  return true;
}

inline bool deserialize(OUT lt_crash *pData, IN ByteStream *pStream, const uint32_t version)
{
  RETURN_ERROR_IF(version < 7, "Invalid Version");

  RETURN_ERROR_IF(!deserialize(static_cast<lt_error *>(pData), pStream, version), "Failed to deserialize.");

  READ_STRING(pStream, pData->firstOccurence);

  return true;
}

inline bool serialize(IN const lt_crash *pData, IN StreamWriter *pStream)
{
  RETURN_ERROR_IF(!serialize(static_cast<const lt_error *>(pData), pStream), "Failed to serialize.");

  WRITE_STRING(pStream, pData->firstOccurence);

  return true;
}

inline bool jsonify(IN const lt_crash *pData, IN JsonWriter *pWriter)
{
  pWriter->begin_body();

  RETURN_ERROR_IF(!jsonify_internal(static_cast<const lt_error *>(pData), pWriter), "Failed to jsonify.");

  pWriter->write("firstOccurence", pData->firstOccurence);

  pWriter->end();

  return true;
}

inline bool deserialize(OUT lt_error_data *pData, IN ByteStream *pStream, const uint32_t version)
{
  RETURN_ERROR_IF(version < 6, "Invalid Version");

  RETURN_ERROR_IF(!deserialize(&pData->error, pStream, version), "Failed to deserialize.");
  RETURN_ERROR_IF(!deserialize(&pData->data, pStream, version), "Failed to deserialize.");

  return true;
}

inline bool serialize(IN const lt_error_data *pData, IN StreamWriter *pStream)
{
  RETURN_ERROR_IF(!serialize(&pData->error, pStream), "Failed to serialize.");
  RETURN_ERROR_IF(!serialize(&pData->data, pStream), "Failed to serialize.");

  return true;
}

inline bool jsonify(IN const lt_error_data *pValue, IN JsonWriter *pWriter)
{
  pWriter->begin_body();

  pWriter->write_name("error");
  RETURN_ERROR_IF(!jsonify(&pValue->error, pWriter), "Failed to jsonify.");

  pWriter->write_name("data");
  RETURN_ERROR_IF(!jsonify(&pValue->data, pWriter), "Failed to jsonify.");

  pWriter->end();

  return true;
}

inline bool deserialize(OUT lt_crash_data *pData, IN ByteStream *pStream, const uint32_t version)
{
  RETURN_ERROR_IF(version < 7, "Invalid Version");

  RETURN_ERROR_IF(!deserialize(&pData->crash, pStream, version), "Failed to deserialize.");
  RETURN_ERROR_IF(!deserialize(&pData->data, pStream, version), "Failed to deserialize.");

  return true;
}

inline bool serialize(IN const lt_crash_data *pData, IN StreamWriter *pStream)
{
  RETURN_ERROR_IF(!serialize(&pData->crash, pStream), "Failed to serialize.");
  RETURN_ERROR_IF(!serialize(&pData->data, pStream), "Failed to serialize.");

  return true;
}

inline bool jsonify(IN const lt_crash_data *pValue, IN JsonWriter *pWriter)
{
  pWriter->begin_body();

  pWriter->write_name("crash");
  RETURN_ERROR_IF(!jsonify(&pValue->crash, pWriter), "Failed to jsonify.");

  pWriter->write_name("data");
  RETURN_ERROR_IF(!jsonify(&pValue->data, pWriter), "Failed to jsonify.");

  pWriter->end();

  return true;
}

inline bool deserialize(OUT lt_error_identifier *pData, IN ByteStream *pStream, const uint32_t version)
{
  RETURN_ERROR_IF(version < 6, "Invalid Version");

  READ(pStream, pData->errorIndex);
  READ(pStream, pData->hasStateIndex);

  if (pData->hasStateIndex)
    RETURN_ERROR_IF(!deserialize(&pData->state, pStream, version), "Failed to deserialize.");

  return true;
}

inline bool serialize(IN const lt_error_identifier *pData, IN StreamWriter *pStream)
{
  WRITE(pStream, pData->errorIndex);
  WRITE(pStream, pData->hasStateIndex);

  if (pData->hasStateIndex)
    RETURN_ERROR_IF(!serialize(&pData->state, pStream), "Failed to serialize.");

  return true;
}

inline bool jsonify(IN const lt_error_identifier *pData, IN JsonWriter *pWriter)
{
  pWriter->begin_body();

  pWriter->write("errorIndex", pData->errorIndex);

  if (pData->hasStateIndex)
  {
    pWriter->write_name("state");

    RETURN_ERROR_IF(!jsonify(&pData->state, pWriter), "Failed to jsonify.");
  }

  pWriter->end();

  return true;
}

template <typename T>
inline bool deserialize(OUT lt_global_values_exact<T> *pData, IN ByteStream *pStream, const uint32_t version)
{
  RETURN_ERROR_IF(version < 4, "Invalid Version");

  READ(pStream, pData->average);
  READ(pStream, pData->minValue);
  READ(pStream, pData->maxValue);
  READ(pStream, pData->count);

  RETURN_ERROR_IF(!deserialize(&pData->single, pStream, version), "Failed to deserialize.");
  RETURN_ERROR_IF(!deserialize(&pData->multiple, pStream, version), "Failed to deserialize.");

  return true;
}

template <typename T>
inline bool serialize(IN const lt_global_values_exact<T> *pData, IN StreamWriter *pStream)
{
  WRITE(pStream, pData->average);
  WRITE(pStream, pData->minValue);
  WRITE(pStream, pData->maxValue);
  WRITE(pStream, pData->count);

  RETURN_ERROR_IF(!serialize(&pData->single, pStream), "Failed to serialize.");
  RETURN_ERROR_IF(!serialize(&pData->multiple, pStream), "Failed to serialize.");

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
  RETURN_ERROR_IF(version < 4, "Invalid Version");

  READ(pStream, pData->average);
  READ(pStream, pData->minValue);
  READ(pStream, pData->maxValue);
  READ(pStream, pData->count);

  RETURN_ERROR_IF(!deserialize(&pData->single, pStream, version), "Failed to deserialize.");
  RETURN_ERROR_IF(!deserialize(&pData->multiple, pStream, version), "Failed to deserialize.");
  RETURN_ERROR_IF(!deserialize(&pData->data, pStream, version), "Failed to deserialize.");

  return true;
}

template <typename T>
inline bool serialize(IN const lt_values_exact<T> *pData, IN StreamWriter *pStream)
{
  WRITE(pStream, pData->average);
  WRITE(pStream, pData->minValue);
  WRITE(pStream, pData->maxValue);
  WRITE(pStream, pData->count);

  RETURN_ERROR_IF(!serialize(&pData->single, pStream), "Failed to serialize.");
  RETURN_ERROR_IF(!serialize(&pData->multiple, pStream), "Failed to serialize.");
  RETURN_ERROR_IF(!serialize(&pData->data, pStream), "Failed to serialize.");

  return true;
}

template <typename T>
inline bool jsonify(IN const lt_values_exact<T> *pValue, IN JsonWriter *pWriter)
{
  pWriter->begin_body();

  pWriter->write_name("data");

  RETURN_ERROR_IF(!jsonify(&pValue->data, pWriter), "Failed to jsonify.");

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
  RETURN_ERROR_IF(version < 4, "Invalid Version");

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
  RETURN_ERROR_IF(version < 5, "Invalid Version");

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
  RETURN_ERROR_IF(version < 5, "Invalid Version");

  READ(pStream, pData->average);
  READ(pStream, pData->minValue);
  READ(pStream, pData->maxValue);
  READ(pStream, pData->count);

  RETURN_ERROR_IF(!deserialize(&pData->values, pStream, version), "Failed to deserialize.");

  return true;
}

template <typename T>
inline bool serialize(IN const lt_global_value_range<T> *pData, IN StreamWriter *pStream)
{
  WRITE(pStream, pData->average);
  WRITE(pStream, pData->minValue);
  WRITE(pStream, pData->maxValue);
  WRITE(pStream, pData->count);

  RETURN_ERROR_IF(!serialize(&pData->values, pStream), "Failed to serialize.");

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

  RETURN_ERROR_IF(!jsonify_internal(pData, pWriter), "Failed to jsonify.");

  pWriter->end();

  return true;
}

template <typename T>
inline bool deserialize(OUT lt_value_range<T> *pData, IN ByteStream *pStream, const uint32_t version)
{
  RETURN_ERROR_IF(version < 5, "Invalid Version");

  RETURN_ERROR_IF(!deserialize(static_cast<lt_global_value_range<T> *>(pData), pStream, version), "Failed to deserialize.");
  RETURN_ERROR_IF(!deserialize(&pData->data, pStream, version), "Failed to deserialize.");

  return true;
}

template <typename T>
inline bool serialize(IN const lt_value_range<T> *pData, IN StreamWriter *pStream)
{
  RETURN_ERROR_IF(!serialize(static_cast<const lt_global_value_range<T> *>(pData), pStream), "Failed to serialize.");
  RETURN_ERROR_IF(!serialize(&pData->data, pStream), "Failed to serialize.");

  return true;
}

template <typename T>
inline bool jsonify(IN const lt_value_range<T> *pData, IN JsonWriter *pWriter)
{
  pWriter->begin_body();

  RETURN_ERROR_IF(!jsonify_internal(static_cast<const lt_global_value_range<T> *>(pData), pWriter), "Failed to jsonify.");

  pWriter->write_name("data");
  RETURN_ERROR_IF(!jsonify(&pData->data, pWriter), "Failed to jsonify.");

  pWriter->end();

  return true;
}

template <typename T>
inline bool deserialize(OUT lt_perf_value_range<T> *pData, IN ByteStream *pStream, const uint32_t version)
{
  RETURN_ERROR_IF(version < 5, "Invalid Version");

  RETURN_ERROR_IF(!deserialize(static_cast<lt_value_range<T> *>(pData), pStream, version), "Failed to deserialize.");
  RETURN_ERROR_IF(!deserialize(&pData->minInfo, pStream, version), "Failed to deserialize.");
  RETURN_ERROR_IF(!deserialize(&pData->maxInfo, pStream, version), "Failed to deserialize.");

  return true;
}

template <typename T>
inline bool serialize(IN const lt_perf_value_range<T> *pData, IN StreamWriter *pStream)
{
  RETURN_ERROR_IF(!serialize(static_cast<const lt_value_range<T> *>(pData), pStream), "Failed to serialize.");
  RETURN_ERROR_IF(!serialize(&pData->minInfo, pStream), "Failed to serialize.");
  RETURN_ERROR_IF(!serialize(&pData->maxInfo, pStream), "Failed to serialize.");

  return true;
}

template <typename T>
inline bool jsonify(IN const lt_perf_value_range<T> *pData, IN JsonWriter *pWriter)
{
  pWriter->begin_body();

  RETURN_ERROR_IF(!jsonify_internal(static_cast<const lt_global_value_range<T> *>(pData), pWriter), "Failed to jsonify.");

  pWriter->write_name("data");
  RETURN_ERROR_IF(!jsonify(&pData->data, pWriter), "Failed to jsonify.");

  pWriter->write_name("minInfo");
  RETURN_ERROR_IF(!jsonify(&pData->minInfo, pWriter), "Failed to jsonify.");

  pWriter->write_name("maxInfo");
  RETURN_ERROR_IF(!jsonify(&pData->maxInfo, pWriter), "Failed to jsonify.");

  pWriter->end();

  return true;
}

inline bool deserialize(OUT lt_hw_info_analyze *pData, IN ByteStream *pStream, const uint32_t version)
{
  RETURN_ERROR_IF(version < 5, "Invalid Version");

  RETURN_ERROR_IF(!deserialize(&pData->cpuName, pStream, version), "Failed to deserialize.");
  RETURN_ERROR_IF(!deserialize(&pData->cpuCores, pStream, version), "Failed to deserialize.");
  RETURN_ERROR_IF(!deserialize(&pData->ramTotalPhysical, pStream, version), "Failed to deserialize.");
  RETURN_ERROR_IF(!deserialize(&pData->ramTotalVirtual, pStream, version), "Failed to deserialize.");
  RETURN_ERROR_IF(!deserialize(&pData->ramAvailablePhysical, pStream, version), "Failed to deserialize.");
  RETURN_ERROR_IF(!deserialize(&pData->ramAvailableVirtual, pStream, version), "Failed to deserialize.");
  RETURN_ERROR_IF(!deserialize(&pData->osName, pStream, version), "Failed to deserialize.");
  RETURN_ERROR_IF(!deserialize(&pData->gpuDedicatedVRam, pStream, version), "Failed to deserialize.");
  RETURN_ERROR_IF(!deserialize(&pData->gpuSharedVRam, pStream, version), "Failed to deserialize.");
  RETURN_ERROR_IF(!deserialize(&pData->gpuTotalVRam, pStream, version), "Failed to deserialize.");
  RETURN_ERROR_IF(!deserialize(&pData->gpuFreeVRam, pStream, version), "Failed to deserialize.");
  RETURN_ERROR_IF(!deserialize(&pData->gpuVendorId, pStream, version), "Failed to deserialize.");
  RETURN_ERROR_IF(!deserialize(&pData->gpuName, pStream, version), "Failed to deserialize.");
  RETURN_ERROR_IF(!deserialize(&pData->langPrimaryName, pStream, version), "Failed to deserialize.");
  RETURN_ERROR_IF(!deserialize(&pData->elevated, pStream, version), "Failed to deserialize.");
  RETURN_ERROR_IF(!deserialize(&pData->monitorCount, pStream, version), "Failed to deserialize.");
  RETURN_ERROR_IF(!deserialize(&pData->monitorSize, pStream, version), "Failed to deserialize.");
  RETURN_ERROR_IF(!deserialize(&pData->totalMonitorSize, pStream, version), "Failed to deserialize.");
  RETURN_ERROR_IF(!deserialize(&pData->monitorDpiAvgXY, pStream, version), "Failed to deserialize.");
  RETURN_ERROR_IF(!deserialize(&pData->storageAvailable, pStream, version), "Failed to deserialize.");
  RETURN_ERROR_IF(!deserialize(&pData->storageTotal, pStream, version), "Failed to deserialize.");
  RETURN_ERROR_IF(!deserialize(&pData->deviceManufacturerName, pStream, version), "Failed to deserialize.");
  RETURN_ERROR_IF(!deserialize(&pData->deviceManufacturerModelName, pStream, version), "Failed to deserialize.");

  return true;
}

inline bool serialize(IN const lt_hw_info_analyze *pData, IN StreamWriter *pStream)
{
  RETURN_ERROR_IF(!serialize(&pData->cpuName, pStream), "Failed to serialize.");
  RETURN_ERROR_IF(!serialize(&pData->cpuCores, pStream), "Failed to serialize.");
  RETURN_ERROR_IF(!serialize(&pData->ramTotalPhysical, pStream), "Failed to serialize.");
  RETURN_ERROR_IF(!serialize(&pData->ramTotalVirtual, pStream), "Failed to serialize.");
  RETURN_ERROR_IF(!serialize(&pData->ramAvailablePhysical, pStream), "Failed to serialize.");
  RETURN_ERROR_IF(!serialize(&pData->ramAvailableVirtual, pStream), "Failed to serialize.");
  RETURN_ERROR_IF(!serialize(&pData->osName, pStream), "Failed to serialize.");
  RETURN_ERROR_IF(!serialize(&pData->gpuDedicatedVRam, pStream), "Failed to serialize.");
  RETURN_ERROR_IF(!serialize(&pData->gpuSharedVRam, pStream), "Failed to serialize.");
  RETURN_ERROR_IF(!serialize(&pData->gpuTotalVRam, pStream), "Failed to serialize.");
  RETURN_ERROR_IF(!serialize(&pData->gpuFreeVRam, pStream), "Failed to serialize.");
  RETURN_ERROR_IF(!serialize(&pData->gpuVendorId, pStream), "Failed to serialize.");
  RETURN_ERROR_IF(!serialize(&pData->gpuName, pStream), "Failed to serialize.");
  RETURN_ERROR_IF(!serialize(&pData->langPrimaryName, pStream), "Failed to serialize.");
  RETURN_ERROR_IF(!serialize(&pData->elevated, pStream), "Failed to serialize.");
  RETURN_ERROR_IF(!serialize(&pData->monitorCount, pStream), "Failed to serialize.");
  RETURN_ERROR_IF(!serialize(&pData->monitorSize, pStream), "Failed to serialize.");
  RETURN_ERROR_IF(!serialize(&pData->totalMonitorSize, pStream), "Failed to serialize.");
  RETURN_ERROR_IF(!serialize(&pData->monitorDpiAvgXY, pStream), "Failed to serialize.");
  RETURN_ERROR_IF(!serialize(&pData->storageAvailable, pStream), "Failed to serialize.");
  RETURN_ERROR_IF(!serialize(&pData->storageTotal, pStream), "Failed to serialize.");
  RETURN_ERROR_IF(!serialize(&pData->deviceManufacturerName, pStream), "Failed to serialize.");
  RETURN_ERROR_IF(!serialize(&pData->deviceManufacturerModelName, pStream), "Failed to serialize.");

  return true;
}

inline bool jsonify(IN const lt_hw_info_analyze *pData, IN JsonWriter *pWriter)
{
  pWriter->begin_body();

  pWriter->write_name("cpu");
  RETURN_ERROR_IF(!jsonify(&pData->cpuName, pWriter), "Failed to jsonify.");

  pWriter->write_name("cpuCores");
  RETURN_ERROR_IF(!jsonify(&pData->cpuCores, pWriter), "Failed to jsonify.");

  pWriter->write_name("totalPhysicalRam");
  RETURN_ERROR_IF(!jsonify(&pData->ramTotalPhysical, pWriter), "Failed to jsonify.");

  pWriter->write_name("totalVirtualRam");
  RETURN_ERROR_IF(!jsonify(&pData->ramTotalVirtual, pWriter), "Failed to jsonify.");

  pWriter->write_name("availablePhysicalRam");
  RETURN_ERROR_IF(!jsonify(&pData->ramAvailablePhysical, pWriter), "Failed to jsonify.");

  pWriter->write_name("availableVirtualRam");
  RETURN_ERROR_IF(!jsonify(&pData->ramAvailableVirtual, pWriter), "Failed to jsonify.");

  pWriter->write_name("os");
  RETURN_ERROR_IF(!jsonify(&pData->osName, pWriter), "Failed to jsonify.");

  pWriter->write_name("gpuDedicatedVRam");
  RETURN_ERROR_IF(!jsonify(&pData->gpuDedicatedVRam, pWriter), "Failed to jsonify.");

  pWriter->write_name("gpuSharedVRam");
  RETURN_ERROR_IF(!jsonify(&pData->gpuSharedVRam, pWriter), "Failed to jsonify.");

  pWriter->write_name("gpuTotalVRam");
  RETURN_ERROR_IF(!jsonify(&pData->gpuTotalVRam, pWriter), "Failed to jsonify.");

  pWriter->write_name("gpuFreeVRam");
  RETURN_ERROR_IF(!jsonify(&pData->gpuFreeVRam, pWriter), "Failed to jsonify.");

  pWriter->write_name("gpuVendorId");
  RETURN_ERROR_IF(!jsonify(&pData->gpuVendorId, pWriter), "Failed to jsonify.");

  pWriter->write_name("gpu");
  RETURN_ERROR_IF(!jsonify(&pData->gpuName, pWriter), "Failed to jsonify.");

  pWriter->write_name("primaryLanguage");
  RETURN_ERROR_IF(!jsonify(&pData->langPrimaryName, pWriter), "Failed to jsonify.");

  pWriter->write_name("isElevated");
  RETURN_ERROR_IF(!jsonify(&pData->elevated, pWriter), "Failed to jsonify.");

  pWriter->write_name("monitorCount");
  RETURN_ERROR_IF(!jsonify(&pData->monitorCount, pWriter), "Failed to jsonify.");

  pWriter->write_name("monitorSize");
  RETURN_ERROR_IF(!jsonify(&pData->monitorSize, pWriter), "Failed to jsonify.");

  pWriter->write_name("totalMonitorSize");
  RETURN_ERROR_IF(!jsonify(&pData->totalMonitorSize, pWriter), "Failed to jsonify.");

  pWriter->write_name("monitorDpi");
  RETURN_ERROR_IF(!jsonify(&pData->monitorDpiAvgXY, pWriter), "Failed to jsonify.");

  pWriter->write_name("availableStorage");
  RETURN_ERROR_IF(!jsonify(&pData->storageAvailable, pWriter), "Failed to jsonify.");

  pWriter->write_name("totalStorage");
  RETURN_ERROR_IF(!jsonify(&pData->storageTotal, pWriter), "Failed to jsonify.");

  pWriter->write_name("deviceManufacturer");
  RETURN_ERROR_IF(!jsonify(&pData->deviceManufacturerName, pWriter), "Failed to jsonify.");

  pWriter->write_name("deviceManufacturerModel");
  RETURN_ERROR_IF(!jsonify(&pData->deviceManufacturerModelName, pWriter), "Failed to jsonify.");

  pWriter->end();

  return true;
}

inline bool deserialize(OUT lt_analyze *pAnalyze, IN ByteStream *pStream)
{
  uint32_t version;
  READ(pStream, version);

  RETURN_ERROR_IF(version > lt_analyze_file_version, "Invalid Version");

  READ(pStream, pAnalyze->majorVersion);
  READ(pStream, pAnalyze->minorVersion);
  READ_STRING(pStream, pAnalyze->productName);

  RETURN_ERROR_IF(!deserialize(&pAnalyze->subSystems, pStream, version), "Failed to deserialize.");

  if (version >= 4)
  {
    RETURN_ERROR_IF(!deserialize(&pAnalyze->observedU64, pStream, version), "Failed to deserialize.");
    RETURN_ERROR_IF(!deserialize(&pAnalyze->observedI64, pStream, version), "Failed to deserialize.");
    RETURN_ERROR_IF(!deserialize(&pAnalyze->observedString, pStream, version), "Failed to deserialize.");
  }

  if (version >= 5)
  {
    RETURN_ERROR_IF(!deserialize(&pAnalyze->hwInfo, pStream, version), "Failed to deserialize.");
    RETURN_ERROR_IF(!deserialize(&pAnalyze->observedRangeU64, pStream, version), "Failed to deserialize.");
    RETURN_ERROR_IF(!deserialize(&pAnalyze->observedRangeI64, pStream, version), "Failed to deserialize.");
    RETURN_ERROR_IF(!deserialize(&pAnalyze->observedRangeF64, pStream, version), "Failed to deserialize.");
    RETURN_ERROR_IF(!deserialize(&pAnalyze->perfMetrics, pStream, version), "Failed to deserialize.");
  }

  if (version >= 7)
  {
    RETURN_ERROR_IF(!deserialize(&pAnalyze->crashes, pStream, version), "Failed to deserialize.");
  }

  if (version >= 9)
  {
    READ(pStream, pAnalyze->firstDayTimestamp);
    RETURN_ERROR_IF(!deserialize(&pAnalyze->days, pStream, version), "Failed to deserialize.");

    RETURN_ERROR_IF(pStream->read(pAnalyze->hourHistogram, ARRAYSIZE(pAnalyze->hourHistogram)), "Failed to read hour histogram.");
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

  RETURN_ERROR_IF(!serialize(&pAnalyze->subSystems, pStream), "Failed to serialize.");
  RETURN_ERROR_IF(!serialize(&pAnalyze->observedU64, pStream), "Failed to serialize.");
  RETURN_ERROR_IF(!serialize(&pAnalyze->observedI64, pStream), "Failed to serialize.");
  RETURN_ERROR_IF(!serialize(&pAnalyze->observedString, pStream), "Failed to serialize.");
  RETURN_ERROR_IF(!serialize(&pAnalyze->hwInfo, pStream), "Failed to serialize.");
  RETURN_ERROR_IF(!serialize(&pAnalyze->observedRangeU64, pStream), "Failed to serialize.");
  RETURN_ERROR_IF(!serialize(&pAnalyze->observedRangeI64, pStream), "Failed to serialize.");
  RETURN_ERROR_IF(!serialize(&pAnalyze->observedRangeF64, pStream), "Failed to serialize.");
  RETURN_ERROR_IF(!serialize(&pAnalyze->perfMetrics, pStream), "Failed to serialize.");
  RETURN_ERROR_IF(!serialize(&pAnalyze->crashes, pStream), "Failed to serialize.");

  WRITE(pStream, pAnalyze->firstDayTimestamp);
  RETURN_ERROR_IF(!serialize(&pAnalyze->days, pStream), "Failed to serialize.");

  RETURN_ERROR_IF(pStream->write(pAnalyze->hourHistogram, ARRAYSIZE(pAnalyze->hourHistogram)), "Failed to write hour histogram.");

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
  RETURN_ERROR_IF(!jsonify(&pAnalyze->subSystems, pWriter), "Failed to jsonify.");

  pWriter->write_name("hwInfo");
  RETURN_ERROR_IF(!jsonify(&pAnalyze->hwInfo, pWriter), "Failed to jsonify.");

  pWriter->write_name("observedU64");
  RETURN_ERROR_IF(!jsonify(&pAnalyze->observedU64, pWriter), "Failed to jsonify.");

  pWriter->write_name("observedI64");
  RETURN_ERROR_IF(!jsonify(&pAnalyze->observedI64, pWriter), "Failed to jsonify.");

  pWriter->write_name("observedString");
  RETURN_ERROR_IF(!jsonify(&pAnalyze->observedString, pWriter), "Failed to jsonify.");

  pWriter->write_name("observedRangeU64");
  RETURN_ERROR_IF(!jsonify(&pAnalyze->observedRangeU64, pWriter), "Failed to jsonify.");

  pWriter->write_name("observedRangeI64");
  RETURN_ERROR_IF(!jsonify(&pAnalyze->observedRangeI64, pWriter), "Failed to jsonify.");

  pWriter->write_name("observedRangeF64");
  RETURN_ERROR_IF(!jsonify(&pAnalyze->observedRangeF64, pWriter), "Failed to jsonify.");

  pWriter->write_name("perfMetrics");
  RETURN_ERROR_IF(!jsonify(&pAnalyze->perfMetrics, pWriter), "Failed to jsonify.");

  pWriter->write_name("crashes");
  RETURN_ERROR_IF(!jsonify(&pAnalyze->crashes.value, pWriter), "Failed to jsonify.");

  pWriter->write("firstDayTimestamp", pAnalyze->firstDayTimestamp);

  pWriter->write_name("days");
  RETURN_ERROR_IF(!jsonify(&pAnalyze->days, pWriter), "Failed to jsonify.");

  pWriter->begin_array("hours");

  for (size_t i = 0; i < ARRAYSIZE(pAnalyze->hourHistogram); i++)
    pWriter->write_value(pAnalyze->hourHistogram[i]);

  pWriter->end();

  pWriter->end();

  return true;
}

#endif // lta_io_h__
