#ifndef _UUI_CONTEXT_H
#define _UUI_CONTEXT_H

#include <uui/fb.h>

typedef struct uui_context uui_context_t;

typedef void (*uui_context_sync_fb)(uui_context_t *context, uui_canvas_t *canvas, uui_rect_t region);
typedef void (*uui_context_set_rendercanvas)(uui_context_t *context, uui_canvas_t *canvas);

struct uui_context {
    uui_context_sync_fb sync_fb;

    // private
    uui_canvas_t *rendercanvas;
};

uui_context_t* uui_context_create(void);

#endif
