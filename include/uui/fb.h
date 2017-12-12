#ifndef _UUI_FB_H
#define _UUI_FB_H

#include <uui/common.h>

typedef struct {
    uint8_t blue;
    uint8_t green;
    uint8_t red;
    uint8_t reserved;
} uui_pixel_t;

typedef struct uui_fb uui_fb_t;

typedef void (*uui_fb_flush)(void);

struct uui_fb {
    uui_pixel_t *pixels;
    uui_size_t size;

    uui_point_t dirtyregion_start;
    uui_point_t dirtyregion_end;
};

static inline uui_pixel_t uui_pixel(uint8_t blue, uint8_t green, uint8_t red, uint8_t reserved) {
    return (uui_pixel_t) {blue, green, red, reserved};
}

uui_fb_t *uui_fb_alloc(intn_t width, intn_t height);
void uui_fb_free(uui_fb_t *fb);
void uui_fb_reset_dirty_region(uui_fb_t *fb);

static inline void uui_fb_mark_dirty_startend(uui_fb_t *fb, uui_point_t start, uui_point_t end) {
    if (fb->dirtyregion_start.x>start.x)
        fb->dirtyregion_start.x = start.x;
    if (fb->dirtyregion_start.y>start.y)
        fb->dirtyregion_start.y = start.y;
    if (fb->dirtyregion_end.x<end.x)
        fb->dirtyregion_end.x = end.x;
    if (fb->dirtyregion_end.y<end.y)
        fb->dirtyregion_end.y = end.y;
}

static inline void uui_fb_mark_dirty_startsize(uui_fb_t *fb, uui_point_t start, uui_size_t size) {
    uui_point_t end = {start.x+size.width, start.y+size.height};
    uui_fb_mark_dirty_startend(fb, start, end);
}

static inline void uui_fb_mark_dirty_startend_xy(uui_fb_t *fb, intn_t startx, intn_t starty, intn_t endx, intn_t endy) {
    uui_point_t start = {startx, starty};
    uui_point_t end = {endx, endy};
    uui_fb_mark_dirty_startend(fb, start, end);
}

#endif
