#include "liblt.h"

extern const char *g_lt_product_name = "lt_example";
extern const uint64_t g_lt_product_major_version = 0x1020304050607080;
extern const uint64_t g_lt_product_minor_version = 0xFEDCBA9876543210;
extern const bool g_lt_is_debug_build = true;
extern const char *g_lt_remote_host = "localhost";
extern const char *g_lt_folder_path = nullptr;
extern const bool g_lt_crash_stack_trace_include_data = true;
extern const bool g_lt_error_include_stack_trace = true;
extern const bool g_lt_warn_include_stack_trace = false;

int32_t main(const int32_t argc, const char **pArgv)
{
  (void)argc;
  (void)pArgv;

  lt_crash(0, nullptr);
  lt_crash(0xFFEEDDCCBBAA9988, "abcdefghijklmnopqrstuvwxyz");
  lt_crash(12345, "");
  lt_crash(54321, "a very long string. a very long string. a very long string. a very long string. a very long string. a very long string. a very long string. a very long string. a very long string. a very long string. a very long string. a very long string. a very long string. 54321");

  lt_error(0, 1, "desc");
  lt_error(0, 0, nullptr);
  lt_error(0, 0, nullptr);
  lt_error(1, 0, "test");
  lt_error(10, 0, "a very long string. a very long string. a very long string. a very long string. a very long string. a very long string. a very long string. a very long string. a very long string. a very long string. a very long string. a very long string. a very long string. 54321");

  lt_warn(0, 123, "desc");
  lt_warn(0, 0, nullptr);
  lt_warn(0, 0, nullptr);
  lt_warn(1, 123, "test");
  lt_warn(10, 0, "a very long string. a very long string. a very long string. a very long string. a very long string. a very long string. a very long string. a very long string. a very long string. a very long string. a very long string. a very long string. a very long string. 54321");

  lt_log(0, "zero");
  lt_log(0, "zero");
  lt_log(1, "one");
  lt_log(100, "one hundred");

  lt_set_state(0, 10, 0);
  lt_set_state(0, 11, 0);
  lt_set_state(0, 11, 0);
  lt_set_state(1, 0, 0);
  lt_set_state(1, 0, 1);
  lt_set_state(1, 1, 0);
  lt_set_state(1, 3, 1);

  lt_operation(0, 10, 123);
  lt_operation(0, 10, 1234);
  lt_operation(0, 11, 1);
  lt_operation(0, 10, 2);
  lt_operation(1, 1, 0);
  lt_operation(1, 1, 1);
  lt_operation(1, 2, 5);

  double data[] = { 1000, 2000, 3000, 4000, 5000, 6000 };
  lt_perf_data(0, data, ARRAYSIZE(data));

  for (size_t i = 0; i < ARRAYSIZE(data); i++)
    data[i] += (i & 1) * 400.0 - 200.0;

  lt_perf_data(0, data, ARRAYSIZE(data));

  for (size_t i = 0; i < ARRAYSIZE(data); i++)
    data[i] += (i & 1) * -800.0 + 400.0;
  
  lt_perf_data(1, data, ARRAYSIZE(data) - 1);

  lt_observe_value_u64(0, 10);
  lt_observe_value_u64(0, 1);
  lt_observe_value_u64(0, 0);
  lt_observe_value_u64(0, 0);
  lt_observe_value_u64(1, 11);
  lt_observe_value_u64(2, 12);
  lt_observe_value_u64(3, 13);

  lt_observe_value_i64(0, -10);
  lt_observe_value_i64(0, 1);
  lt_observe_value_i64(0, 0);
  lt_observe_value_i64(0, 0);
  lt_observe_value_i64(1, 11);
  lt_observe_value_i64(2, -12);
  lt_observe_value_i64(3, 13);

  lt_observe_value_f64(0, -10.0);
  lt_observe_value_f64(0, 1);
  lt_observe_value_f64(0, 0);
  lt_observe_value_f64(0, 0);
  lt_observe_value_f64(1, 11.1);
  lt_observe_value_f64(2, -12.2);
  lt_observe_value_f64(3, 13.3);

  lt_observe_exact_value_u64(0, 10);
  lt_observe_exact_value_u64(0, 1);
  lt_observe_exact_value_u64(0, 0);
  lt_observe_exact_value_u64(0, 0);
  lt_observe_exact_value_u64(1, 11);
  lt_observe_exact_value_u64(2, 12);
  lt_observe_exact_value_u64(3, 13);
  
  lt_observe_exact_value_i64(0, -10);
  lt_observe_exact_value_i64(0, 1);
  lt_observe_exact_value_i64(0, 0);
  lt_observe_exact_value_i64(0, 0);
  lt_observe_exact_value_i64(1, 11);
  lt_observe_exact_value_i64(2, -12);
  lt_observe_exact_value_i64(3, 13);

  lt_observe_exact_value_string(0, "zero");
  lt_observe_exact_value_string(0, "o");
  lt_observe_exact_value_string(0, "0");
  lt_observe_exact_value_string(0, "zero");
  lt_observe_exact_value_string(1, "one");
  lt_observe_exact_value_string(2, "two");
  lt_observe_exact_value_string(3, "a very long string. a very long string. a very long string. a very long string. a very long string. a very long string. a very long string. a very long string. a very long string. a very long string. a very long string. a very long string. a very long string. 54321");

  return 0;
}
