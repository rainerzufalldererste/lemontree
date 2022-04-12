#ifndef lta_types_h__
#define lta_types_h__

#include "lta.h"

#include <stdio.h>
#include <inttypes.h>

#include <memory>
#include <vector>
#include <string>

#include <windows.h>

//////////////////////////////////////////////////////////////////////////

void print_string_as_json(FILE *pFile, const char *string);
void print_string_as_json(FILE *pFile, const wchar_t *string);
void print_bytes_as_base64string(FILE *pFile, const uint8_t *pData, const size_t size);

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
  inline void _write(const uint8_t value) { fprintf(pFile, "%" PRIu8 "", value); }
  inline void _write(const double value) { fprintf(pFile, "%e", value); }
  inline void _write(const bool value) { fputs(value ? "true" : "false", pFile); }

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

template <typename T, typename U>
inline void DualPivotQuickSort_Partition(const int64_t low, const int64_t high, int64_t *pRightPivot, int64_t *pLeftPivot, SoaList<T, U> *pQueue)
{
  if (((*pQueue).value[low]) < ((*pQueue).value[high]))
  {
    std::swap((*pQueue).index[low], (*pQueue).index[high]);
    std::swap((*pQueue).value[low], (*pQueue).value[high]);
  }

  int64_t j = low + 1;
  int64_t g = high - 1;
  int64_t k = low + 1;

  U *pP = &(*pQueue).value[low];
  U *pQ = &(*pQueue).value[high];

  while (k <= g)
  {
    if (((*pQueue).value[k]) > (*pP))
    {
      std::swap((*pQueue).index[k], (*pQueue).index[j]);
      std::swap((*pQueue).value[k], (*pQueue).value[j]);
      j++;
    }

    else if (!(((*pQueue).value[k]) > (*pQ)))
    {
      while (((*pQueue).value[g]) < (*pQ) && k < g)
        g--;

      std::swap((*pQueue).index[k], (*pQueue).index[g]);
      std::swap((*pQueue).value[k], (*pQueue).value[g]);
      g--;

      if (((*pQueue).value[k]) > (*pP))
      {
        std::swap((*pQueue).index[k], (*pQueue).index[j]);
        std::swap((*pQueue).value[k], (*pQueue).value[j]);
        j++;
      }
    }

    k++;
  }

  j--;
  g++;

  std::swap((*pQueue).index[low], (*pQueue).index[j]);
  std::swap((*pQueue).value[low], (*pQueue).value[j]);
  std::swap((*pQueue).index[high], (*pQueue).index[g]);
  std::swap((*pQueue).value[high], (*pQueue).value[g]);

  *pLeftPivot = j;
  *pRightPivot = g;
}

template <typename T, typename U>
inline void QuickSort(const int64_t start, const int64_t end, SoaList<T, U> *pQueue)
{
  if (start < end)
  {
    int64_t leftPivot, rightPivot;

    DualPivotQuickSort_Partition(start, end, &rightPivot, &leftPivot, pQueue);

    QuickSort(start, leftPivot - 1, pQueue);
    QuickSort(leftPivot + 1, rightPivot - 1, pQueue);
    QuickSort(rightPivot + 1, end, pQueue);
  }
}

template <typename T, typename U>
inline void sort_by_value(SoaList<T, U> *pQueue)
{
  QuickSort(0, (int64_t)pQueue->size() - 1, pQueue);
}

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
  bool hasStateIndex = false;
  lt_state_identifier state;
  uint64_t errorIndex;
};

struct lt_crash_ref
{
  uint64_t index;
  lt_transition_data data;
};

enum lt_stack_trace_type : uint8_t
{
  lt_stt_external_module,
  lt_stt_internal_offset,
  lt_stt_function_name,
};

struct lt_stack_trace
{
  uint8_t stackTraceType;
  uint64_t offset;

  union
  {
    struct
    {
      char moduleName[0x100];
    } externalModule;

    struct
    {
    } internalOffset;

    struct
    {
      char functionName[0x100];
      char file[0x100];
      uint32_t line;
    } functionName;
  } info;

  bool hasDisasm = false;
  char disasm[0x100];
};

struct lt_error
{
  uint64_t errorCode;

  bool hasDescription = false;
  char description[0x100];

  bool hasStackTrace = false;
  uint32_t stackTraceHash;
  std::vector<lt_stack_trace> stackTrace;
};

struct lt_error_data
{
  lt_error error;
  lt_transition_data data;
};

