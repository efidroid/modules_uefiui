#include <uui_platform.h>

#include <uui/views/rect.h>

static void uui_view_rect_draw(uui_view_t *view, uui_canvas_t *canvas) {
    uui_view_rect_t* rect = containerof(view, uui_view_rect_t, view);

    canvas->color = rect->color;
    canvas->draw_rect(canvas, uui_point(0, 0), canvas->size);
}

int uui_view_rect_initialize(uui_view_rect_t *rect) {
    memset(rect, 0, sizeof(*rect));
    uui_view_initialize(&rect->view);
    rect->view.draw = uui_view_rect_draw;

    return 0;
}

uui_view_rect_t* uui_view_rect_create(void) {
    uui_view_rect_t *o;
    UUI_CREATE_IMPLEMENTATION(o, uui_view_rect_initialize);
    o->allocated = 1;
    return o;
}

void uui_view_rect_free(uui_view_rect_t *rect) {
    if (rect->allocated)
        FreePool(rect);
}
