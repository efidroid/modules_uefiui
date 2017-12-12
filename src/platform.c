#define ENABLE_SDL 1

#if ENABLE_SDL
#include <SDL/SDL.h>
#endif

typedef struct {
    urc_source_t source;
    int dirfd;
} urc_source_fs_t;

typedef struct {
    urc_file_t file;
    int fd;
} urc_file_fs_t;

#if ENABLE_SDL
static SDL_Surface  *window = NULL;
static uui_fb_t     *hwfb = NULL;
static uui_canvas_t *hwfbcanvas = NULL;
#endif

int uui_platform_init_hw(void) {
    int rc;

#if ENABLE_SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        fprintf(stderr, "can't init SDL: %s\n", SDL_GetError());
        return 1;
    }

    window = SDL_SetVideoMode(720/2, 1280/2, 32, SDL_HWSURFACE);
    if (window==NULL) {
        window = SDL_SetVideoMode(300, 600, 32, SDL_SWSURFACE);
        if (window==NULL) {
            fprintf(stderr, "can't create SDL surface: %s\n", SDL_GetError());
            rc = 1;
            goto sdl_quit;
        }
    }

    hwfb = malloc(sizeof(*hwfb));
    hwfb->pixels = window->pixels;
    hwfb->size.width = window->w;
    hwfb->size.height = window->h;
    uui_fb_reset_dirty_region(hwfb);
    hwfbcanvas = uui_canvas_framebuffer_create(hwfb);

    uui_context_set_default_rendercanvas(uui_canvas_create(window->w, window->h));
#else
    uui_context_set_default_rendercanvas(uui_canvas_create(720, 1280));
#endif

    rc = 0;

#if ENABLE_SDL
sdl_quit:
    SDL_Quit();
#endif
    return rc;
}

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

static int urc_context_register_source_fs(const char *path) {
    int dirfd = open(path, O_RDONLY);
    if (dirfd < 0)
        return -1;

    urc_source_fs_t *isource = AllocateZeroPool(sizeof(urc_source_fs_t));
    if(!isource) return -1;

    isource->source.open = urc_source_fs_open;
    isource->dirfd = dirfd;

    return &isource->source;
}

urc_source_t* uui_platform_get_default_source(void) {
    return urc_context_register_source_fs("/media/Data/repositories/git/efidroid2/modules/uefiui/resources");
}

uui_context_t *uui_platform_get_hw_context(void) {

}
