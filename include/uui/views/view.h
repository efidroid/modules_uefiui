#ifndef _UUI_VIEWS_VIEW_H
#define _UUI_VIEWS_VIEW_H

#include <uui/common.h>
#include <uui/canvas.h>

#define UUI_MATCH_PARENT ((uintn_t)-1)
#define UUI_WRAP_CONTENT ((uintn_t)-2)

#define UUI_AT_MOST      0x8
#define UUI_EXACTLY      0x4
#define UUI_UNSPECIFIED  0x0

#define UUI_MEASURED_STATE_TOO_SMALL 0x1

#define UUI_INVALID_MEASURE 0x1
#define UUI_INVALID_LAYOUT  0x2
#define UUI_INVALID_DRAW    0x4

typedef struct {
    uintn_t size;
    uint8_t mode;
} uui_measure_spec_t;

struct uui_viewgroup;

typedef struct uui_view uui_view_t;

typedef void (*uui_view_measure_t)(uui_view_t *view, uui_measure_spec_t measure_spec_width, uui_measure_spec_t measure_spec_height);
typedef void (*uui_view_layout_t)(uui_view_t *view, uui_point_t position, uui_size_t size);
typedef void (*uui_view_draw_t)(uui_view_t *view, uui_canvas_t *canvas);
typedef void (*uui_view_invalidate_t)(uui_view_t *view, uintn_t flags, uui_rect_t *rect);

struct uui_view {
    int allocated;

    listnode_t node;

    // set by layout manager
    uui_size_t measured_size;
    uintn_t measured_state[2];
    uui_point_t computed_position;
    uui_size_t computed_size;
    uui_point_t old_computed_position;
    uui_size_t old_computed_size;
    struct uui_viewgroup *parent;

    uui_rect_t invalid_region;
    uintn_t invalid_flags;

    uui_size_t layout_size;

    // callbacks
    uui_view_measure_t measure;
    uui_view_layout_t layout;
    uui_view_draw_t draw;
    uui_view_invalidate_t invalidate;
};

int uui_view_initialize(uui_view_t *view);
uui_view_t* uui_view_create(void);
void uui_view_free(uui_view_t *view);

static inline uui_measure_spec_t uui_measure_spec(uintn_t size, uint8_t mode) {
    return (uui_measure_spec_t) {size, mode};
}

static inline void uui_resolve_size_and_state(uintn_t *psize, uintn_t *pstate, uui_measure_spec_t measure_spec) {
    switch (measure_spec.mode) {
        case UUI_AT_MOST:
            if (measure_spec.size < *psize) {
                *psize = measure_spec.size;
                if(pstate)
                    *pstate |= UUI_MEASURED_STATE_TOO_SMALL;
            }
            break;
        case UUI_EXACTLY:
            *psize = measure_spec.size;
            break;
    }
}


#endif
