#include "lt_common.h"

#include <stdio.h>
#include <inttypes.h>

#include <memory>
#include <vector>
#include <string>

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

#define READ(pWriter, value) do { if (!pWriter->read(&(value))) return false; } while (0)
#define WRITE(pWriter, value) do { if (!pWriter->write(&(value))) return false; } while (0)
#define READ_STRING(pWriter, value) do { uint8_t __len__; if (!pWriter->read(&__len__)) return false; if (__len__ >= sizeof(value)) return false; if (!pWriter->read((value), __len__)) return false; (value)[__len__] = '\0'; } while (0)
#define WRITE_STRING(pWriter, value) do { \
  const uint8_t __len__ = (uint8_t)min(0xFF, strlen(value)); \
  if (!pWriter->write(&__len__)) \
    return false; \
  if (__len__ > 0 && !pWriter->write((value), __len__)) \
    return false; \
  } while (0)

//////////////////////////////////////////////////////////////////////////

void print_string_as_json(FILE *pFile, const char *string);
void print_string_as_json(FILE *pFile, const wchar_t *string);
void print_bytes_as_base64string(FILE *pFile, const uint8_t *pData, const size_t size);

//////////////////////////////////////////////////////////////////////////

template <typename T>
T adjust(const T _old, const T _new, const uint64_t oldCount)
{
  return (T)(_old + (_new - _old) * (1.0 / (oldCount + 1)));
}

double to_seconds(uint64_t timestampDiff)
{
  return timestampDiff * 1e-7;
}

//////////////////////////////////////////////////////////////////////////

class StreamWriter
{
public:
  uint8_t *pBytes = nullptr;
  size_t capacity = 0;
  size_t size = 0;

  StreamWriter() {}

  template <typename T>
  bool write(const T *pData, const size_t count = 1)
  {
    if (pData == nullptr)
      return false;

    const size_t writeSize = sizeof(T) * count;

    if (size + writeSize > capacity)
    {
      const size_t newCapacity = (capacity + writeSize) * 2;
      pBytes = reinterpret_cast<uint8_t *>(realloc(pBytes, newCapacity));

      if (pBytes == nullptr)
        return false;

      capacity = newCapacity;
    }

    memcpy(pBytes + size, pData, writeSize);
    size += writeSize;

    return true;
  }
};

template <typename TIndex, typename TValue>
class SoaList
{
public:
  std::vector<TIndex> index;
  std::vector<TValue> value;

  inline size_t size() const { return index.size(); }
  inline void push_back(const TIndex &idx, const TValue &val) { index.push_back(idx); value.push_back(val); }
  inline void push_back(TIndex &&idx, TValue &&val) { index.push_back((idx)); value.push_back((val)); }
  inline void emplace_back(const TIndex &idx, const TValue &val) { index.emplace_back(idx); value.emplace_back(val); }
  inline void emplace_back(TIndex &&idx, TValue &&val) { index.emplace_back((idx)); value.emplace_back((val)); }
};

class JsonWriter
{
  enum JsonContainerType : uint8_t
  {
    JCT_Body = 0b00,
    JCT_BodyWithContents = 0b01,
    JCT_Array = 0b10,
    JCT_ArrayWithContents = 0b11,
  };

  std::vector<uint8_t> stack;
  FILE *pFile = nullptr;
  bool lastWasNameOnly = false;

  inline void space() { for (const auto &_ : stack) { (void)_; fputs("  ", pFile); } }
  inline void line() { if (lastWasNameOnly) { lastWasNameOnly = false; return; } if ((stack.back() & 1) != 0) fputs(",\n", pFile); else { fputs("\n", pFile); stack.back()++; } space(); }

  inline void _write(const char *value) { print_string_as_json(pFile, value); }
  inline void _write(const wchar_t *value) { print_string_as_json(pFile, value); }
  inline void _write(const uint64_t value) { fprintf(pFile, "\"0x%" PRIX64 "\"", value); }
  inline void _write(const int64_t value) { fprintf(pFile, "\"%" PRIi64 "\"", value); }
  inline void _write(const uint32_t value) { fprintf(pFile, "%" PRIu32 "", value); }
  inline void _write(const int32_t value) { fprintf(pFile, "%" PRIi32 "", value); }
  inline void _write(const double value) { fprintf(pFile, "%e", value); }

public:
  inline JsonWriter(FILE *pFile) : pFile(pFile) { }

