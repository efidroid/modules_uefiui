#include <time.h>
#include <stdio.h>
#include <fcntl.h>

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


int maintest(uui_comp_context_t *ccontext, int width, int height);

typedef struct {
    urc_source_t source;

    int dirfd;
} urc_source_fs_t;

typedef struct {
    urc_file_t file;

    int fd;
} urc_file_fs_t;

static int urc_file_fs_read(urc_file_t *file, void *buf, size_t *psize) {
    urc_file_fs_t* ifile = containerof(file, urc_file_fs_t, file);
    ssize_t nbytes;

    if (!psize)
        return -1;

    nbytes = read(ifile->fd, buf, *psize);
    if (nbytes == -1)
        return -1;

    *psize = (size_t) nbytes;
    return 0;
}

static int urc_file_fs_seek(urc_file_t *file, size_t *poffset, int whence) {
    urc_file_fs_t* ifile = containerof(file, urc_file_fs_t, file);
    off_t newoffset;

    if (!poffset)
        return -1;

    size_t offset = *poffset;
    switch (whence) {
        case URC_SEEK_SET:
            newoffset = lseek(ifile->fd, (off_t)offset, SEEK_SET);
            break;

        case URC_SEEK_CUR:
            newoffset = lseek(ifile->fd, (off_t)offset, SEEK_CUR);
            break;

        case URC_SEEK_END:
            newoffset = lseek(ifile->fd, (off_t)offset, SEEK_END);
            break;

        default:
            return -1;
    }

    if (newoffset == (off_t)-1)
        return -1;

    *poffset = newoffset;
    return 0;
}

static int urc_file_fs_close(urc_file_t *file) {
    urc_file_fs_t* ifile = containerof(file, urc_file_fs_t, file);
    close(ifile->fd);
    FreePool(ifile);

    return 0;
}

static int urc_source_fs_open(urc_source_t *source, const char *path, urc_file_t **pfile) {
    urc_source_fs_t* isource = containerof(source, urc_source_fs_t, source);
    int fd = openat(isource->dirfd, path+1, O_RDONLY);
    if (fd < 0)
        return -1;

    urc_file_fs_t *ifile = AllocateZeroPool(sizeof(urc_file_fs_t));
    if (!ifile) return -1;

    ifile->file.read = urc_file_fs_read;
    ifile->file.seek = urc_file_fs_seek;
    ifile->file.close = urc_file_fs_close;
    ifile->fd = fd;

    *pfile = &ifile->file;
    return 0;
}

static int urc_context_register_source_fs(urc_context_t *context, const char *path) {
    int dirfd = open(path, O_RDONLY);
    if (dirfd < 0)
        return -1;

    urc_source_fs_t *isource = AllocateZeroPool(sizeof(urc_source_fs_t));
    if(!isource) return -1;

    isource->source.open = urc_source_fs_open;
    isource->dirfd = dirfd;

    return context->register_source(context, &isource->source);
}

int main(void) {
    int rc = 0;

    uui_comp_context_t *ccontext = uui_comp_create_context();
    urc_context_register_source_fs(ccontext->urc, "/media/Data/repositories/git/efidroid2/modules/uefiui/resources");
    uui_comp_add_path(ccontext, "/components");

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

    maintest(ccontext, window->w, window->h);
#else
    maintest(ccontext, 720, 1280);
    rc = 0;
#endif

#if ENABLE_SDL
sdl_quit:
    SDL_Quit();
#endif

    return rc;
}
