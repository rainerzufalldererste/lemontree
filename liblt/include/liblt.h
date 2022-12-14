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

  // The first call into one of the reporting functions will start the telemetry submission thread.
  // This thread will wait for this process to quit before transmitting information about the session to the server specified in `lt_get_remote_host`.
  // General system information (like CPU, GPU, Memory, Storage, Display, OS, Device, Language, Privileged State) will be included automatically.

#ifdef _WIN32
  // Report a crash in another process & thread.
  // Depending on the value of `lt_get_crash_stack_trace_include_data` the stack trace may include data excerpts to disassemble later.
  void lt_crash_ex(const unsigned long processId, const unsigned long threadId, const uint64_t errorCode, IN OPTIONAL const char *description);
#endif

  // Report a crash.
  // Depending on the value of `lt_get_crash_stack_trace_include_data` the stack trace may include data excerpts to disassemble later.
  void lt_crash(const uint64_t errorCode, IN OPTIONAL const char *description); // `description`: 255 chars max (not including \0).
  
  // Report a failed application state of a specific sub system.
  // May include a stack trace if `lt_error_include_stack_trace` returns true.
  // Depending on the value of `lt_get_crash_stack_trace_include_data` the stack trace may include data excerpts to disassemble later.
  void lt_error(const uint64_t subSystem, const uint64_t errorCode, IN OPTIONAL const char *description); // `description`: 255 chars max (not including \0).
  
  // Report a warning of a specific sub system.
  // May include a stack trace if `lt_error_include_stack_trace` returns true.
  // Depending on the value of `lt_get_crash_stack_trace_include_data` the stack trace may include data excerpts to disassemble later.
  void lt_warn(const uint64_t subSystem, const uint64_t errorCode, IN OPTIONAL const char *description); // `description`: 255 chars max (not including \0).

  // Logs a string to the telemetry system.
  void lt_log(const uint64_t subSystem, IN OPTIONAL const char *description); // `description`: 255 chars max (not including \0).

  // Reports a the state transition of a specific sub system.
  void lt_set_state(const uint64_t subSystem, const uint64_t stateIndex, const uint64_t subStateIndex);

  // Report performance data about the a sub system.
  // `pDataMs` is an array of doubles with length `count`.
  // The data represents the amount of milliseconds the sub system has spent in the performance section corresponding the array index.
  // Performance data is usually associated with a specific sub system state/subState.
  void lt_perf_data(const uint64_t subSystem, IN const double *pDataMs, const uint8_t count);

  // Report an operation that a certain sub system has performed.
  // The `operationIndex` can be used to represent alternate states for the same `operationType`.
  void lt_operation(const uint64_t subSystem, const uint64_t operationType, const uint64_t operationIndex);

  // Report a specific performance metric.
  // These metrics aren't identical to the data collected in `lt_perf_data` and are aimed towards tracking the performance of occasional tasks.
  void lt_perf_metric(const uint64_t valueIndex, const double value);

  // Report the value of a specific variable (`valueIndex`).
  // The exact values may not be preserved, but a histogram of values will be presented.
  void lt_observe_value_u64(const uint64_t valueIndex, const uint64_t value);
  void lt_observe_value_i64(const uint64_t valueIndex, const int64_t value);
  void lt_observe_value_f64(const uint64_t valueIndex, const double value);
  void lt_observe_value_f32_2(const uint64_t valueIndex, const float valueX, const float valueY);

  // Report the value of a specific variable (`valueIndex`).
  // The exact values of this variable is preserved. This can be used for values representing abstract concepts and wouldn't make sense on a histogram.
  void lt_observe_exact_value_u64(const uint64_t exactValueIndex, const uint64_t value);
  void lt_observe_exact_value_i64(const uint64_t exactValueIndex, const int64_t value);
  void lt_observe_exact_value_string(const uint64_t exactValueIndex, IN const char *value); // `value`: 255 chars max (not including \0).

#ifdef __cplusplus
};
#endif

#endif // !liblt_h__