  inline void begin_body() { if (stack.size()) line(); fputs("{", pFile); stack.push_back(JCT_Body); }
  inline void begin_object(const char *name) { line(); fprintf(pFile, "\"%s\": {", name); stack.push_back(JCT_Body); }
  inline void begin_array(const char *name) { line(); fprintf(pFile, "\"%s\": [", name); stack.push_back(JCT_Array); }
  inline void begin_array() { line(); fputs("[", pFile); stack.push_back(JCT_Array); }
  inline void end() { FATAL_IF(lastWasNameOnly, "Invalid JsonWriter State."); const uint8_t back = stack.back(); stack.pop_back(); if (back & 1) { fputs("\n", pFile); space(); } if ((back & 0b10) == JCT_Array) fputs("]", pFile); else fputs("}", pFile); }
  
  inline void write_array_base64(const uint8_t *pData, const size_t length) { line(); print_bytes_as_base64string(pFile, pData, length); }

  template <typename T>
  inline void write_value(T value) { line(); _write(value); }

  template <typename T>
  inline void write(const char *name, const T value) { line(); fprintf(pFile, "\"%s\": ", name); _write(value); }
  
  inline void write_name(const char *name) { line(); fprintf(pFile, "\"%s\": ", name); lastWasNameOnly = true; }
};

//////////////////////////////////////////////////////////////////////////

struct lt_state_identifier
{
  uint64_t stateIndex, subStateIndex;
};

struct lt_full_state_identifier : lt_state_identifier
{
  uint64_t subSystem;
};

struct lt_operation_identifier
{
  uint64_t operationType;
};

struct lt_full_operation_identifier : lt_operation_identifier
{
  uint64_t subSystem;
};

struct lt_transition_data
{
  uint64_t count = 0;
  double avgDelayS = 0;
  uint64_t maxDelay = 0, minDelay = 0;
};

struct lt_operation_transition_data : lt_transition_data
{
  SoaList<uint64_t, uint64_t> operationIndexCount;
};

struct lt_error_identifier
{
  uint64_t subSystem;
  uint64_t errorIndex;
};

//struct lt_operation_index_data
//{
//  uint64_t index, count;
//};

//struct lt_state_ref
//{
//  lt_state_identifier index;
//  lt_transition_data data;
//};

//struct lt_explicit_operation_ref
//{
//  lt_operation_identifier index;
//  lt_operation_transition_data data;
//};

//struct lt_operation_ref
//{
//  lt_operation_identifier index;
//  lt_transition_data data;
//};

struct lt_error_ref
{
  lt_error_identifier index;
  lt_transition_data data;
};

typedef lt_error_ref lt_warning_ref, lt_log_ref;

struct lt_crash_ref
{
  uint64_t index;
  lt_transition_data data;
};

typedef lt_crash_ref lt_value_ref;

//template <typename T>
//struct lt_reach_probability
//{
//  T index;
//  uint64_t hits;
//};

template <typename T>
struct lt_values
{
  std::vector<std::pair<T, uint64_t>> multiple;
  std::vector<T> single;

  size_t count = 0;
  T min, max, average;
};

struct lt_state
{
  lt_transition_data data;
  double avgTimeSinceStartS = 0;

  SoaList<lt_state_identifier, lt_transition_data> previousState;
  SoaList<lt_state_identifier, lt_transition_data> nextState;
  SoaList<lt_operation_identifier, lt_operation_transition_data> operations;
  SoaList<lt_operation_identifier, lt_transition_data> previousOperation;

  SoaList<lt_state_identifier, lt_transition_data> stateReach;
  SoaList<lt_operation_identifier, lt_transition_data> operationReach;

  //std::vector<lt_error_ref> errors;
  //std::vector<lt_warning_ref> warnings;
  //std::vector<lt_log_ref> logs;
  //std::vector<lt_crash_ref> crashes;
};

struct lt_operation
{
  lt_transition_data data;
  double avgTimeSinceStartS = 0;

  SoaList<uint64_t, uint64_t> operationIndexCount;
  SoaList<lt_state_identifier, lt_transition_data> parentState;
  SoaList<lt_state_identifier, lt_transition_data> nextState;
  SoaList<lt_operation_identifier, lt_transition_data> lastOperation;
  SoaList<lt_operation_identifier, lt_operation_transition_data> nextOperation;
  //std::vector<lt_error_ref> errors;
  //std::vector<lt_warning_ref> warnings;
  //std::vector<lt_log_ref> logs;
  //std::vector<lt_crash_ref> crashes;
  //std::vector<lt_value_ref> observedU64;
  //std::vector<lt_value_ref> observedI64;
  //std::vector<lt_value_ref> observedF64;
  //std::vector<lt_value_ref> observedString;
  //std::vector<lt_value_ref> observedRangeU64;
  //std::vector<lt_value_ref> observedRangeI64;
  //std::vector<lt_value_ref> observedRangeF64;
};

