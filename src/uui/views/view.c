#include <uui_platform.h>

#include <uui/views/view.h>
#include <uui/views/viewgroup.h>

static inline uintn_t get_default_size(uintn_t size, uui_measure_spec_t measure_spec) {
    uintn_t result = size;

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
    view->computed_size =size;
}

static void uui_view_invalidate(uui_view_t *view) {
    view->invalid = 1;
    if (view->parent)
        view->parent->view.invalidate(&view->parent->view);
}

int uui_view_initialize(uui_view_t *view) {
    memset(view, 0, sizeof(*view));
    view->measure = uui_view_measure;
    view->layout = uui_view_layout;
    view->draw = NULL;
    view->invalidate = uui_view_invalidate;

    view->invalid = 1;
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
