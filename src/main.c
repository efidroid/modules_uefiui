#include <time.h>
#include <stdio.h>

#include <SDL/SDL.h>
#include <uui/canvas.h>
#include <uui/views/view.h>
#include <uui/views/viewgroup.h>
#include <uui/views/rect.h>
#include <uui/fb.h>

static SDL_Surface *window = NULL;
static uui_fb_t    * fb = NULL;
static uui_canvas_t *fbcanvas = NULL;

void hw_fb_flush(uui_canvas_t *canvas, uui_point_t start, uui_size_t size) {
    uui_canvas_copy(fbcanvas, canvas, start, start, size);
    SDL_Flip(window);
}

void hw_mainloop_cb(void) {
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
}

int maintest(int width, int height);

int main(void) {
    int rc;

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

sdl_quit:
    SDL_Quit();

    return rc;
}
