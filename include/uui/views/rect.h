#ifndef _UUI_VIEWS_RECT_H
#define _UUI_VIEWS_RECT_H

#include <uui/common.h>
#include <uui/views/view.h>

typedef struct {
    int allocated;

    uui_view_t view;
    uui_pixel_t color;

    // private
    uui_view_comp_set_property_t view_comp_set_property;
} uui_view_rect_t;

int uui_view_rect_initialize(uui_view_rect_t *rect);
uui_view_rect_t* uui_view_rect_create(void);
void uui_view_rect_free(uui_view_rect_t *rect);

#endif