struct lt_error
{
  uint64_t subSystem;
  uint64_t errorCode;

  size_t descriptionLength = 0;
  char *description = nullptr;

  size_t stackTraceLength = 0;
  uint8_t *pStackTrace = nullptr;
};

struct lt_log
{
  uint64_t subSystem;

  size_t descriptionLength = 0;
  char *description = nullptr;
};

//struct lt_state_pack
//{
//  lt_state_identifier index;
//  lt_state state;
//};
//
//struct lt_operation_pack
//{
//  lt_operation_identifier index;
//  lt_operation operation;
//};

struct lt_analyze
{
  uint64_t majorVersion = 0;
  uint64_t minorVersion = 0;
  char productName[0x100];

  SoaList<uint64_t, SoaList<lt_state_identifier, lt_state>> states;
  SoaList<uint64_t, SoaList<lt_operation_identifier, lt_operation>> operations;
  //std::vector<std::pair<uint64_t, lt_values<uint64_t>>> observedU64;
  //std::vector<std::pair<uint64_t, lt_values<int64_t>>> observedI64;
  //std::vector<std::pair<uint64_t, lt_values<double>>> observedF64;
  //std::vector<std::pair<uint64_t, lt_values<char[256]>>> observedString;
  //std::vector<std::pair<uint64_t, lt_values<uint64_t>>> observedRangeU64;
  //std::vector<std::pair<uint64_t, lt_values<int64_t>>> observedRangeI64;
  //std::vector<std::pair<uint64_t, lt_values<double>>> observedRangeF64;

  // Errors? Warnings? Crashes?
};

enum
{
  lt_analyze_file_version = 1,
};

template <typename T>
bool deserialize(OUT std::vector<T> *pVector, IN ByteStream *pStream, const uint32_t version)
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
bool serialize(IN const std::vector<T> *pVector, IN StreamWriter *pStream)
{
  const uint64_t count = pVector->size();
  WRITE(pStream, count);

  for (const auto &_item : *pVector)
    if (!serialize(&_item, pStream))
      return false;

  return true;
}

template <typename T>
bool jsonify(IN const std::vector<T> *pVector, IN JsonWriter *pWriter)
{
  pWriter->begin_array();

  for (const auto &_item : *pVector)
    if (!jsonify(&_item, pWriter))
      return false;

  pWriter->end();

  return true;
}

template <typename T, typename T2>
bool deserialize(OUT SoaList<T, T2> *pList, IN ByteStream *pStream, const uint32_t version)
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
bool serialize(IN const SoaList<T, T2> *pList, IN StreamWriter *pStream)
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
bool jsonify(IN const SoaList<T, T2> *pList, IN JsonWriter *pWriter)
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

bool deserialize(OUT uint64_t *pValue, IN ByteStream *pStream, const uint32_t /* version */)
{
  READ(pStream, *pValue);

  return true;
}

bool serialize(IN const uint64_t *pValue, IN StreamWriter *pStream)
{
  WRITE(pStream, *pValue);

  return true;
}

bool jsonify(IN const uint64_t *pValue, IN JsonWriter *pWriter)
{
  pWriter->write_value(*pValue);

  return true;
}

bool deserialize(OUT lt_state_identifier *pId, IN ByteStream *pStream, const uint32_t /* version */)
{
  READ(pStream, pId->stateIndex);
  READ(pStream, pId->subStateIndex);

  return true;
}

bool serialize(IN const lt_state_identifier *pId, IN StreamWriter *pStream)
{
  WRITE(pStream, pId->stateIndex);
  WRITE(pStream, pId->subStateIndex);

  return true;
}

bool jsonify(IN const lt_state_identifier *pValue, IN JsonWriter *pWriter)
{
  pWriter->begin_body();
  
  pWriter->write("state", pValue->stateIndex);
  pWriter->write("subState", pValue->subStateIndex);
  
  pWriter->end();

  return true;
}

bool deserialize(OUT lt_transition_data *pData, IN ByteStream *pStream, const uint32_t /* version */)
{
  READ(pStream, pData->avgDelayS);
  READ(pStream, pData->count);
  READ(pStream, pData->minDelay);
  READ(pStream, pData->maxDelay);

  return true;
}

