#ifndef _UUI_CANVAS_H
#define _UUI_CANVAS_H

#include <list.h>
#include <uui/common.h>
#include <uui/fb.h>

typedef struct {
    listnode_t node;

    uui_point_t offset;
    uui_size_t size;
} uui_canvas_boundary_t;

typedef struct uui_canvas uui_canvas_t;

typedef void (*uui_canvas_boundary_push_t)(uui_canvas_t *canvas, uui_canvas_boundary_t* boundary, uintn_t update);
typedef void (*uui_canvas_boundary_update_t)(uui_canvas_t *canvas);
typedef uui_canvas_boundary_t* (*uui_canvas_boundary_pop_t)(uui_canvas_t *canvas, uintn_t update);
typedef void (*uui_canvas_draw_rect_t)(uui_canvas_t *canvas, uui_point_t dst, uui_size_t size);

struct uui_canvas {
    int allocated;

    uui_fb_t *fb;
    int fb_allocated;

    listnode_t boundary_stack;
    uui_point_t offset;
    uui_size_t size;

    uui_pixel_t color;

    uui_canvas_boundary_push_t boundary_push;
    uui_canvas_boundary_update_t boundary_update;
    uui_canvas_boundary_pop_t boundary_pop;

    uui_canvas_draw_rect_t draw_rect;
};

int uui_canvas_initialize(uui_canvas_t *canvas, intn_t width, intn_t height);
uui_canvas_t *uui_canvas_create(intn_t width, intn_t height);

int uui_canvas_framebuffer_initialize(uui_canvas_t *canvas, uui_fb_t *fb);
uui_canvas_t *uui_canvas_framebuffer_create(uui_fb_t *fb);

void uui_canvas_free(uui_canvas_t *canvas);

void uui_canvas_copy(uui_canvas_t *canvas_dst, uui_canvas_t *canvas_src, uui_point_t dst, uui_point_t src, uui_size_t size);

#endif
