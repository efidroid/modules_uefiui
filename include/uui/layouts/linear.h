#ifndef _UUI_LAYOUTS_LINEAR_H
#define _UUI_LAYOUTS_LINEAR_H

#include <uui/common.h>
#include <list.h>
#include <uui/views/view.h>
#include <uui/views/viewgroup.h>

#define UUI_LAYOUT_LINEAR_ORIENTATION_VERTICAL 0
#define UUI_LAYOUT_LINEAR_ORIENTATION_HORIZONTAL 1

typedef struct {
    listnode_t node;
    uui_view_t *view;

    uintn_t weight;
    intn_t margin_left;
    intn_t margin_top;
    intn_t margin_right;
    intn_t margin_bottom;
} uui_layoutparams_linear_t;

typedef struct uui_layout_linear uui_layout_linear_t;

typedef void (*uui_layout_linear_set_orientation_t)(uui_layout_linear_t *linear, uintn_t orientation);
typedef void (*uui_layout_linear_add_view_t)(uui_layout_linear_t *linear, uui_view_t *view);
typedef uui_layoutparams_linear_t* (*uui_layout_linear_get_layoutparams_t)(uui_layout_linear_t *linear, uui_view_t *view);

struct uui_layout_linear {
    uui_viewgroup_t viewgroup;

    // methods
    uui_layout_linear_set_orientation_t set_orientation;
    uui_layout_linear_add_view_t add_view;
    uui_layout_linear_get_layoutparams_t get_layoutparams;

    // private
    int allocated;
    uintn_t orientation;
    listnode_t children_params;
    uui_view_layout_t view_layout;
    uui_view_comp_set_property_t view_comp_set_property;
};

int uui_layout_linear_initialize(uui_layout_linear_t *linear);
uui_layout_linear_t* uui_layout_linear_create(void);
void uui_layout_linear_free(uui_layout_linear_t *linear);

#endif