struct lt_log
{
  uint64_t subSystem;

  size_t descriptionLength = 0;
  char description[0x100];
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
  bool hasMinLastOperation, hasMaxLastOperation;
  lt_operation_identifier minLastOperation, maxLastOperation;
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
  SoaList<uint64_t, lt_error_data> errors; // DO NOT REORDER THESE.
  SoaList<uint64_t, lt_error_data> warnings; // DO NOT REORDER THESE.

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

  SoaList<lt_error_identifier, lt_transition_data> errors;
  SoaList<lt_error_identifier, lt_transition_data> warnings;
  
  //std::vector<lt_log_ref> logs;
  //std::vector<lt_crash_ref> crashes;
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

struct lt_sub_system_data
{
  SoaList<lt_state_identifier, lt_state> states;
  SoaList<lt_operation_identifier, lt_operation> operations;
  std::vector<lt_perf_data> profilerData;
  SoaList<uint64_t, lt_error_data> noStateErrors, noStateWarnings;
};

template <typename T>
struct lt_global_values_exact
{
  SoaList<T, uint64_t> multiple;
  std::vector<T> single;

  uint64_t count = 0;
  T minValue, maxValue;
  double average = 0;
};

template <typename T>
struct lt_values_exact : lt_global_values_exact<T>
{
  lt_transition_data data;
};

struct lt_string_value_entry
{
  char value[256];
};

template <typename T>
struct lt_vec2
{
  T x, y;
};

template <typename T>
struct lt_global_values
{
  SoaList<T, uint64_t> values;
};

template <typename T>
struct lt_values : lt_global_values<T>
{
  lt_transition_data data;
};

template <typename T>
struct lt_min_value
{
  static constexpr T value = (T)1;
};

template <>
struct lt_min_value<double>
{
  static constexpr double value = DBL_EPSILON;
};

template <>
struct lt_min_value<float>
{
  static constexpr float value = FLT_EPSILON;
};

template <typename T>
struct lt_global_value_range
{
  uint64_t count = 0;
  T minValue, maxValue;
  double average = 0;

  SoaList<double, uint64_t> values;
};

template <typename T>
struct lt_value_range : lt_global_value_range<T>
{
  lt_transition_data data;
};

template <typename T>
struct lt_perf_value_range : lt_value_range<T>
{
  lt_short_hw_info minInfo, maxInfo;
};

struct lt_hw_info_analyze
{
  lt_global_values<lt_string_value_entry> cpuName;
  lt_global_values_exact<uint32_t> cpuCores;
  lt_global_value_range<double> ramTotalPhysical, ramTotalVirtual, ramAvailablePhysical, ramAvailableVirtual;
  lt_global_values<lt_string_value_entry> osName;
  lt_global_value_range<double> gpuDedicatedVRam, gpuSharedVRam, gpuTotalVRam, gpuFreeVRam;
  lt_global_values<uint32_t> gpuVendorId;
  lt_global_values<lt_string_value_entry> gpuName;
  lt_global_values<lt_string_value_entry> langPrimaryName;
  lt_global_values<uint8_t> elevated;
  lt_global_values_exact<uint64_t> monitorCount;
  lt_global_values<lt_vec2<uint32_t>> monitorSize;
  lt_global_values<lt_vec2<uint32_t>> totalMonitorSize;
  lt_global_values_exact<uint32_t> monitorDpiAvgXY;
  lt_global_value_range<double> storageAvailable, storageTotal;
  lt_global_values<lt_string_value_entry> deviceManufacturerName;
  lt_global_values<lt_string_value_entry> deviceManufacturerModelName;
};

struct lt_analyze
{
  uint64_t majorVersion = 0;
  uint64_t minorVersion = 0;
  char productName[0x100];

  SoaList<uint64_t, lt_sub_system_data> subSystems;

  lt_hw_info_analyze hwInfo;

  SoaList<uint64_t, lt_values_exact<uint64_t>> observedU64;
  SoaList<uint64_t, lt_values_exact<int64_t>> observedI64;
  SoaList<uint64_t, lt_values<lt_string_value_entry>> observedString;

  SoaList<uint64_t, lt_value_range<uint64_t>> observedRangeU64;
  SoaList<uint64_t, lt_value_range<int64_t>> observedRangeI64;
  SoaList<uint64_t, lt_value_range<double>> observedRangeF64;

  SoaList<uint64_t, lt_perf_value_range<double>> perfMetrics;

  // Errors? Warnings? Crashes?
};


#endif // lta_types_h__
