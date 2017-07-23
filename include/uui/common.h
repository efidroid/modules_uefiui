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

static inline uintn_t uui_size_eq(uui_size_t s1, uui_size_t s2) {
    return (s1.width==s2.width && s1.height==s2.height);
}

static inline uui_size_t uui_size_add(uui_size_t s1, uui_size_t s2) {
    return uui_size(s1.width+s2.width, s1.height+s2.height);
}

static inline uui_point_t uui_point(uintn_t x, uintn_t y) {
    return (uui_point_t) {x, y};
}

static inline uintn_t uui_point_eq(uui_point_t p1, uui_point_t p2) {
    return (p1.x==p2.x && p1.y==p2.y);
}

static inline uui_rect_t uui_rect(uui_point_t pos, uui_size_t size) {
    return (uui_rect_t) {pos, size};
}

static inline uui_point_t uui_point_sub(uui_point_t p1, uui_point_t p2) {
    return uui_point(p1.x-p2.x, p1.y-p2.y);
}

static inline uui_point_t uui_point_add(uui_point_t p1, uui_point_t p2) {
    return uui_point(p1.x+p2.x, p1.y+p2.y);
}

static inline uui_rect_t uui_rect_boundingbox(uui_rect_t r1, uui_rect_t r2) {
    uui_rect_t rc;

    if ((r1.pos.x|r1.pos.y|r1.size.width|r1.size.height)==0) {
        return r2;
    }
    else if ((r2.pos.x|r2.pos.y|r2.size.width|r2.size.height)==0) {
        return r1;
    }

    rc.pos.x = MIN(r1.pos.x, r2.pos.x);
    rc.pos.y = MIN(r1.pos.y, r2.pos.y);

    uintn_t x2 = MAX(r1.pos.x+r1.size.width, r2.pos.x+r2.size.width);
    uintn_t y2 = MAX(r1.pos.y+r1.size.height, r2.pos.y+r2.size.height);

    rc.size.width = x2 - rc.pos.x;
    rc.size.height = y2 - rc.pos.y;

    return rc;
}

static inline uintn_t uui_rect_intersect(uui_rect_t r1, uui_rect_t r2) {
    return MAX(r1.pos.x, r2.pos.x) <= MIN(r1.pos.x+r1.size.width, r2.pos.x+r2.size.width)
        && MAX(r1.pos.y, r2.pos.y) <= MIN(r1.pos.y+r1.size.height, r2.pos.y+r2.size.height);
}

#endif
