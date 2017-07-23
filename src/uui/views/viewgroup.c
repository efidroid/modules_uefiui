#include <uui_platform.h>

#include <uui/views/viewgroup.h>

static void uui_viewgroup_measure(uui_view_t *view, uui_measure_spec_t measure_spec_width, uui_measure_spec_t measure_spec_height) {
    view->measured_size = uui_size(0, 0);
    view->measured_state[0] = 0;
    view->measured_state[1] = 0;

    uui_resolve_size_and_state(&view->measured_size.width, &view->measured_state[0], measure_spec_width);
    uui_resolve_size_and_state(&view->measured_size.height, &view->measured_state[1], measure_spec_height);
}

static void uui_viewgroup_layout(uui_view_t *view, uui_point_t position, uui_size_t size) {
    view->old_computed_position = view->computed_position;
    view->old_computed_size = view->computed_size;

    view->computed_position = position;
    view->computed_size = size;
}

static void uui_view_viewgroup_draw(uui_view_t *view, uui_canvas_t *canvas) {
    uui_viewgroup_t* viewgroup = containerof(view, uui_viewgroup_t, view);

    uui_canvas_boundary_t boundary = {.offset = uui_point(0, 0), .size = uui_size(0, 0)};
    canvas->boundary_push(canvas, &boundary, 0);

    uui_view_t *childview;
    list_for_every_entry(&viewgroup->children, childview, uui_view_t, node) {
        //if (!childview->invalid)
        //    continue;

        boundary.offset = childview->computed_position;
        boundary.size = childview->computed_size;
        canvas->boundary_update(canvas);

        childview->invalid = 0;
        if(childview->draw)
            childview->draw(childview, canvas);
    }

    canvas->boundary_pop(canvas, 1);
}

static void uui_viewgroup_internal_add_view(uui_viewgroup_t *viewgroup, uui_view_t *view) {
    list_add_tail(&viewgroup->children, &view->node);
    view->parent = viewgroup;
}

int uui_viewgroup_initialize(uui_viewgroup_t *viewgroup) {
    SetMem(viewgroup, sizeof(*viewgroup), 0);

    uui_view_initialize(&viewgroup->view);
    viewgroup->view.measure = uui_viewgroup_measure;
    viewgroup->view.layout = uui_viewgroup_layout;
    viewgroup->view.draw = uui_view_viewgroup_draw;

    list_initialize(&viewgroup->children);
    viewgroup->internal_add_view = uui_viewgroup_internal_add_view;

    return 0;
}

uui_viewgroup_t* uui_viewgroup_create(void) {
    uui_viewgroup_t *o;
    UUI_CREATE_IMPLEMENTATION(o, uui_viewgroup_initialize);
    o->allocated = 1;
    return o;
}

void uui_viewgroup_free(uui_viewgroup_t *viewgroup) {
    if (viewgroup->allocated)
        FreePool(viewgroup);
}
