#ifndef _UUI_LAYOUTS_ABSOLUTE_H
#define _UUI_LAYOUTS_ABSOLUTE_H

#include <uui/common.h>
#include <list.h>
#include <uui/views/view.h>
#include <uui/views/viewgroup.h>


typedef struct {
    listnode_t node;
    uui_view_t *view;

    uui_point_t position;
} uui_layoutparams_absolute_t;

typedef struct uui_layout_absolute uui_layout_absolute_t;

typedef void (*uui_layout_ansolute_add_view_t)(uui_layout_absolute_t *absolute, uui_view_t *view, uui_point_t position);
typedef uui_layoutparams_absolute_t* (*uui_layout_absolute_get_layoutparams_t)(uui_layout_absolute_t *absolute, uui_view_t *view);

struct uui_layout_absolute {
    uui_viewgroup_t viewgroup;

    // methods
    uui_layout_ansolute_add_view_t add_view;
    uui_layout_absolute_get_layoutparams_t get_layoutparams;

    // private
    int allocated;
    listnode_t children_params;
};

int uui_layout_absolute_initialize(uui_layout_absolute_t *absolute);
uui_layout_absolute_t* uui_layout_absolute_create(void);
void uui_layout_absolute_free(uui_layout_absolute_t *absolute);

#endif
