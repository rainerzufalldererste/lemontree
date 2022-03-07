#include "liblt.h"

extern const char *g_lt_product_name = "lt_example";
extern const uint64_t g_lt_product_major_version = 0x1020304050607080;
extern const uint64_t g_lt_product_minor_version = 0xFEDCBA9876543210;
extern const bool g_lt_is_debug_build = true;
extern const char *g_lt_remote_host = "localhost";
extern const char *g_lt_folder_path = nullptr;
extern const bool g_lt_crash_stack_trace_include_data = true;

int32_t main(const int32_t argc, const char **pArgv)
{
  (void)argc;
  (void)pArgv;

  lt_crash(0, nullptr);
  lt_crash(0xFFEEDDCCBBAA9988, "abcdefghijklmnopqrstuvwxyz");
  lt_crash(12345, "");
  lt_crash(54321, "a very long string. a very long string. a very long string. a very long string. a very long string. a very long string. a very long string. a very long string. a very long string. a very long string. a very long string. a very long string. a very long string. 54321");

  return 0;
}
