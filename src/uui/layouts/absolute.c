#include <uui/layouts/absolute.h>

static uui_measure_spec_t get_child_measure_spec(uui_measure_spec_t spec, intn_t padding, intn_t childDimension) {
    intn_t specSize = spec.size;

    intn_t size = specSize - padding;

    intn_t resultSize = 0;
    uint8_t resultMode = 0;

    switch (spec.mode) {
    // Parent has imposed an exact size on us
    case UUI_EXACTLY:
        if (childDimension == UUI_MATCH_PARENT) {
            // Child wants to be our size. So be it.
            resultSize = size;
            resultMode = UUI_EXACTLY;
        } else if (childDimension == UUI_WRAP_CONTENT) {
            // Child wants to determine its own size. It can't be
            // bigger than us.
            resultSize = size;
            resultMode = UUI_AT_MOST;
        }
        else {
            resultSize = childDimension;
            resultMode = UUI_EXACTLY;
        }
        break;

    // Parent has imposed a maximum size on us
    case UUI_AT_MOST:
        if (childDimension == UUI_MATCH_PARENT) {
            // Child wants to be our size, but our size is not fixed.
            // Constrain child to not be bigger than us.
            resultSize = size;
            resultMode = UUI_AT_MOST;
        } else if (childDimension == UUI_WRAP_CONTENT) {
            // Child wants to determine its own size. It can't be
            // bigger than us.
            resultSize = size;
            resultMode = UUI_AT_MOST;
        }
        else {
            // Child wants a specific size... so be it
            resultSize = childDimension;
            resultMode = UUI_EXACTLY;
        }
        break;

    // Parent asked to see how big we want to be
    case UUI_UNSPECIFIED:
        if (childDimension == UUI_MATCH_PARENT) {
            // Child wants to be our size... find out how big it should
            // be
            resultSize = size;
            resultMode = UUI_UNSPECIFIED;
        } else if (childDimension == UUI_WRAP_CONTENT) {
            // Child wants to determine its own size.... find out how
            // big it should be
            resultSize = size;
            resultMode = UUI_UNSPECIFIED;
        }
        else {
            // Child wants a specific size... let him have it
            resultSize = childDimension;
            resultMode = UUI_EXACTLY;
        }
        break;
    }

    return uui_measure_spec(resultSize, resultMode);
}

static void uui_layout_absolute_measure(uui_view_t *view, uui_measure_spec_t measure_spec_width, uui_measure_spec_t measure_spec_height) {
    uui_layout_absolute_t* absolute = containerof(view, uui_layout_absolute_t, viewgroup.view);
    uui_size_t size = uui_size(0, 0);

    uui_view_t *childview;
    uui_layoutparams_absolute_t *lp;
    list_for_every_entry(&absolute->children_params, lp, uui_layoutparams_absolute_t, node) {
        childview = lp->view;

        if (childview->measure && (childview->invalid_flags & UUI_INVALID_MEASURE)) {
            childview->invalid_flags &= ~(UUI_INVALID_MEASURE);
            childview->measure(childview,
                get_child_measure_spec(measure_spec_width, 0, childview->layout_size.width),
                get_child_measure_spec(measure_spec_height, 0, childview->layout_size.height)
            );
        }

        size.width = MAX(size.width, lp->position.x+childview->measured_size.width);
        size.height = MAX(size.height, lp->position.y+childview->measured_size.height);
    }

    view->measured_size = size;
    view->measured_state[0] = 0;
    view->measured_state[1] = 0;
    uui_resolve_size_and_state(&view->measured_size.width, &view->measured_state[0], measure_spec_width);
    uui_resolve_size_and_state(&view->measured_size.height, &view->measured_state[1], measure_spec_height);
}

static void uui_layout_absolute_layout(uui_view_t *view, uui_point_t position, uui_size_t size) {
    uui_layout_absolute_t* absolute = containerof(view, uui_layout_absolute_t, viewgroup.view);

    uui_view_t *childview;
    uui_layoutparams_absolute_t *lp;
    list_for_every_entry(&absolute->children_params, lp, uui_layoutparams_absolute_t, node) {
        childview = lp->view;

        if (childview->layout && (childview->invalid_flags & UUI_INVALID_LAYOUT)) {
            childview->invalid_flags &= ~(UUI_INVALID_LAYOUT);
            childview->layout(childview, lp->position, childview->measured_size);
        }
    }

    absolute->view_layout(view, position, size);
}

static void uui_layout_absolute_add_view(uui_layout_absolute_t *absolute, uui_view_t *view, uui_point_t position) {
    uui_layoutparams_absolute_t *lp = AllocatePool(sizeof(uui_layoutparams_absolute_t));
    if (!lp) return;

    lp->view = view;
    list_add_tail(&absolute->children_params, &lp->node);
    lp->position = position;

    absolute->viewgroup.internal_add_view(&absolute->viewgroup, view);
}

static uui_layoutparams_absolute_t* uui_layout_absolute_get_layoutparams(uui_layout_absolute_t *absolute, uui_view_t *view) {
    uui_layoutparams_absolute_t *lp;
    list_for_every_entry(&absolute->children_params, lp, uui_layoutparams_absolute_t, node) {
        if (lp->view == view)
            return lp;
    }
    return NULL;
}

int uui_layout_absolute_initialize(uui_layout_absolute_t *absolute) {
    SetMem(absolute, sizeof(*absolute), 0);
    uui_viewgroup_initialize(&absolute->viewgroup);
    absolute->view_layout = absolute->viewgroup.view.layout;
    absolute->viewgroup.view.measure = uui_layout_absolute_measure;
    absolute->viewgroup.view.layout = uui_layout_absolute_layout;

    list_initialize(&absolute->children_params);
    absolute->add_view = uui_layout_absolute_add_view;
    absolute->get_layoutparams = uui_layout_absolute_get_layoutparams;

    return 0;
}

uui_layout_absolute_t* uui_layout_absolute_create(void) {
    uui_layout_absolute_t *o;
    UUI_CREATE_IMPLEMENTATION(o, uui_layout_absolute_initialize);
    o->allocated = 1;
    return o;
}

void uui_layout_absolute_free(uui_layout_absolute_t *absolute) {
    if(absolute->allocated)
        FreePool(absolute);
}
