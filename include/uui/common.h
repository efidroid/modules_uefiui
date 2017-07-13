#ifndef _UUI_COMMON_H
#define _UUI_COMMON_H

#include <uui_platform.h>
#include <list.h>

#define UUI_CREATE_IMPLEMENTATION(o, fn, ...) do { \
    o = AllocatePool(sizeof(*(o))); \
    if (o == NULL) break; \
    int rc = fn (o, ##__VA_ARGS__); \
    if (rc) { \
        FreePool(o); \
        o = NULL; \
        break; \
    } \
} while(0)

typedef struct {
    uintn_t width;
    uintn_t height;
} uui_size_t;

typedef struct {
    uintn_t x;
    uintn_t y;
} uui_point_t;

typedef struct {
    uui_point_t pos;
    uui_size_t size;
} uui_rect_t;

static inline uui_size_t uui_size(uintn_t width, uintn_t height) {
    return (uui_size_t) {width, height};
}

static inline uintn_t uui_size_lt(uui_size_t s1, uui_size_t s2) {
    return (s1.width<s2.width || s1.height<s2.height);
}

static inline uui_point_t uui_point(uintn_t x, uintn_t y) {
    return (uui_point_t) {x, y};
}

static inline uui_rect_t uui_rect(uui_point_t pos, uui_size_t size) {
    return (uui_rect_t) {pos, size};
}

static inline uui_point_t uui_point_sub(uui_point_t p1, uui_point_t p2) {
    return uui_point(p1.x-p2.x, p1.y-p2.y);
}

#endif
