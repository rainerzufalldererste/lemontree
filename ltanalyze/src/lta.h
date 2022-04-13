#ifndef lta_h__
#define lta_h__

#include "lt_common.h"

//////////////////////////////////////////////////////////////////////////

#ifndef IN
#define IN
#endif

#ifndef OUT
#define OUT
#endif

#ifndef IN_OUT
#define IN_OUT IN OUT
#endif

#ifndef OPTIONAL
#define OPTIONAL
#endif

#ifdef _DEBUG
#define DBG_BREAK() __debugbreak()
#else
#define DBG_BREAK() do { } while (0)
#endif

#define FATAL(x, ...) do { printf(x "\n", __VA_ARGS__); DBG_BREAK(); ExitProcess((UINT)-1); } while (0)
#define FATAL_IF(conditional, x, ...) do { if (conditional) { FATAL(x, __VA_ARGS__); } } while (0)
#define RECOVERABLE_ERROR(x, ...) FATAL(x, __VA_ARGS__)
#define RECOVERABLE_ERROR_IF(conditional, x, ...) do { if (conditional) { RECOVERABLE_ERROR(x, __VA_ARGS__); } } while (0)
#define RETURN_ERROR(x, ...) do { printf(x "\n", __VA_ARGS__); DBG_BREAK(); return false; } while (0)
#define RETURN_ERROR_IF(conditional, x, ...) do { if (conditional) { RETURN_ERROR(x, __VA_ARGS__); } } while (0)

//////////////////////////////////////////////////////////////////////////

template <typename T, typename U>
inline T lerp(const T _a, const T _b, const U _x)
{
  return (T)(_a + (_b - _a) * (double)_x);
}

template <typename T, typename U>
inline T adjust(const T _old, const U _new, const uint64_t oldCount)
{
  return (T)(_old + (_new - _old) * (1.0 / (oldCount + 1)));
}

template <typename T, typename U>
inline T adjust(const T _a, const U _b, const uint64_t countA, const uint64_t countB)
{
  return (T)(_a + (_b - _a) * (double)(countB) / (double)(countA + countB));
}

inline double to_seconds(uint64_t timestampDiff)
{
  return timestampDiff * 1e-7;
}

template <typename T>
inline T min(const T a, const T b)
{
  return a <= b ? a : b;
}

template <typename T>
inline T max(const T a, const T b)
{
  return a >= b ? a : b;
}

#endif // lta_h__
