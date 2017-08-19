#include <uui/layouts/linear.h>

static inline uui_measure_spec_t get_child_spec(intn_t size, intn_t margin, uui_measure_spec_t measure_spec) {
    uui_measure_spec_t childspec;
    switch (size) {
        case UUI_WRAP_CONTENT:
            childspec.size = measure_spec.size - margin;
            childspec.mode = UUI_UNSPECIFIED;
            break;

        case UUI_MATCH_PARENT:
            childspec.size = measure_spec.size - margin;
            childspec.mode = UUI_AT_MOST;
            break;

        default:
            childspec.size = size;
            childspec.mode = UUI_EXACTLY;
            break;
    }

    return childspec;
}

static void uui_layout_linear_measure(uui_view_t *view, uui_measure_spec_t measure_spec_width, uui_measure_spec_t measure_spec_height) {
    uui_layout_linear_t* linear = containerof(view, uui_layout_linear_t, viewgroup.view);
    uui_view_t *childview;
    uui_layoutparams_linear_t *lp;

    intn_t fixed_length = 0;
    uintn_t num_match_parents = 0;
    uintn_t match_parents_weight_sum = 0;

    view->measured_size = uui_size(0, 0);
    view->measured_state[0] = 0;
    view->measured_state[1] = 0;

    list_for_every_entry(&linear->children_params, lp, uui_layoutparams_linear_t, node) {
        childview = lp->view;

        uui_size_t layout_size = childview->layout_size;
        uui_measure_spec_t child_measure_spec_width = get_child_spec(layout_size.width, lp->margin_left+lp->margin_right, measure_spec_width);
        uui_measure_spec_t child_measure_spec_height = get_child_spec(layout_size.height, lp->margin_top+lp->margin_bottom, measure_spec_height);

        if (linear->orientation==UUI_LAYOUT_LINEAR_ORIENTATION_VERTICAL && layout_size.width != UUI_MATCH_PARENT) {
            if (childview->measure && (childview->invalid_flags & UUI_INVALID_MEASURE)) {
                childview->invalid_flags &= ~(UUI_INVALID_MEASURE);
                childview->measure(childview, child_measure_spec_width, child_measure_spec_height);
            }

            intn_t length = childview->measured_size.width + lp->margin_left + lp->margin_right;
            fixed_length += length;
            view->measured_size.width += length;
            view->measured_size.height = MAX(view->measured_size.height, childview->measured_size.height + lp->margin_top + lp->margin_bottom);

        }
        else if (linear->orientation==UUI_LAYOUT_LINEAR_ORIENTATION_HORIZONTAL && layout_size.height != UUI_MATCH_PARENT) {
            if (childview->measure && (childview->invalid_flags & UUI_INVALID_MEASURE)) {
                childview->invalid_flags &= ~(UUI_INVALID_MEASURE);
                childview->measure(childview, child_measure_spec_width, child_measure_spec_height);
            }

            intn_t length = childview->measured_size.height + lp->margin_top + lp->margin_bottom;
            fixed_length += length;
            view->measured_size.width = MAX(view->measured_size.width, childview->measured_size.width + lp->margin_left + lp->margin_right);
            view->measured_size.height += length;
        }
        else {
            num_match_parents++;
            match_parents_weight_sum += lp->weight;
        }
    }

    if (num_match_parents) {
        intn_t matchparent_length;
        switch (linear->orientation) {
            case UUI_LAYOUT_LINEAR_ORIENTATION_HORIZONTAL:
                matchparent_length = (measure_spec_height.size-fixed_length)/match_parents_weight_sum;
                break;

            case UUI_LAYOUT_LINEAR_ORIENTATION_VERTICAL:
            default:
                matchparent_length = (measure_spec_width.size-fixed_length)/match_parents_weight_sum;
                break;
        }

        uintn_t i = 0;
        list_for_every_entry(&linear->children_params, lp, uui_layoutparams_linear_t, node) {
            childview = lp->view;

            // get layout size
            uui_size_t layout_size = childview->layout_size;

            if (linear->orientation==UUI_LAYOUT_LINEAR_ORIENTATION_VERTICAL && layout_size.width!=UUI_MATCH_PARENT)
                continue;
            if (linear->orientation==UUI_LAYOUT_LINEAR_ORIENTATION_HORIZONTAL && layout_size.height!=UUI_MATCH_PARENT)
                continue;

            if (linear->orientation==UUI_LAYOUT_LINEAR_ORIENTATION_VERTICAL) {
                uui_measure_spec_t widthspec = uui_measure_spec(matchparent_length * lp->weight, UUI_EXACTLY);

                // account for rounding
                if (i==num_match_parents-1) {
                    intn_t diff = (measure_spec_width.size-fixed_length) - matchparent_length*match_parents_weight_sum;
                    if(diff) {
                        widthspec.size += diff;
                    }
                }

                uui_measure_spec_t child_measure_spec_width = get_child_spec(layout_size.width, lp->margin_left+lp->margin_right, widthspec);
                uui_measure_spec_t child_measure_spec_height = get_child_spec(layout_size.height, lp->margin_top+lp->margin_bottom, measure_spec_height);

                if (childview->measure && (childview->invalid_flags & UUI_INVALID_MEASURE)) {
                    childview->invalid_flags &= ~(UUI_INVALID_MEASURE);
                    childview->measure(childview, child_measure_spec_width, child_measure_spec_height);
                }

                intn_t length = childview->measured_size.width + lp->margin_left + lp->margin_right;
                view->measured_size.width += length;
                view->measured_size.height = MAX(view->measured_size.height, childview->measured_size.height + lp->margin_top + lp->margin_bottom);

            }
            else if (linear->orientation==UUI_LAYOUT_LINEAR_ORIENTATION_HORIZONTAL) {
                uui_measure_spec_t heightspec = uui_measure_spec(matchparent_length * lp->weight, UUI_EXACTLY);

                // account for rounding
                if (i==num_match_parents-1) {
                    intn_t diff = (measure_spec_height.size-fixed_length) - matchparent_length*match_parents_weight_sum;
                    if(diff) {
                        heightspec.size += diff;
                    }
                }

                uui_measure_spec_t child_measure_spec_width = get_child_spec(layout_size.width, lp->margin_left+lp->margin_right, measure_spec_width);
                uui_measure_spec_t child_measure_spec_height = get_child_spec(layout_size.height, lp->margin_top+lp->margin_bottom, heightspec);

                if (childview->measure && (childview->invalid_flags & UUI_INVALID_MEASURE)) {
                    childview->invalid_flags &= ~(UUI_INVALID_MEASURE);
                    childview->measure(childview, child_measure_spec_width, child_measure_spec_height);
                }

                intn_t length = childview->measured_size.height + lp->margin_top + lp->margin_bottom;
                view->measured_size.width = MAX(view->measured_size.width, childview->measured_size.width + lp->margin_left + lp->margin_right);
                view->measured_size.height += length;
            }

            i++;
        }
    }

    if (view->layout_size.width==UUI_MATCH_PARENT) {
        view->measured_size.width = measure_spec_width.size;
    }
    if (view->layout_size.height==UUI_MATCH_PARENT) {
        view->measured_size.height = measure_spec_height.size;
    }

    uui_resolve_size_and_state(&view->measured_size.width, &view->measured_state[0], measure_spec_width);
    uui_resolve_size_and_state(&view->measured_size.height, &view->measured_state[1], measure_spec_height);
}

