#ifndef lt_common_h__
#define lt_common_h__

#include <stdint.h>

enum : uint32_t
{
  lt_version = 0x10000001,

  _lt_state_length = 1 + 8 + 8 + 8 + 8,
  _lt_operation_length = 1 + 8 + 8 + 8 + 8,
  _lt_observed_value_length = 1 + 1 + 8 + 8 + 8,
  _lt_observed_exact_value_length = 1 + 1 + 8 + 8 + 8,
};

enum lt_type : uint8_t
{
  lt_t_start,
  // followed by uint32_t versionNumber.

  // variable length.
  lt_t_hardware_info,
  lt_t_crash,
  lt_t_error,
  lt_t_warning,
  lt_t_log,
  lt_t_perf_data,
  lt_t_observed_value_variable_length, // unused.
  lt_t_observed_exact_value_variable_length,

  // fixed length.
  __lt_t_fixed_length = 0xC0,
  lt_t_state = __lt_t_fixed_length,
  lt_t_operation,
  lt_t_observed_value,
  lt_t_observed_exact_value,
};

enum lt_stacktrace : uint8_t
{
  lt_st_start,
  // followed by DWORD hash, uint64_t count.
  
  lt_st_app_offset,
  // followed by uint64_t offset.
  
  lt_st_dll_offset,
  // followed by uint8_t nameBytes, char[nameBytes] name (not null terminated), uint64_t offset.
  
  lt_st_same_dll_offset,
  // followed by uint64_t offset.
  
  lt_st_data16,
  // followed by uint8_t[16] assembly snapshot.
  // apparently 15 bytes is the maximum length of an x86_64 instruction, so we'll just do 16 bytes when including data (See: https://stackoverflow.com/questions/14698350/x86-64-asm-maximum-bytes-for-an-instruction#:~:text=The%20maximum%20length%20of%20an,and%20would%20probably%20not%20execute.)
  
  lt_st_end,
};

enum lt_value_type : uint8_t
{
  lt_vt_reserved,
  lt_vt_u64,
  lt_vt_i64,
  lt_vt_f64,
  lt_vt_string,
};

#endif // !lt_common_h__