bool serialize(IN const lt_transition_data *pData, IN StreamWriter *pStream)
{
  WRITE(pStream, pData->avgDelayS);
  WRITE(pStream, pData->count);
  WRITE(pStream, pData->minDelay);
  WRITE(pStream, pData->maxDelay);

  return true;
}

bool jsonify(IN const lt_transition_data *pValue, IN JsonWriter *pWriter)
{
  pWriter->begin_body();

  pWriter->write("avgDelay", pValue->avgDelayS);
  pWriter->write("minDelay", to_seconds(pValue->minDelay));
  pWriter->write("maxDelay", to_seconds(pValue->maxDelay));
  pWriter->write("count", pValue->count);

  pWriter->end();

  return true;
}

//bool deserialize(OUT lt_state_ref *pRef, IN ByteStream *pWriter, const uint32_t version)
//{
//  if (!deserialize(&pRef->index, pWriter, version))
//    return false;
//
//  if (!deserialize(&pRef->data, pWriter, version))
//    return false;
//
//  return true;
//}
//
//bool serialize(IN const lt_state_ref *pRef, IN StreamWriter *pWriter)
//{
//  if (!serialize(&pRef->index, pWriter))
//    return false;
//
//  if (!serialize(&pRef->data, pWriter))
//    return false;
//
//  return true;
//}

bool deserialize(OUT lt_operation_identifier *pId, IN ByteStream *pStream, const uint32_t /* version */)
{
  READ(pStream, pId->operationType);

  return true;
}

bool serialize(IN const lt_operation_identifier *pId, IN StreamWriter *pStream)
{
  WRITE(pStream, pId->operationType);

  return true;
}

bool jsonify(IN const lt_operation_identifier *pValue, IN JsonWriter *pWriter)
{
  pWriter->write_value(pValue->operationType);

  return true;
}

bool deserialize(OUT lt_operation_transition_data *pData, IN ByteStream *pStream, const uint32_t version)
{
  if (!deserialize(static_cast<lt_transition_data *>(pData), pStream, version))
    return false;

  if (!deserialize(&pData->operationIndexCount, pStream, version))
    return false;

  return true;
}

bool serialize(IN const lt_operation_transition_data *pData, IN StreamWriter *pStream)
{
  if (!serialize(static_cast<const lt_transition_data *>(pData), pStream))
    return false;
  
  if (!serialize(&pData->operationIndexCount, pStream))
    return false;

  return true;
}

bool jsonify(IN const lt_operation_transition_data *pValue, IN JsonWriter *pWriter)
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

//bool deserialize(OUT lt_explicit_operation_ref *pRef, IN ByteStream *pWriter, const uint32_t version)
//{
//  if (!deserialize(&pRef->index, pWriter, version))
//    return false;
//
//  if (!deserialize(&pRef->data, pWriter, version))
//    return false;
//
//  return true;
//}
//
//bool serialize(IN const lt_explicit_operation_ref *pRef, IN StreamWriter *pWriter)
//{
//  if (!serialize(&pRef->index, pWriter))
//    return false;
//
//  if (!serialize(&pRef->data, pWriter))
//    return false;
//
//  return true;
//}
//
//bool deserialize(OUT lt_operation_ref *pRef, IN ByteStream *pWriter, const uint32_t version)
//{
//  if (!deserialize(&pRef->index, pWriter, version))
//    return false;
//
//  if (!deserialize(&pRef->data, pWriter, version))
//    return false;
//
//  return true;
//}
//
//bool serialize(IN const lt_operation_ref *pRef, IN StreamWriter *pWriter)
//{
//  if (!serialize(&pRef->index, pWriter))
//    return false;
//
//  if (!serialize(&pRef->data, pWriter))
//    return false;
//
//  return true;
//}
//
//template <typename T>
//bool deserialize(OUT lt_reach_probability<T> *pProb, IN ByteStream *pWriter, const uint32_t version)
//{
//  if (!deserialize(pProb->index, pWriter, version))
//    return false;
//
//  READ(pWriter, pProb->hits);
//
//  return true;
//}
//
//template <typename T>
//bool serialize(IN const lt_reach_probability<T> *pProb, IN StreamWriter *pWriter)
//{
//  if (!serialize(pProb->index, pWriter))
//    return false;
//
//  WRITE(pWriter, pProb->hits);
//
//  return true;
//}
//
//bool deserialize(OUT lt_operation_index_data *pData, IN ByteStream *pWriter, const uint32_t /* version */)
//{
//  READ(pWriter, pData->index);
//  READ(pWriter, pData->count);
//
//  return true;
//}
//
//bool serialize(IN const lt_operation_index_data *pData, IN StreamWriter *pWriter)
//{
//  WRITE(pWriter, pData->index);
//  WRITE(pWriter, pData->count);
//
//  return true;
//}

