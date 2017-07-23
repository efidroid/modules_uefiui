#include <uui_platform.h>

#include <uui/views/viewgroup.h>

static void uui_view_viewgroup_draw(uui_view_t *view, uui_canvas_t *canvas) {
    uui_viewgroup_t* viewgroup = containerof(view, uui_viewgroup_t, view);

    uui_canvas_boundary_t boundary = {.offset = uui_point(0, 0), .size = uui_size(0, 0)};
    canvas->boundary_push(canvas, &boundary, 0);

    uui_view_t *childview;
    list_for_every_entry(&viewgroup->children, childview, uui_view_t, node) {
        boundary.offset = childview->computed_position;
        boundary.size = childview->computed_size;
        canvas->boundary_update(canvas);

        uui_rect_t child_bb  = uui_rect(childview->computed_position, childview->computed_size);

        uintn_t redraw = 0;
        if (childview->invalid_flags & UUI_INVALID_DRAW)
            redraw = 1;
        if (uui_rect_intersect(child_bb, view->invalid_region))
            redraw = 1;

        if(childview->draw && redraw) {
#if 1
            int64_t p0x = ((int64_t)view->invalid_region.pos.x) - ((int64_t)childview->computed_position.x);
            int64_t p0y = ((int64_t)view->invalid_region.pos.y) - ((int64_t)childview->computed_position.y);
            //uui_point_t p0 = uui_point_sub(view->invalid_region.pos, childview->computed_position);
            //uui_point_t p1 = uui_point(p0.x+view->invalid_region.size.width, p0.y+view->invalid_region.size.height);
            int64_t p1x = p0x+view->invalid_region.size.width;
            int64_t p1y = p0y+view->invalid_region.size.height;

            childview->invalid_flags &= ~(UUI_INVALID_DRAW);
            childview->invalid_region = uui_rect(uui_point((uintn_t)MAX(0ll, p0x), (uintn_t)MAX(0ll, p0y)),
                uui_size(
                    (uintn_t)(MIN((int64_t)childview->computed_size.width, p1x)-MAX(0ll, p0x)),
                    (uintn_t)(MIN((int64_t)childview->computed_size.height, p1y)-MAX(0ll, p0y)))
                );

            childview->draw(childview, canvas);
            childview->invalid_region = uui_rect(uui_point(0, 0), uui_size(0, 0));
#elif 0
            uui_point_t p0 = uui_point(0, 0);

            if (childview->computed_position.x < view->invalid_region.pos.x)
                p0.x = view->invalid_region.pos.x - childview->computed_position.x;
            if (childview->computed_position.y < view->invalid_region.pos.y)
                p0.y = view->invalid_region.pos.y - childview->computed_position.y;
#endif
        }
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
