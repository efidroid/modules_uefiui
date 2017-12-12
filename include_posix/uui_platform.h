#ifndef _UUI_PLATFORM_H
#define _UUI_PLATFORM_H

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <stdio.h>
#include <stdio.h>

typedef intptr_t intn_t;
typedef uintptr_t uintn_t;

#define AllocatePool malloc
#define AllocateZeroPool(s) calloc(1, (s))
#define FreePool(x) do { assert((x)); free((x)); } while(0)
#define ReallocatePool(old, new, x) realloc((x), (new))

#define ASSERT assert

#define SetMem(s, n, c) memset((s), (c), (n))
#define CopyMem memcpy
#define CompareMem memcmp

#define TRUE true
#define FALSE false
typedef bool bool_t;

#define OFFSET_OF offsetof

#define AsciiStrCmp strcmp
#define AsciiStrDup strdup
#define AsciiStrLen strlen
#define AsciiStrnCmp strncmp
#define AsciiStrRChr strrchr
#define AsciiStrToLL strtoll

static inline uintn_t AsciiStrCpyS (char *Destination, uintn_t DestMax, const char *Source) {
    return strncpy(Destination, Source, DestMax)?0:1;
}

static inline uintn_t AsciiStrCatS (char *Destination, uintn_t DestMax, const char  *Source) {
    return strncat(Destination, Source, DestMax)?0:1;
}

#define _DEBUG(level, fmt, ...) fprintf(stderr, fmt, ##__VA_ARGS__)
#define DEBUG(x) _DEBUG x
#define FMTS "s"

#define DEBUG_INIT      0x00000001  // Initialization
#define DEBUG_WARN      0x00000002  // Warnings
#define DEBUG_FS        0x00000008  // EFI File system
#define DEBUG_INFO      0x00000040  // Informational debug messages
#define DEBUG_VERBOSE   0x00400000  // Detailed debug messages that may
                                    // significantly impact boot performance
#define DEBUG_ERROR     0x80000000  // Error


#endif