bool deserialize(OUT lt_state *pValue, IN ByteStream *pStream, const uint32_t version)
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

  return true;
}

bool serialize(IN const lt_state *pValue, IN StreamWriter *pStream)
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

  return true;
}

bool jsonify(IN const lt_state *pValue, IN JsonWriter *pWriter)
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

  pWriter->end();

  return true;
}

bool deserialize(OUT lt_operation *pValue, IN ByteStream *pStream, const uint32_t version)
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

bool serialize(IN const lt_operation *pValue, IN StreamWriter *pStream)
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

bool jsonify(IN const lt_operation *pValue, IN JsonWriter *pWriter)
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


bool deserialize(OUT lt_analyze *pAnalyze, IN ByteStream *pStream)
{
  uint32_t version;
  READ(pStream, version);

  if (version > lt_analyze_file_version)
    return false;

  READ(pStream, pAnalyze->majorVersion);
  READ(pStream, pAnalyze->minorVersion);
  READ_STRING(pStream, pAnalyze->productName);

  // Read States.
  {
    if (!deserialize(&pAnalyze->states, pStream, version))
      return false;
  }

  // Read Operations.
  {
    if (!deserialize(&pAnalyze->operations, pStream, version))
      return false;
  }

  return true;
}

bool serialize(IN const lt_analyze *pAnalyze, OUT StreamWriter *pStream)
{
  const uint32_t version = lt_analyze_file_version;
  WRITE(pStream, version);

  WRITE(pStream, pAnalyze->majorVersion);
  WRITE(pStream, pAnalyze->minorVersion);
  WRITE_STRING(pStream, pAnalyze->productName);

  // Write States.
  {
    if (!serialize(&pAnalyze->states, pStream))
      return false;
  }

  // Write Operations.
  {
    if (!serialize(&pAnalyze->operations, pStream))
      return false;
  }

  return true;
}

bool jsonify(IN const lt_analyze *pAnalyze, OUT JsonWriter *pWriter)
{
  pWriter->begin_body();

  pWriter->write("outputVersion", 1);

  pWriter->write("productName", pAnalyze->productName);
  pWriter->write("majorVersion", pAnalyze->majorVersion);
  pWriter->write("minorVersion", pAnalyze->minorVersion);
  
  // Write States.
  {
    pWriter->write_name("states");

    if (!jsonify(&pAnalyze->states, pWriter))
      return false;
  }

  // Write Operations.
  {
    pWriter->write_name("operations");

    if (!jsonify(&pAnalyze->operations, pWriter))
      return false;
  }

  pWriter->end();

  return true;
}

//////////////////////////////////////////////////////////////////////////

