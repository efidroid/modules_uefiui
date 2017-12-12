#include <time.h>
#include <stdio.h>
#include <fcntl.h>

#include <uui/canvas.h>
#include <uui/views/view.h>
#include <uui/views/viewgroup.h>
#include <uui/views/rect.h>
#include <uui/fb.h>

int uui_platform_init_hw(void);
urc_source_t* uui_platform_get_default_source(void);

static void uui_context_sync_fb(uui_context_t *context, uui_canvas_t *canvas, uui_rect_t region) {
#if ENABLE_SDL
    uui_canvas_copy(hwfbcanvas, canvas, region.pos, region.pos, region.size);
    SDL_Flip(window);
#endif
}

void hw_mainloop_cb(void) {
#if ENABLE_SDL
    static SDL_Event event;

    if(SDL_PollEvent(&event)) {
        if(event.type == SDL_QUIT) {
            SDL_Quit();
            exit(1);
        }

        if (event.type == SDL_MOUSEMOTION) {
            if (SDL_GetMouseState(NULL, NULL) & SDL_BUTTON(SDL_BUTTON_LEFT)) {
            }
        }
    }
#endif
}

int main(void) {
    int rc = 0;

    // init hw
    uui_platform_init_hw();

    // create compcontext
    uui_comp_context_t *ccontext = uui_comp_create_context();
    ccontext->urc->register_source(ccontext->urc, uui_platform_get_default_source());
    uui_comp_add_path(ccontext, "/components");

    // load main component
    uui_component_t *comp = uui_component_load(ccontext, "app.Main");
    ASSERT(comp);
    ASSERT(comp->allocate_view);
    uui_view_t *rootview = comp->allocate_view(comp);
    ASSERT(rootview);

    // enter render loop
    uui_render(NULL, rootview);

    return rc;
}
