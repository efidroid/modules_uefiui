#include <uui_platform.h>

#include <uui/canvas.h>
#include <uui/components.h>

typedef struct {
    uui_fb_t *fb;

    uui_canvas_t *fbcanvas;
    uui_view_t *rootview;
} context_t;

void hw_fb_flush(uui_canvas_t *canvas, uui_point_t start, uui_size_t size);
void hw_mainloop_cb(void);

static void fb_flush(context_t *context) {
    if ((context->fb->dirtyregion_end.x > context->fb->dirtyregion_start.x) && (context->fb->dirtyregion_end.y > context->fb->dirtyregion_start.y)) {
        uui_size_t size = {context->fb->dirtyregion_end.x-context->fb->dirtyregion_start.x, context->fb->dirtyregion_end.y-context->fb->dirtyregion_start.y};
        hw_fb_flush(context->fbcanvas, context->fb->dirtyregion_start, size);
        uui_fb_reset_dirty_region(context->fb);
    }
}

int maintest(uui_comp_context_t *ccontext, int width, int height) {
    int rc;
    context_t *context = AllocateZeroPool(sizeof(context_t));

    // init context
    context->fb = uui_fb_alloc(width, height);
    context->fbcanvas = uui_canvas_framebuffer_create(context->fb);

    // load main component
    uui_component_t *comp = uui_component_load(ccontext, "app.Main");
    ASSERT(comp);
    ASSERT(comp->allocate_view);

    context->rootview = comp->allocate_view(comp);
    ASSERT(context->rootview);

    uui_canvas_boundary_t boundary = {.offset = uui_point(0, 0), .size = uui_size(0, 0)};
    for(;;) {
        hw_mainloop_cb();

        if (context->rootview->invalid_flags & UUI_INVALID_MEASURE) {
            context->rootview->invalid_flags &= ~(UUI_INVALID_MEASURE);
            if (context->rootview->measure)
                context->rootview->measure(context->rootview, uui_measure_spec(context->fb->size.width, UUI_EXACTLY), uui_measure_spec(context->fb->size.height, UUI_EXACTLY));
        }

        if (context->rootview->invalid_flags & UUI_INVALID_LAYOUT) {
            context->rootview->invalid_flags &= ~(UUI_INVALID_LAYOUT);
            if (context->rootview->layout)
                context->rootview->layout(context->rootview, uui_point(0, 0), context->rootview->measured_size);
        }

        if (context->rootview->invalid_flags & UUI_INVALID_DRAW) {
            context->rootview->invalid_flags &= ~(UUI_INVALID_DRAW);
            if (context->rootview->draw) {
                // apply boundary
                boundary.offset = context->rootview->computed_position;
                boundary.size = context->rootview->computed_size;
                context->fbcanvas->boundary_push(context->fbcanvas, &boundary, 1);

                // draw
                context->rootview->draw(context->rootview, context->fbcanvas);

                // remove boundary
                context->fbcanvas->boundary_pop(context->fbcanvas, 1);
            }
            context->rootview->invalid_region = uui_rect(uui_point(0, 0), uui_size(0, 0));
        }

        // flush
        fb_flush(context);
        rc = 0;
    }

    return rc;
}
