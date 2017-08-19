#ifndef _UUI_PLATFORM_H
#define _UUI_PLATFORM_H

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>

typedef intptr_t intn_t;
typedef uintptr_t uintn_t;

#define AllocatePool malloc
#define AllocateZeroPool(s) calloc(1, (s))
#define FreePool(x) do { assert((x)); free((x)); } while(0)

#define ASSERT assert

#define SetMem(s, n, c) memset((s), (c), (n))
#define CopyMem memcpy

#define TRUE true
#define FALSE false
typedef bool bool_t;

#define OFFSET_OF offsetof

#endif
