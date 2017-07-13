#include <uui_platform.h>

#include <uui/canvas.h>
#include <uui/views/view.h>
#include <uui/views/viewgroup.h>
#include <uui/views/rect.h>
#include <uui/layouts/linear.h>
#include <uui/layouts/absolute.h>

#include <stdio.h>

typedef struct {
    uui_fb_t *fb;

    uui_canvas_t *fbcanvas;
    uui_view_t *rootview;
} context_t;

void hw_fb_flush(uui_canvas_t *canvas, uui_point_t start, uui_size_t size);
void hw_mainloop_cb(void);

static void fb_flush(context_t *context) {
#if 0
    context->fb->dirtyregion_start.x = 0;
    context->fb->dirtyregion_start.y = 0;
    context->fb->dirtyregion_end.x = context->fb->size.width;
    context->fb->dirtyregion_end.y = context->fb->size.height;
#endif

#if 0
    printf("%ux%u - %ux%u\n",
        context->fb->dirtyregion_start.x,
        context->fb->dirtyregion_start.y,
        context->fb->dirtyregion_end.x,
        context->fb->dirtyregion_end.y
    );
#endif

    if ((context->fb->dirtyregion_end.x > context->fb->dirtyregion_start.x) && (context->fb->dirtyregion_end.y > context->fb->dirtyregion_start.y)) {
        uui_size_t size = {context->fb->dirtyregion_end.x-context->fb->dirtyregion_start.x, context->fb->dirtyregion_end.y-context->fb->dirtyregion_start.y};
        hw_fb_flush(context->fbcanvas, context->fb->dirtyregion_start, size);
        uui_fb_reset_dirty_region(context->fb);
    }
}

uui_view_rect_t *g_touch_rect = NULL;

