#include <time.h>
#include <stdio.h>

#define ENABLE_SDL 1

#if ENABLE_SDL
#include <SDL/SDL.h>
#endif
#include <uui/canvas.h>
#include <uui/views/view.h>
#include <uui/views/viewgroup.h>
#include <uui/views/rect.h>
#include <uui/fb.h>
#if ENABLE_SDL
static SDL_Surface *window = NULL;
static uui_fb_t    * fb = NULL;
static uui_canvas_t *fbcanvas = NULL;
#endif

void hw_fb_flush(uui_canvas_t *canvas, uui_point_t start, uui_size_t size) {
#if ENABLE_SDL
    uui_canvas_copy(fbcanvas, canvas, start, start, size);
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

int maintest(int width, int height);

int main(void) {
    int rc;

#if ENABLE_SDL
    if(SDL_Init(SDL_INIT_VIDEO) < 0) {
        fprintf(stderr, "can't init SDL: %s\n", SDL_GetError());
        return 1;
    }

    window = SDL_SetVideoMode(720/2, 1280/2, 32, SDL_HWSURFACE);
    if(window==NULL) {
        window = SDL_SetVideoMode(300, 600, 32, SDL_SWSURFACE);
        if(window==NULL) {
            fprintf(stderr, "can't create SDL surface: %s\n", SDL_GetError());
            rc = 1;
            goto sdl_quit;
        }
    }

    fb = malloc(sizeof(*fb));
    fb->pixels = window->pixels;
    fb->size.width = window->w;
    fb->size.height = window->h;
    uui_fb_reset_dirty_region(fb);

    fbcanvas = uui_canvas_framebuffer_create(fb);

    maintest(window->w, window->h);
#else
    maintest(720, 1280);
    rc = 0;
#endif

#if ENABLE_SDL
sdl_quit:
    SDL_Quit();
#endif

    return rc;
}
