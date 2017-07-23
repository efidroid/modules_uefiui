#include <uui_platform.h>

#include <uui/views/view.h>
#include <uui/views/viewgroup.h>

static inline intn_t get_default_size(intn_t size, uui_measure_spec_t measure_spec) {
    intn_t result = size;

    switch (measure_spec.mode) {
    case UUI_UNSPECIFIED:
        result = size;
        break;
    case UUI_AT_MOST:
    case UUI_EXACTLY:
        result = measure_spec.size;
        break;
    }
    return result;
}

static void uui_view_measure(uui_view_t *view, uui_measure_spec_t measure_spec_width, uui_measure_spec_t measure_spec_height) {
    view->measured_size = uui_size(
        get_default_size(0, measure_spec_width),
        get_default_size(0, measure_spec_height)
    );
    view->measured_state[0] = 0;
    view->measured_state[1] = 0;

    uui_resolve_size_and_state(&view->measured_size.width, &view->measured_state[0], measure_spec_width);
    uui_resolve_size_and_state(&view->measured_size.height, &view->measured_state[1], measure_spec_height);
}

static void uui_view_layout(uui_view_t *view, uui_point_t position, uui_size_t size) {
    view->old_computed_position = view->computed_position;
    view->old_computed_size = view->computed_size;

    view->computed_position = position;
    view->computed_size = size;

    if (   !uui_point_eq(view->old_computed_position, view->computed_position)
        || !uui_size_eq(view->old_computed_size, view->computed_size)
    )
    {
        uui_rect_t rect = uui_rect_boundingbox(
                            uui_rect(view->old_computed_position, view->old_computed_size),
                            uui_rect(view->computed_position, view->computed_size)
                          );
        if (view->parent)
            view->parent->view.invalidate(&view->parent->view, UUI_INVALID_DRAW, &rect);
    }
}

static void uui_view_invalidate(uui_view_t *view, uintn_t flags, uui_rect_t *rect) {
    view->invalid_flags |= flags;

    if (flags & UUI_INVALID_DRAW) {
        ASSERT(rect);
        view->invalid_region = uui_rect_boundingbox(view->invalid_region, *rect);

        if (view->parent) {
            uui_rect_t parentrect = uui_rect(uui_point_add(view->computed_position, rect->pos), rect->size);
            view->parent->view.invalidate(&view->parent->view, flags, &parentrect);
        }
    }
    else {
        ASSERT(!rect);
        if (view->parent) {
            view->parent->view.invalidate(&view->parent->view, flags, rect);
        }
    }
}

int uui_view_initialize(uui_view_t *view) {
    SetMem(view, sizeof(*view), 0);
    view->measure = uui_view_measure;
    view->layout = uui_view_layout;
    view->draw = NULL;
    view->invalidate = uui_view_invalidate;

    view->invalid_flags = UUI_INVALID_MEASURE|UUI_INVALID_LAYOUT|UUI_INVALID_DRAW;
    view->invalid_region = uui_rect(uui_point(0, 0), uui_size(UUI_INTN_MAX, UUI_INTN_MAX));
    view->parent = NULL;

    view->layout_size = uui_size(UUI_WRAP_CONTENT, UUI_WRAP_CONTENT);

    return 0;
}

uui_view_t* uui_view_create(void) {
    uui_view_t *o;
    UUI_CREATE_IMPLEMENTATION(o, uui_view_initialize);
    o->allocated = 1;
    return o;
}

void uui_view_free(uui_view_t *view) {
    if (view->allocated)
        FreePool(view);
}
