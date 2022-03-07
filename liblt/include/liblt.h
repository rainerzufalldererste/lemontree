#ifndef liblt_h__
#define liblt_h__

#include <stdbool.h>
#include <stdint.h>

#ifdef _WIN32
#include <windows.h>
#endif

#ifndef IN
 #define IN
#endif // !IN

#ifndef OUT
 #define OUT
#endif // !OUT

#ifndef IN_OUT
 #define IN_OUT IN OUT
#endif // !IN_OUT

#ifndef OPTIONAL
 #define OPTIONAL
#endif // !OPTIONAL

#ifdef __cplusplus
extern "C"
{
#endif
  // Reads from extern `const char * g_lt_product_name`.
  const char * lt_get_product_name();

  // Reads from extern `const uint64_t g_lt_product_major_version`.
  uint64_t lt_get_product_major_version();

  // Reads from extern `const uint64_t g_lt_product_minor_version`.
  uint64_t lt_get_product_minor_version();

  // Reads from extern `const bool g_lt_is_debug_build`.
  bool lt_is_debug_build();

  // Reads from extern `const char * g_lt_remote_host`.
  const char * lt_get_remote_url();

  // Reads from extern `const char * g_lt_folder_path`.
  // This will be interpreted using `PathUnExpandEnvStringsA`.
  const char * lt_get_output_folder_path();

  // Reads from extern `const bool g_lt_crash_stack_trace_include_data`.
  const bool lt_get_crash_stack_trace_include_data();

  //////////////////////////////////////////////////////////////////////////

#ifdef _WIN32
  void lt_crash_ex(IN HANDLE process, IN HANDLE thread, const uint64_t errorCode, IN OPTIONAL const char *description);
#endif

  void lt_crash(const uint64_t errorCode, IN OPTIONAL const char *description);
  void lt_error(const uint64_t errorCode, IN OPTIONAL const char *description);
  void lt_warn(const uint64_t errorCode, IN OPTIONAL const char *description);
  void lt_set_state(const uint64_t stateIndex, const uint64_t subStateIndex);
  void lt_perf_data(const uint64_t performanceDataIndex, IN const uint64_t *pDataNs, const size_t count);
  void lt_operation(const uint64_t operationType, const uint64_t operationIndex);
  void lt_observe_value_u64(const uint64_t valueIndex, const uint64_t value);
  void lt_observe_value_i64(const uint64_t valueIndex, const int64_t value);
  void lt_observe_value_f64(const uint64_t valueIndex, const double value);
  void lt_observe_value_string(const uint64_t valueIndex, IN const char *value);
  void lt_observe_exact_value_u64(const uint64_t exactValueIndex, const uint64_t value);
  void lt_observe_exact_value_i64(const uint64_t exactValueIndex, const int64_t value);
  void lt_observe_exact_value_string(const uint64_t exactValueIndex, IN const char *value);

#ifdef __cplusplus
};
#endif

#endif // !liblt_h__
