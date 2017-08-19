#include <uui_platform.h>

#include <uui/views/view.h>
#include <uui/views/viewgroup.h>
#include <lib/strhashmap.h>

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

static int value2length(uui_component_value_t *value, intn_t *plength) {
    if (value->type==UUI_COMPONENT_VALUETYPE_IDENTIFIER) {
        if (!AsciiStrCmp(value->u.str, "MATCH_PARENT"))
            *plength = (intn_t)UUI_MATCH_PARENT;
        else if (!AsciiStrCmp(value->u.str, "WRAP_CONTENT"))
            *plength = (intn_t)UUI_WRAP_CONTENT;
        else
            return -1;
    }
    else if(value->type==UUI_COMPONENT_VALUETYPE_NUMBER) {
        *plength = (intn_t)value->u.i64;
    }
    else return -1;

    return 0;
}

static int uui_view_set_property(uui_view_t *view, const char *name, uui_component_value_t *value) {
    if (!AsciiStrCmp(name, "layout_width")) {
        return value2length(value, &view->layout_size.width);
    }

    else if (!AsciiStrCmp(name, "layout_height")) {
        return value2length(value, &view->layout_size.height);
    }

    else if (!AsciiStrCmp(name, "id")) {
        if (value->type==UUI_COMPONENT_VALUETYPE_STRING) {
            view->id = AsciiStrDup(value->u.str);
        }
        else return -1;
    }

    else return -1;

    return 0;
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
    view->comp_layout_properties = strHashmapCreate(5);
    view->comp_set_property = uui_view_set_property;

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
