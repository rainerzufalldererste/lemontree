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

//#define PRINT_X64(prefix, name, stream) do { uint64_t v = 0; FATAL_IF(!stream.read(&v), "Insufficient data stream"); printf(prefix "\"" name "\":\"0x%" PRIX64 "\"", v); } while (0);
//#define PRINT_U64(prefix, name, stream) do { uint64_t v = 0; FATAL_IF(!stream.read(&v), "Insufficient data stream"); printf(prefix "\"" name "\":\"%" PRIu64 "\"", v); } while (0);
//#define PRINT_I64(prefix, name, stream) do { int64_t v = 0; FATAL_IF(!stream.read(&v), "Insufficient data stream"); printf(prefix "\"" name "\":\"%" PRIi64 "\"", v); } while (0);
//#define PRINT_F64(prefix, name, stream) do { double v = 0; FATAL_IF(!stream.read(&v), "Insufficient data stream"); printf(prefix "\"" name "\":%e", v); } while (0);
//#define PRINT_U32(prefix, name, stream) do { uint32_t v = 0; FATAL_IF(!stream.read(&v), "Insufficient data stream"); printf(prefix "\"" name "\":%" PRIu32 "", v); } while (0);
//#define PRINT_I32(prefix, name, stream) do { int32_t v = 0; FATAL_IF(!stream.read(&v), "Insufficient data stream"); printf(prefix "\"" name "\":%" PRIi32 "", v); } while (0);
//#define PRINT_U8(prefix, name, stream) do { uint8_t v = 0; FATAL_IF(!stream.read(&v), "Insufficient data stream"); printf(prefix "\"" name "\":%" PRIu8 "", v); } while (0);
//#define PRINT_BOOL(prefix, name, stream) do { uint8_t v = 0; FATAL_IF(!stream.read(&v), "Insufficient data stream"); v = !!v; printf(prefix "\"" name "\":%" PRIu8 "", v); } while (0);
//
//#define PRINT_STRING(prefix, name, stream) do { \
//  char buffer256[0x100]; \
//  uint8_t length = 0; \
//  FATAL_IF(!stream.read(&length), "Insufficient data stream"); \
//  if (length > 0) \
//  { FATAL_IF(!stream.read(buffer256, length), "Insufficient data stream"); \
//    buffer256[length] = '\0'; \
//    fputs(prefix "\"" name "\":", stdout); \
//    print_string_as_json(buffer256); \
//  } } while (0);

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

template <typename T>
struct lt_values
{
  std::vector<std::pair<T, uint64_t>> multiple;
  std::vector<T> single;

  size_t count = 0;
  T min, max, average;
};

struct lt_short_hw_info
{
  char cpuName[0x100] = "<UNKNOWN>";
  uint32_t cpuCores = 0;
  uint64_t freeRam = 0;
  uint64_t totalRam = 0;
  char gpuName[0x100] = "<UNKNOWN>";
  uint64_t freeVRam = 0;
  uint64_t dedicatedVRam = 0;
  uint64_t totalVRam = 0;
  char osName[0x100] = "<UNKNOWN>";
  uint64_t monitorCount = 0;
  uint32_t monitorTotalWidth = 0, monitorTotalHeight = 0;
};

template <typename T>
struct lt_avg_data
{
  uint64_t count = 0;
  double value = 0;
  T maxValue = 0, minValue = 0;
};

// 1.01^(x^1.7)-1
static const double lt_perf_data_sizes[] = { 0.01, 0.03285697058147385, 0.06652806072336404, 0.110750954546329, 0.1658987828997884, 0.2327704810349522, 0.3125338060030061, 0.4067245919737239, 0.5172768409819379, 0.646575833877717, 0.7975327335576197, 0.9736822038758584, 1.179306592328094, 1.4195920019549, 1.700823462599401, 2.030628623602832, 2.418282116414415, 2.875086176114238, 3.414847505261783, 4.054476015351149, 4.814738383900456, 5.721208834388007, 6.80547186160106, 8.10664768624367, 9.673332224582024, 11.56607089221216, 13.86052174911005, 16.65151117747014, 20.05824827622747, 24.2310475824663, 29.36002049995231, 35.68634326613121, 43.51690606970438, 53.24341121737348, 65.36734142191057 };

struct lt_perf_data
{
  uint64_t hist[ARRAYSIZE(lt_perf_data_sizes) + 1] = {};
  lt_avg_data<double> timeMs;
  lt_short_hw_info minInfo, maxInfo;
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

