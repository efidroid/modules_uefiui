#ifndef _COMMON_H
#define _COMMON_H

#include <uui_platform.h>

#ifndef MIN
#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#endif

#ifndef MAX
#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#endif

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(a) (sizeof(a) / sizeof(*(a)))
#endif

#ifndef containerof
#define containerof(ptr, type, member) \
    ((type *)((uintn_t)(ptr) - OFFSET_OF(type, member)))
#endif

#ifndef ALIGN_VALUE
#define ALIGN_VALUE(Value, Alignment) ((Value) + (((Alignment) - (Value)) & ((Alignment) - 1)))
#endif

#endif
