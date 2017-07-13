#ifndef _UUI_VIEWS_VIEWGROUP_H
#define _UUI_VIEWS_VIEWGROUP_H

#include <uui/common.h>
#include <uui/views/view.h>

typedef struct uui_viewgroup uui_viewgroup_t;

typedef void (*uui_viewgroup_internal_add_view_t)(uui_viewgroup_t *viewgroup, uui_view_t *view);

struct uui_viewgroup {
    int allocated;
    uui_view_t view;
    listnode_t children;

    uui_viewgroup_internal_add_view_t internal_add_view;
};

int uui_viewgroup_initialize(uui_viewgroup_t *viewgroup);
uui_viewgroup_t* uui_viewgroup_create(void);
void uui_viewgroup_free(uui_viewgroup_t *viewgroup);

#endif