  std::vector<lt_perf_data> profilerData;
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

struct lt_monitor_info
{
  uint32_t width, height;
  uint32_t dpi; // average of dpiX, dpiY
};

struct lt_hw_info
{
  bool hasCpuInfo = false;
  char cpuName[0x100];
  uint32_t cpuCores;
  bool hasRamInfo = false;
  uint64_t ramTotalPhysical, ramTotalVirtual, ramAvailablePhysical, ramAvailableVirtual;
  bool hasOsInfo = false;
  char osName[0x100];
  bool hasGpuInfo = false;
  uint64_t gpuDedicatedVRam, gpuSharedVRam, gpuTotalVRam, gpuFreeVRam, gpuDriverVersion;
  uint32_t gpuVendorId, gpuDeviceId, gpuRevisionId, gpuBoardId;
  char gpuName[0x100];
  char gpuDriverName[0x100];
  bool hasLangInfo = false;
  char langPrimaryName[0x100];
  bool hasElevatedInfo = false;
  bool elevated;
  bool hasMonitorInfo = false;
  std::vector<lt_monitor_info> monitorSizes;
  uint32_t monitorTotalWidth, monitorTotalHeight;
  bool hasStorageInfo = false;
  uint64_t storageAvailable, storageTotal;
  bool hasDeviceInfo = false;
  char deviceManufacturerName[0x100];
  char deviceModelName[0x100];
};

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
  lt_analyze_file_version = 2,
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

template <typename T>
bool deserialize(OUT lt_avg_data<T> *pData, IN ByteStream *pStream, const uint32_t version)
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
bool serialize(IN const lt_avg_data<T> *pData, IN StreamWriter *pStream)
{
  WRITE(pStream, pData->count);
  WRITE(pStream, pData->value);
  WRITE(pStream, pData->minValue);
  WRITE(pStream, pData->maxValue);

  return true;
}

template <typename T>
bool jsonify(IN const lt_avg_data<T> *pData, IN JsonWriter *pWriter)
{
  pWriter->begin_body();

  pWriter->write("count", pData->count);
  pWriter->write("value", pData->value);
  pWriter->write("min", pData->minValue);
  pWriter->write("max", pData->maxValue);

  pWriter->end();

  return true;
}

bool deserialize(OUT lt_short_hw_info *pData, IN ByteStream *pStream, const uint32_t version)
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

bool serialize(IN const lt_short_hw_info *pData, IN StreamWriter *pStream)
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

bool jsonify(IN const lt_short_hw_info *pData, IN JsonWriter *pWriter)
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

bool deserialize(OUT lt_perf_data *pData, IN ByteStream *pStream, const uint32_t version)
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

  return true;
}

bool serialize(IN const lt_perf_data *pData, IN StreamWriter *pStream)
{
  if (!pStream->write(pData->hist, ARRAYSIZE(pData->hist)))
    return false;

  if (!serialize(&pData->timeMs, pStream))
    return false;

  if (!serialize(&pData->minInfo, pStream))
    return false;

  if (!serialize(&pData->minInfo, pStream))
    return false;

  return true;
}

bool jsonify(IN const lt_perf_data *pData, IN JsonWriter *pWriter)
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

  pWriter->end();

  return true;
}

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

  if (version >= 2)
    if (!deserialize(&pValue->profilerData, pStream, version))
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

  if (!serialize(&pValue->profilerData, pStream))
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

  pWriter->write_name("profileData");

  if (!jsonify(&pValue->profilerData, pWriter))
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

void to_short_hw_info(IN const lt_hw_info *pHwInfo, OUT lt_short_hw_info *pShortHwInfo)
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

lt_transition_data *get_transition_data(SoaList<lt_state_identifier, lt_transition_data> *pList, const lt_state_identifier *pId)
{
  for (size_t i = 0; i < pList->size(); i++)
    if (pList->index[i].stateIndex == pId->stateIndex && pList->index[i].subStateIndex == pId->subStateIndex)
      return &pList->value[i];

  pList->push_back(*pId, lt_transition_data());

  return &pList->value.back();
}

lt_transition_data *get_transition_data(SoaList<lt_operation_identifier, lt_transition_data> *pList, const lt_operation_identifier *pId)
{
  for (size_t i = 0; i < pList->size(); i++)
    if (pList->index[i].operationType == pId->operationType)
      return &pList->value[i];

  pList->push_back(*pId, lt_transition_data());

  return &pList->value.back();
}

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

template <typename T>
void update_avg_value(lt_avg_data<T> *pAvgData, const T value)
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

void add_perf_data(lt_state *pState, const size_t index, const double ms, lt_short_hw_info *pHwInfo)
{
  while (pState->profilerData.size() <= index)
    pState->profilerData.emplace_back(lt_perf_data());

  lt_perf_data *pData = &pState->profilerData[index];

  update_avg_value(&pData->timeMs, ms);

  if (pData->timeMs.maxValue == ms)
    pData->maxInfo = *pHwInfo;

  if (pData->timeMs.minValue == ms)
    pData->minInfo = *pHwInfo;

  size_t i = 0;

  for (; i < ARRAYSIZE(pData->hist) - 1; i++)
    if (ms < lt_perf_data_sizes[i])
      break;

  pData->hist[i]++;
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
        uint16_t size = 0;
        FATAL_IF(!stream.read(&size), "Insufficient data stream.");

        uint64_t subSystem, timestamp;

        FATAL_IF(!stream.read(&timestamp), "Insufficient Data");
        FATAL_IF(!stream.read(&subSystem), "Insufficient Data");

        uint8_t count = 0;
        FATAL_IF(!stream.read(&count), "Insufficient data stream.");

        lt_sub_system *pSubSystem = get_sub_system(&state, subSystem);
        
        if (count > 0 && pSubSystem->hasLastState)
        {
          lt_state *pState = get_state(pAnalyze, subSystem, pSubSystem->lastState.stateIndex, pSubSystem->lastState.subStateIndex);

          const double *pPerfData = reinterpret_cast<const double *>(stream.pData);
          FATAL_IF(!stream.read<double>(nullptr, count), "Insufficient data stream.");

          for (uint8_t i = 0; i < count; i++)
            add_perf_data(pState, i, pPerfData[i], &hwInfoShort);
        }

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