static void uui_layout_linear_layout(uui_view_t *view, uui_point_t position, uui_size_t size) {
    uui_layout_linear_t* linear = containerof(view, uui_layout_linear_t, viewgroup.view);
    uui_point_t pos = uui_point(0, 0);

    uui_view_t *childview;
    uui_layoutparams_linear_t *lp;
    list_for_every_entry(&linear->children_params, lp, uui_layoutparams_linear_t, node) {
        childview = lp->view;

        if (childview->layout && (childview->invalid_flags & UUI_INVALID_LAYOUT)) {
            childview->invalid_flags &= ~(UUI_INVALID_LAYOUT);
            childview->layout(childview, pos, childview->measured_size);
        }

        switch (linear->orientation) {
            case UUI_LAYOUT_LINEAR_ORIENTATION_HORIZONTAL:
                pos.x = lp->margin_left;
                pos.y += lp->margin_top;

                childview->computed_position = pos;
                pos.y += childview->computed_size.height;

                pos.y += lp->margin_bottom;
                break;

            case UUI_LAYOUT_LINEAR_ORIENTATION_VERTICAL:
            default:
                pos.x += lp->margin_left;
                pos.y = lp->margin_top;

                childview->computed_position = pos;
                pos.x += childview->computed_size.width;

                pos.x += lp->margin_right;
                break;
        }
    }

    linear->view_layout(view, position, size);
}

static void uui_layout_linear_set_orientation(uui_layout_linear_t *linear, uintn_t orientation) {
    linear->orientation = orientation;
}

static void uui_layout_linear_add_view(uui_layout_linear_t *linear, uui_view_t *view) {
    uui_layoutparams_linear_t *lp = AllocatePool(sizeof(uui_layoutparams_linear_t));
    if (!lp) return;

    lp->view = view;
    list_add_tail(&linear->children_params, &lp->node);
    lp->weight = 1;
    lp->margin_left = 0;
    lp->margin_top = 0;
    lp->margin_right = 0;
    lp->margin_bottom = 0;

    linear->viewgroup.internal_add_view(&linear->viewgroup, view);
}

static uui_layoutparams_linear_t* uui_layout_linear_get_layoutparams(uui_layout_linear_t *linear, uui_view_t *view){
    uui_layoutparams_linear_t *lp;
    list_for_every_entry(&linear->children_params, lp, uui_layoutparams_linear_t, node) {
        if (lp->view == view)
            return lp;
    }
    return NULL;
}

int uui_layout_linear_initialize(uui_layout_linear_t *linear) {
    SetMem(linear, sizeof(*linear), 0);
    uui_viewgroup_initialize(&linear->viewgroup);
    linear->view_layout = linear->viewgroup.view.layout;
    linear->viewgroup.view.measure = uui_layout_linear_measure;
    linear->viewgroup.view.layout = uui_layout_linear_layout;

    list_initialize(&linear->children_params);
    linear->set_orientation = uui_layout_linear_set_orientation;
    linear->orientation = UUI_LAYOUT_LINEAR_ORIENTATION_VERTICAL;
    linear->add_view = uui_layout_linear_add_view;
    linear->get_layoutparams = uui_layout_linear_get_layoutparams;

    return 0;
}

uui_layout_linear_t* uui_layout_linear_create(void) {
    uui_layout_linear_t *o;
    UUI_CREATE_IMPLEMENTATION(o, uui_layout_linear_initialize);
    o->allocated = 1;
    return o;
}

void uui_layout_linear_free(uui_layout_linear_t *linear) {
    if(linear->allocated)
        FreePool(linear);
}
