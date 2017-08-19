#include <uui_platform.h>

#include <uui/views/rect.h>

static void uui_view_rect_draw(uui_view_t *view, uui_canvas_t *canvas) {
    uui_view_rect_t* rect = containerof(view, uui_view_rect_t, view);

    canvas->color = rect->color;
    canvas->draw_rect(canvas, view->invalid_region.pos, view->invalid_region.size);
}

static int uui_view_rect_set_property(uui_view_t *view, const char *name, uui_component_value_t *value) {
    uui_view_rect_t* rect = containerof(view, uui_view_rect_t, view);

    if (!AsciiStrCmp(name, "color") && value->type==UUI_COMPONENT_VALUETYPE_NUMBER) {
        rect->color = (uui_pixel_t)value->u.color;
    }

    else if (rect->view_comp_set_property) {
        return rect->view_comp_set_property(&rect->view, name, value);
    }

    else return -1;

    return 0;
}

int uui_view_rect_initialize(uui_view_rect_t *rect) {
    SetMem(rect, sizeof(*rect), 0);
    uui_view_initialize(&rect->view);
    rect->view_comp_set_property = rect->view.comp_set_property;
    rect->view.draw = uui_view_rect_draw;
    rect->view.comp_set_property = uui_view_rect_set_property;

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