int maintest(int width, int height) {
    int rc;
    context_t *context = calloc(sizeof(context_t), 1);

    // init context
    context->fb = uui_fb_alloc(width, height);
    context->fbcanvas = uui_canvas_framebuffer_create(context->fb);

    uui_layout_absolute_t *absolute = uui_layout_absolute_create();
    context->rootview = &absolute->viewgroup.view;

    if(1){
    // add rect
    uui_view_rect_t *rect = uui_view_rect_create();
    rect->color = uui_pixel(0, 0, 0, 0);
    rect->view.layout_size = uui_size(UUI_MATCH_PARENT, UUI_MATCH_PARENT);
    absolute->add_view(absolute, &rect->view, uui_point(0, 0));
    }

    uui_layout_linear_t *linear = uui_layout_linear_create();
    linear->viewgroup.view.layout_size = uui_size(UUI_WRAP_CONTENT, UUI_MATCH_PARENT);
    linear->set_orientation(linear, UUI_LAYOUT_LINEAR_ORIENTATION_HORIZONTAL);
    absolute->add_view(absolute, &linear->viewgroup.view, uui_point(0, 0));

    uui_layout_linear_t *linear2 = uui_layout_linear_create();
    linear2->set_orientation(linear2, UUI_LAYOUT_LINEAR_ORIENTATION_VERTICAL);
    linear2->viewgroup.view.layout_size = uui_size(UUI_MATCH_PARENT, UUI_WRAP_CONTENT);
    linear->add_view(linear, &linear2->viewgroup.view);

    {
    // add rect
    uui_view_rect_t *rect = uui_view_rect_create();
    rect->color = uui_pixel(0, 0, 255, 0);
    rect->view.layout_size = uui_size(50, 50);
    linear2->add_view(linear2, &rect->view);
    uui_layoutparams_linear_t *lp = linear2->get_layoutparams(linear2, &rect->view);
    lp->margin_left = 5;
    lp->margin_top = 5;
    //lp->margin_right = 5;
    lp->margin_bottom = 5;
    }

    {
    // add rect
    uui_view_rect_t *rect = uui_view_rect_create();
    rect->color = uui_pixel(0, 255, 255, 0);
    rect->view.layout_size = uui_size(50, 50);
    linear2->add_view(linear2, &rect->view);
    uui_layoutparams_linear_t *lp = linear2->get_layoutparams(linear2, &rect->view);
    lp->margin_left = 5;
    lp->margin_top = 5;
    //lp->margin_right = 5;
    lp->margin_bottom = 5;
    }

    {
    // add rect
    uui_view_rect_t *rect = uui_view_rect_create();
    rect->color = uui_pixel(255, 0, 255, 0);
    rect->view.layout_size = uui_size(UUI_MATCH_PARENT, 50);
    linear2->add_view(linear2, &rect->view);
    uui_layoutparams_linear_t *lp = linear2->get_layoutparams(linear2, &rect->view);
    lp->margin_left = 5;
    lp->margin_top = 5;
    //lp->margin_right = 5;
    lp->margin_bottom = 5;
    }

    {
    // add rect
    uui_view_rect_t *rect = uui_view_rect_create();
    rect->color = uui_pixel(0, 255, 255, 0);
    rect->view.layout_size = uui_size(50, 50);
    linear2->add_view(linear2, &rect->view);
    uui_layoutparams_linear_t *lp = linear2->get_layoutparams(linear2, &rect->view);
    lp->margin_left = 5;
    lp->margin_top = 5;
    lp->margin_right = 5;
    lp->margin_bottom = 5;
    }

    {
    // add rect
    uui_view_rect_t *rect = uui_view_rect_create();
    rect->color = uui_pixel(0, 255, 0, 0);
    rect->view.layout_size = uui_size(UUI_MATCH_PARENT, UUI_MATCH_PARENT);
    linear->add_view(linear, &rect->view);
    }

    {
    // add rect
    uui_view_rect_t *rect = uui_view_rect_create();
    rect->color = uui_pixel(255, 0, 0, 0);
    rect->view.layout_size = uui_size(10, 10);
    linear->add_view(linear, &rect->view);
    }

    {
    // add rect
    uui_view_rect_t *rect = uui_view_rect_create();
    rect->color = uui_pixel(255, 255, 0, 0);
    rect->view.layout_size = uui_size(UUI_MATCH_PARENT, UUI_MATCH_PARENT);
    linear->add_view(linear, &rect->view);
    uui_layoutparams_linear_t *lp = linear->get_layoutparams(linear, &rect->view);
    lp->weight = 3;
    lp->margin_left = 10;
    lp->margin_top = 10;
    lp->margin_right = 10;
    lp->margin_bottom = 10;
    }

    uui_layout_linear_t *linear3 = uui_layout_linear_create();
    linear3->set_orientation(linear3, UUI_LAYOUT_LINEAR_ORIENTATION_VERTICAL);
    linear3->viewgroup.view.layout_size = uui_size(UUI_MATCH_PARENT, 50);
    linear->add_view(linear, &linear3->viewgroup.view);

    {
    // add rect
    uui_view_rect_t *rect = uui_view_rect_create();
    rect->color = uui_pixel(255, 255, 255, 0);
    rect->view.layout_size = uui_size(50, 50);
    linear3->add_view(linear3, &rect->view);
    }

    {
    // add rect
    uui_view_rect_t *rect = uui_view_rect_create();
    rect->color = uui_pixel(128, 255, 0, 0);
    rect->view.layout_size = uui_size(UUI_MATCH_PARENT, UUI_MATCH_PARENT);
    linear3->add_view(linear3, &rect->view);
    }

    uui_view_rect_t *rect = uui_view_rect_create();
    rect->color = uui_pixel(128, 128, 128, 0);
    rect->view.layout_size = uui_size(UUI_MATCH_PARENT, 10);
    absolute->add_view(absolute, &rect->view, uui_point(0, 0));
    uui_layoutparams_absolute_t *lp_debugrect = absolute->get_layoutparams(absolute, &rect->view);

    uui_canvas_boundary_t boundary = {.offset = uui_point(0, 0), .size = uui_size(0, 0)};
    int first = 1;
    int dir = 0;
    for(;;) {
        hw_mainloop_cb();

        if(first) {
            context->rootview->measure(context->rootview, uui_measure_spec(context->fb->size.width, UUI_EXACTLY), uui_measure_spec(context->fb->size.height, UUI_EXACTLY));
            context->rootview->layout(context->rootview, uui_point(0, 0), context->rootview->measured_size);
            //first = 0;
        }
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

#if 1
        if(dir)
            lp_debugrect->position.y+=1;
        else
            lp_debugrect->position.y-=1;

        if (lp_debugrect->position.y >= context->fb->size.height)
            dir = !dir;

        rect->view.invalidate(&rect->view);
#endif

        // flush
        fb_flush(context);
        rc = 0;
        //break;
    }

    return rc;
}
