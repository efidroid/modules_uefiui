#ifndef _UUI_PLATFORM_H
#define _UUI_PLATFORM_H

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <assert.h>

typedef uintptr_t uintn_t;

#define AllocatePool malloc
#define FreePool(x) do { assert((x)); free((x)); } while(0)

#endif
