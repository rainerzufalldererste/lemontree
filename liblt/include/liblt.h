#ifndef liblt_h__
#define liblt_h__

#include <stdbool.h>
#include <stdint.h>

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
  // Telemetry Logs from debug_builds will not be transmitted to the specified telemetry server.
  bool lt_is_debug_build();

  // Reads from extern `const char * g_lt_remote_host`.
  // Either an IPv4/IPv6 address or a domain name.
  const char * lt_get_remote_host();

  // Reads from extern `const char * g_lt_folder_path`.
  // This will be interpreted using `PathUnExpandEnvStringsA`.
  const char * lt_get_output_folder_path();

  // Reads from extern `const bool g_lt_crash_stack_trace_include_data`.
  const bool lt_get_crash_stack_trace_include_data();

  // Reads from extern `const bool g_lt_error_include_stack_trace`.
  const bool lt_error_include_stack_trace();

  // Reads from extern `const bool g_lt_warn_include_stack_trace`.
  const bool lt_warn_include_stack_trace();

  //////////////////////////////////////////////////////////////////////////

#ifdef _WIN32
  void lt_crash_ex(const unsigned long processId, const unsigned long threadId, const uint64_t errorCode, IN OPTIONAL const char *description);
#endif

  void lt_crash(const uint64_t errorCode, IN OPTIONAL const char *description); // `description`: 255 chars max (not including \0).
  void lt_error(const uint64_t subSystem, const uint64_t errorCode, IN OPTIONAL const char *description); // `description`: 255 chars max (not including \0).
  void lt_warn(const uint64_t subSystem, const uint64_t errorCode, IN OPTIONAL const char *description); // `description`: 255 chars max (not including \0).
  void lt_log(const uint64_t subSystem, IN OPTIONAL const char *description); // `description`: 255 chars max (not including \0).
  void lt_set_state(const uint64_t subSystem, const uint64_t stateIndex, const uint64_t subStateIndex);
  void lt_perf_data(const uint64_t subSystem, IN const double *pDataMs, const uint8_t count);
  void lt_operation(const uint64_t subSystem, const uint64_t operationType, const uint64_t operationIndex);
  void lt_perf_metric(const uint64_t valueIndex, const double value);
  void lt_observe_value_u64(const uint64_t valueIndex, const uint64_t value);
  void lt_observe_value_i64(const uint64_t valueIndex, const int64_t value);
  void lt_observe_value_f64(const uint64_t valueIndex, const double value);
  void lt_observe_exact_value_u64(const uint64_t exactValueIndex, const uint64_t value);
  void lt_observe_exact_value_i64(const uint64_t exactValueIndex, const int64_t value);
  void lt_observe_exact_value_string(const uint64_t exactValueIndex, IN const char *value); // `value`: 255 chars max (not including \0).

#ifdef __cplusplus
};
#endif

#endif // !liblt_h__
