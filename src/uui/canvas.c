#include <uui_platform.h>

#include <uui/canvas.h>

static void uui_canvas_draw_rect(uui_canvas_t *canvas, uui_point_t dst, uui_size_t size) {
    uintn_t x, y;

    // 0 means max
    if (size.width==0)
        size.width = canvas->size.width;
    if (size.height==0)
        size.height = canvas->size.height;

    // this allows the canvas to be bigger than the buffer
    uintn_t max_x = MIN(canvas->offset.x + canvas->size.width, canvas->fb->size.width);
    uintn_t max_y = MIN(canvas->offset.y + canvas->size.height, canvas->fb->size.height);

    uui_point_t draw_start = {canvas->offset.x + dst.x, canvas->offset.y + dst.y};
    uui_point_t draw_end = {MIN(draw_start.x + size.width, max_x), MIN(draw_start.y + size.height, max_y)};

    // protect against integer overflow
    if (draw_end.x<draw_start.x)
        return;
    if (draw_end.y<draw_start.y)
        return;

    for (y=draw_start.y; y<draw_end.y; y++) {
        for (x=draw_start.x; x<draw_end.x; x++) {
            canvas->fb->pixels[y*canvas->fb->size.width + x] = canvas->color;
        }
    }

    uui_fb_mark_dirty_startend(canvas->fb, draw_start, draw_end);
}

static inline void uui_canvas_boundary_update_internal(uui_canvas_t *canvas) {
    canvas->offset = uui_point(0, 0);
    canvas->size = canvas->fb->size;

    uui_canvas_boundary_t *boundary;
    list_for_every_entry(&canvas->boundary_stack, boundary, uui_canvas_boundary_t, node) {
        canvas->size.width = MIN(canvas->size.width - boundary->offset.x, boundary->size.width);
        canvas->size.height = MIN(canvas->size.height - boundary->offset.y, boundary->size.height);

        canvas->offset.x += boundary->offset.x;
        canvas->offset.y += boundary->offset.y;
    }
}

static void uui_canvas_boundary_push(uui_canvas_t *canvas, uui_canvas_boundary_t *boundary, uintn_t update) {
    list_add_tail(&canvas->boundary_stack, &boundary->node);
    if (update)
        uui_canvas_boundary_update_internal(canvas);
}

static void uui_canvas_boundary_update(uui_canvas_t *canvas) {
    uui_canvas_boundary_update_internal(canvas);
}

static uui_canvas_boundary_t* uui_canvas_boundary_pop(uui_canvas_t *canvas, uintn_t update) {
    uui_canvas_boundary_t *boundary = list_remove_tail_type(&canvas->boundary_stack, uui_canvas_boundary_t, node);
    if (update)
        uui_canvas_boundary_update_internal(canvas);
    return boundary;
}

void uui_canvas_copy(uui_canvas_t *canvas_dst, uui_canvas_t *canvas_src, uui_point_t dst, uui_point_t src, uui_size_t size) {
    uintn_t y;

    if (size.width==0)
        size.width = canvas_src->size.width;
    if (size.height==0)
        size.height = canvas_src->size.height;

    // this allows the canvas to be bigger than the buffer
    uintn_t dst_max_x = MIN(canvas_dst->offset.x + canvas_dst->size.width, canvas_dst->fb->size.width);
    uintn_t dst_max_y = MIN(canvas_dst->offset.y + canvas_dst->size.height, canvas_dst->fb->size.height);
    uintn_t src_max_x = MIN(canvas_src->offset.x + canvas_src->size.width, canvas_src->fb->size.width);
    uintn_t src_max_y = MIN(canvas_src->offset.y + canvas_src->size.height, canvas_src->fb->size.height);

    uui_point_t dst_draw_start = {canvas_dst->offset.x + dst.x, canvas_dst->offset.y + dst.y};
    uui_point_t dst_draw_end = {MIN(dst_draw_start.x + size.width, dst_max_x), MIN(dst_draw_start.y + size.height, dst_max_y)};
    uui_point_t src_draw_start = {canvas_src->offset.x + src.x, canvas_src->offset.y + src.y};
    uui_point_t src_draw_end = {MIN(src_draw_start.x + size.width, src_max_x), MIN(src_draw_start.y + size.height, src_max_y)};

    // protect against integer overflow
    if (src_draw_end.x<src_draw_start.x)
        return;
    if (src_draw_end.y<src_draw_start.y)
        return;
    if (dst_draw_end.x<dst_draw_start.x)
        return;
    if (dst_draw_end.y<dst_draw_start.y)
        return;

    uintn_t copy_max_y = MIN(src_draw_end.y-src_draw_start.y, dst_draw_end.y-dst_draw_start.y);
    uintn_t copy_max_x = MIN(src_draw_end.x-src_draw_start.x, dst_draw_end.x-dst_draw_start.x);

    for (y=0; y<copy_max_y; y++) {
        uintn_t y_src = src_draw_start.y + y;
        uintn_t y_dst = dst_draw_start.y + y;

        uui_pixel_t *srcbuf = &canvas_src->fb->pixels[y_src*canvas_src->fb->size.width + src_draw_start.x];
        uui_pixel_t *dstbuf = &canvas_dst->fb->pixels[y_dst*canvas_dst->fb->size.width + dst_draw_start.x];
        memcpy(dstbuf, srcbuf, copy_max_x*sizeof(uui_pixel_t));
    }

    uui_point_t dirty_end = uui_point(dst_draw_start.x + copy_max_x, dst_draw_end.y + copy_max_y);
    uui_fb_mark_dirty_startend(canvas_dst->fb, dst_draw_start, dirty_end);
}

int uui_canvas_framebuffer_initialize(uui_canvas_t *canvas, uui_fb_t *fb) {
    memset(canvas, 0, sizeof(*canvas));

    list_initialize(&canvas->boundary_stack);
    canvas->fb = fb;
    canvas->size.width = fb->size.width;
    canvas->size.height = fb->size.height;
    canvas->boundary_push = uui_canvas_boundary_push;
    canvas->boundary_update = uui_canvas_boundary_update;
    canvas->boundary_pop = uui_canvas_boundary_pop;
    canvas->draw_rect = uui_canvas_draw_rect;
    return 0;
}

int uui_canvas_initialize(uui_canvas_t *canvas, uintn_t width, uintn_t height) {
    uui_fb_t *fb = uui_fb_alloc(width, height);
    if (fb==NULL)
        return -1;

    int rc = uui_canvas_framebuffer_initialize(canvas, fb);
    if (rc) {
        uui_fb_free(fb);
        return rc;
    }
    canvas->fb_allocated = 1;

    return 0;
}

uui_canvas_t *uui_canvas_create(uintn_t width, uintn_t height) {
    uui_canvas_t *o;
    UUI_CREATE_IMPLEMENTATION(o, uui_canvas_initialize, width, height);
    o->allocated = 1;
    return o;
}

uui_canvas_t *uui_canvas_framebuffer_create(uui_fb_t *fb) {
    uui_canvas_t *o;
    UUI_CREATE_IMPLEMENTATION(o, uui_canvas_framebuffer_initialize, fb);
    o->allocated = 1;
    return o;
}

void uui_canvas_free(uui_canvas_t *canvas) {
    if (canvas==NULL)
        return;

    if (canvas->fb_allocated)
        uui_fb_free(canvas->fb);

    if (canvas->allocated)
        FreePool(canvas);
}
