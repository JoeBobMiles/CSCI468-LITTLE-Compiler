#ifndef main_h
#define main_h

#define PRINT_INLINE 1

#ifdef NDEBUG
#   define assert(E)
#   define InvalidCodePath
#else
#   include <assert.h>
#   include <stdlib.h>
#   define InvalidCodePath abort()
#endif

#include <stdint.h>
typedef int8_t  s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef float  r32;
typedef double r64;

/* for convenience */
typedef const char cchar;

#endif
