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

#endif