lt_state *get_state(IN lt_analyze *pAnalyze, const uint64_t subSystem, const uint64_t stateIndex, const uint64_t subStateIndex)
{
  if (pAnalyze == nullptr)
    return nullptr;

  for (size_t i = 0; i < pAnalyze->states.size(); i++)
  {
    if (pAnalyze->states.index[i] == subSystem)
    {
      auto &list = pAnalyze->states.value[i];

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
  }

  // Create New SubSystem.
  {
    pAnalyze->states.push_back(subSystem, SoaList<lt_state_identifier, lt_state>());

    lt_state_identifier index;
    index.stateIndex = stateIndex;
    index.subStateIndex = subStateIndex;

    pAnalyze->states.value.back().push_back(std::move(index), lt_state());

    return &pAnalyze->states.value.back().value.back();
  }
}

lt_state *get_state(IN lt_analyze *pAnalyze, IN lt_full_state_identifier *pIndex)
{
  if (pIndex == nullptr)
    return nullptr;

  return get_state(pAnalyze, pIndex->subSystem, pIndex->stateIndex, pIndex->subStateIndex);
}

lt_operation *get_operation(IN lt_analyze *pAnalyze, const uint64_t subSystem, const uint64_t operationType)
{
  if (pAnalyze == nullptr)
    return nullptr;

  for (size_t i = 0; i < pAnalyze->operations.size(); i++)
  {
    if (pAnalyze->operations.index[i] == subSystem)
    {
      auto &list = pAnalyze->operations.value[i];

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
  }

  // Create New SubSystem.
  {
    pAnalyze->operations.push_back(subSystem, SoaList<lt_operation_identifier, lt_operation>());

    lt_operation_identifier index;
    index.operationType = operationType;

    pAnalyze->operations.value.back().push_back(std::move(index), lt_operation());

    return &pAnalyze->operations.value.back().value.back();
  }
}

lt_operation *get_operation(IN lt_analyze *pAnalyze, IN lt_full_operation_identifier *pIndex)
{
  if (pIndex == nullptr)
    return nullptr;

  return get_operation(pAnalyze, pIndex->subSystem, pIndex->operationType);
}

//lt_transition_data *get_transition_data(std::vector<lt_state_ref> *pList, const lt_state_identifier *pId)
//{
//  for (auto &_item : *pList)
//    if (_item.index.stateIndex == pId->stateIndex && _item.index.subStateIndex == pId->subStateIndex)
//      return &_item.data;
//
//  lt_state_ref ref;
//  ref.index = *pId;
//
//  pList->push_back(std::move(ref));
//
//  return &pList->back().data;
//}

lt_transition_data *get_transition_data(SoaList<lt_state_identifier, lt_transition_data> *pList, const lt_state_identifier *pId)
{
  for (size_t i = 0; i < pList->size(); i++)
    if (pList->index[i].stateIndex == pId->stateIndex && pList->index[i].subStateIndex == pId->subStateIndex)
      return &pList->value[i];

  pList->push_back(*pId, lt_transition_data());

  return &pList->value.back();
}

//lt_transition_data *get_transition_data(std::vector<lt_operation_ref> *pList, const lt_operation_identifier *pId)
//{
//  for (auto &_item : *pList)
//    if (_item.index.operationType == pId->operationType)
//      return &_item.data;
//
//  lt_operation_ref ref;
//  ref.index = *pId;
//
//  pList->push_back(std::move(ref));
//
//  return &pList->back().data;
//}

lt_transition_data *get_transition_data(SoaList<lt_operation_identifier, lt_transition_data> *pList, const lt_operation_identifier *pId)
{
  for (size_t i = 0; i < pList->size(); i++)
    if (pList->index[i].operationType == pId->operationType)
      return &pList->value[i];

  pList->push_back(*pId, lt_transition_data());

  return &pList->value.back();
}

//lt_operation_transition_data *get_operation_transition_data(std::vector<lt_explicit_operation_ref> *pList, const lt_operation_identifier *pId)
//{
//  for (auto &_item : *pList)
//    if (_item.index.operationType == pId->operationType)
//      return &_item.data;
//
//  lt_explicit_operation_ref ref;
//  ref.index = *pId;
//
//  pList->push_back(std::move(ref));
//
//  return &pList->back().data;
//}

lt_operation_transition_data *get_operation_transition_data(SoaList<lt_operation_identifier, lt_operation_transition_data> *pList, const lt_operation_identifier *pId)
{
  for (size_t i = 0; i < pList->size(); i++)
    if (pList->index[i].operationType == pId->operationType)
      return &pList->value[i];

  pList->push_back(*pId, lt_operation_transition_data());

  return &pList->value.back();
}

void update_transition_data(lt_transition_data *pTransition, const uint64_t delay)
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

void update_operation_transition_data(lt_operation_transition_data *pTransition, const uint64_t delay, const uint64_t operationIndex)
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

void update_operation_index_counts(SoaList<uint64_t, uint64_t> *pIndices, const uint64_t operationIndex)
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

bool analyze_file(const wchar_t *inputFileName, lt_analyze *pAnalyze, bool isNewFile)
{
  lt_analyze_state state;

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

      case lt_t_observed_value:
      {
        //uint8_t dataType = 0;
        //FATAL_IF(!stream.read(&dataType), "Insufficient data steam.");
        //printf(",\"dataType\":%" PRIu8 "", dataType);
        //
        //SKIP_X64(",", "valueIndex", stream);
        //
        //switch (dataType)
        //{
        //case lt_vt_u64:
        //  SKIP_U64(",", "value", stream);
        //  break;
        //
        //case lt_vt_i64:
        //  SKIP_I64(",", "value", stream);
        //  break;
        //
        //case lt_vt_f64:
        //  SKIP_F64(",", "value", stream);
        //  break;
        //
        //default:
        //  RECOVERABLE_ERROR("Invalid data type.");
        //  break;
        //}
        //
        //SKIP_X64(",", "timestamp", stream);

        break;
      }

      case lt_t_observed_exact_value:
      {
        //uint8_t dataType = 0;
        //FATAL_IF(!stream.read(&dataType), "Insufficient data steam.");
        //printf(",\"dataType\":%" PRIu8 "", dataType);
        //
        //SKIP_X64(",", "valueIndex", stream);
        //
        //switch (dataType)
        //{
        //case lt_vt_u64:
        //  SKIP_U64(",", "value", stream);
        //  break;
        //
        //case lt_vt_i64:
        //  SKIP_I64(",", "value", stream);
        //  break;
        //
        //case lt_vt_f64:
        //  SKIP_F64(",", "value", stream);
        //  break;
        //
        //default:
        //  RECOVERABLE_ERROR("Invalid data type.");
        //  break;
        //}
        //
        //SKIP_X64(",", "timestamp", stream);

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
        //uint16_t size = 0;
        //FATAL_IF(!stream.read(&size), "Insufficient data stream.");
        //
        //SKIP_X64(",", "timestamp", stream);
        //SKIP_X64(",", "subSystem", stream);
        //
        //uint8_t count = 0;
        //FATAL_IF(!stream.read(&count), "Insufficient data stream.");
        //
        //if (count > 0)
        //{
        //  const double *pPerfData = reinterpret_cast<const double *>(stream.pData);
        //  FATAL_IF(!stream.read<double>(nullptr, count), "Insufficient data stream.");
        //
        //  fputs(",\"data\":[", stdout);
        //
        //  for (uint8_t i = 0; i < count; i++)
        //  {
        //    if (i > 0)
        //      fputs(",", stdout);
        //
        //    printf("%e", pPerfData[i]);
        //  }
        //
        //  fputs("]", stdout);
        //}

        break;
      }

      case lt_t_observed_exact_value_variable_length:
      {
        //uint16_t size = 0;
        //FATAL_IF(!stream.read(&size), "Insufficient data stream.");
        //
        //uint8_t dataType = 0;
        //FATAL_IF(!stream.read(&dataType), "Insufficient data stream.");
        //
        //SKIP_X64(",", "exactValueIndex", stream);
        //SKIP_X64(",", "timestamp", stream);
        //
        //RECOVERABLE_ERROR_IF(dataType != lt_vt_string, "Invalid Value Type.");
        //
        //if (dataType == lt_vt_string)
        //  SKIP_STRING(",", "value", stream);

        break;
      }

      case lt_t_system_info:
      {
        //uint16_t size = 0;
        //FATAL_IF(!stream.read(&size), "Insufficient data stream.");
        //
        //while (stream.sizeRemaining > 0)
        //{
        //  uint8_t info = 0;
        //  FATAL_IF(!stream.read(&info), "Insufficient data stream.");
        //
        //  switch (info)
        //  {
        //  case lt_si_cpu:
        //  {
        //    SKIP_STRING(",", "description", stream);
        //    SKIP_U32(",", "processorCount", stream);
        //
        //    break;
        //  }
        //
        //  case lt_si_ram:
        //  {
        //    SKIP_X64(",", "totalPhysical", stream);
        //    SKIP_X64(",", "availablePhysical", stream);
        //    SKIP_X64(",", "totalVirtual", stream);
        //    SKIP_X64(",", "availableVirtual", stream);
        //
        //    break;
        //  }
        //
        //  case lt_si_os:
        //  {
        //    SKIP_STRING(",", "os", stream);
        //
        //    break;
        //  }
        //
        //  case lt_si_gpu:
        //  {
        //    SKIP_U64(",", "dedicatedVRAM", stream);
        //    SKIP_U64(",", "sharedVRAM", stream);
        //    SKIP_U64(",", "totalVRAM", stream);
        //    SKIP_U64(",", "freeVRAM", stream);
        //    SKIP_X64(",", "driverVersion", stream);
        //    SKIP_U32(",", "vendorId", stream);
        //    SKIP_U32(",", "deviceId", stream);
        //    SKIP_U32(",", "revisionId", stream);
        //    SKIP_U32(",", "boardId", stream);
        //    SKIP_STRING(",", "deviceDescription", stream);
        //    SKIP_STRING(",", "driverId", stream);
        //
        //    break;
        //  }
        //
        //  case lt_si_lang:
        //  {
        //    uint8_t length = 0;
        //    FATAL_IF(!stream.read(&length), "Insufficient data stream.");
        //    FATAL_IF(!stream.read(svalue, length), "Insufficient data stream.");
        //    svalue[length] = '\0';
        //
        //    size_t offset = 0;
        //
        //    while (offset < length)
        //    {
        //      print_string_as_json(svalue + offset);
        //
        //      offset += strlen(svalue + offset) + 1;
        //    }
        //
        //    break;
        //  }
        //
        //  case lt_si_elevated:
        //  {
        //    SKIP_BOOL(",", "isElevated", stream);
        //
        //    break;
        //  }
        //
        //  case lt_si_monitor:
        //  {
        //    uint8_t count = 0;
        //    FATAL_IF(!stream.read(&count), "Insufficient data stream.");
        //
        //    for (uint8_t i = 0; i < count; i++)
        //    {
        //      SKIP_I32("", "posX", stream);
        //      SKIP_I32(",", "posY", stream);
        //      SKIP_U32(",", "sizeX", stream);
        //      SKIP_U32(",", "sizeY", stream);
        //      SKIP_U32(",", "dpiX", stream);
        //      SKIP_U32(",", "dpiY", stream);
        //    }
        //
        //    break;
        //  }
        //
        //  case lt_si_storage:
        //  {
        //    SKIP_X64(",", "freeBytesAvailable", stream);
        //    SKIP_X64(",", "totalBytes", stream);
        //
        //    break;
        //  }
        //
        //  case lt_si_device:
        //  {
        //    SKIP_STRING(",", "manufacturer", stream);
        //    SKIP_STRING(",", "model", stream);
        //
        //    break;
        //  }
        //
        //  default:
        //  {
        //    stream.sizeRemaining = 0;
        //    RECOVERABLE_ERROR("Unsupported Info (0x%02" PRIX8 ")", info);
        //    break;
        //  }
        //  }
        //}

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

  free(pData);

  return true;
}

//////////////////////////////////////////////////////////////////////////

int32_t main(void)
{
  const wchar_t *commandLine = GetCommandLineW();

  int32_t argc = 0;
  wchar_t **pArgv = CommandLineToArgvW(commandLine, &argc);
  FATAL_IF(argc != 4, "Invalid Parameter.\nUsage: [-io <LT Analyze File> | -o <New LT Analyze File>] <LT Log File> ...");

  const wchar_t *outputFileName = pArgv[2];
  bool isNewFile = true;

  lt_analyze analyze;

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
    FATAL_IF(!analyze_file(pArgv[i], &analyze, isNewFile), "Failed to analyze file %ws", pArgv[i]);

    isNewFile = false;
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

//////////////////////////////////////////////////////////////////////////

void print_string_as_json(FILE *pFile, const char *string)
{
  if (string == nullptr)
  {
    fputs("null", pFile);
    return;
  }

  fputs("\"", pFile);

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
    case '\\': fputs("\\\\", pFile); break;
    case '\"': fputs("\\\"", pFile); break;
    case '\b': fputs("\\b", pFile); break;
    case '\f': fputs("\\f", pFile); break;
    case '\n': fputs("\\n", pFile); break;
    case '\r': fputs("\\r", pFile); break;
    case '\t': fputs("\\t", pFile); break;
    default:
    {
      if (singleChar[0] > 0 && singleChar[0] <= 0x1F)
        fprintf(pFile, "\\u00%02" PRIX8, singleChar[0]);
      else
        fputs(singleChar, pFile);

      break;
    }
    }
  }

  fputs("\"", pFile);
}

void print_string_as_json(FILE *pFile, const wchar_t *string)
{
  if (string == nullptr)
  {
    fputs("null", pFile);
    return;
  }

  fputs("\"", pFile);

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
    case L'\\': fputs("\\\\", pFile); break;
    case L'\"': fputs("\\\"", pFile); break;
    case L'\b': fputs("\\b", pFile); break;
    case L'\f': fputs("\\f", pFile); break;
    case L'\n': fputs("\\n", pFile); break;
    case L'\r': fputs("\\r", pFile); break;
    case L'\t': fputs("\\t", pFile); break;
    default:
    {
      if (singleChar[0] > 0 && singleChar[0] <= 0x1F)
        fprintf(pFile, "\\u00%02" PRIX8, singleChar[0]);
      else
        fputws(singleChar, pFile);

      break;
    }
    }
  }

  fputs("\"", pFile);
}

void print_bytes_as_base64string(FILE *pFile, const uint8_t *pData, const size_t size)
{
  if (pData == nullptr)
  {
    fputs("null", pFile);
    return;
  }

  fputs("\"", pFile);

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

      fputs(next, pFile);
    }
  }

  // Add Padding.
  {
    const size_t bytesMod3 = size % 3;

    if (bytesMod3 != 0)
      for (size_t i = 0; i < 3 - bytesMod3; i++)
        fputs("=", pFile);
  }

  fputs("\"", pFile);
}
