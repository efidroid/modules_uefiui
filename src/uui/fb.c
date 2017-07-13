#include <uui_platform.h>

#include <uui/fb.h>

uui_fb_t *uui_fb_alloc(uintn_t width, uintn_t height) {
    uui_pixel_t *pixels = calloc(width * height * sizeof(uui_pixel_t), 1);
    if (pixels==NULL)
        return NULL;

    uui_fb_t *fb = calloc(sizeof(uui_fb_t), 1);
    if (fb==NULL) {
        FreePool(pixels);
        return NULL;
    }

    fb->pixels = pixels;
    fb->size.width = width;
    fb->size.height = height;
    uui_fb_reset_dirty_region(fb);

    return fb;
}

void uui_fb_free(uui_fb_t *fb) {
    if (fb==NULL)
        return;

    FreePool(fb->pixels);
    FreePool(fb);
}

void uui_fb_reset_dirty_region(uui_fb_t *fb) {
    fb->dirtyregion_start.x = fb->size.width;
    fb->dirtyregion_end.x = 0;
    fb->dirtyregion_start.y = fb->size.height;
    fb->dirtyregion_end.y = 0;
}
