#ifndef _UUI_RESOURCES_H
#define _UUI_RESOURCES_H

#include <list.h>

#define URC_SEEK_SET 0
#define URC_SEEK_CUR 1
#define URC_SEEK_END 2

typedef struct urc_context urc_context_t;
typedef struct urc_file urc_file_t;
typedef struct urc_source urc_source_t;

typedef int (*urc_file_read_t)(urc_file_t *file, void *buf, size_t *psize);
typedef int (*urc_file_seek_t)(urc_file_t *file, size_t *poffset, int whence);
typedef int (*urc_file_close_t)(urc_file_t *file);
typedef int (*urc_source_open_t)(urc_source_t *source, const char *path, urc_file_t **pfile);
typedef int (*urc_context_register_source_t)(urc_context_t *context, urc_source_t *source);
typedef int (*urc_context_register_source_container_t)(urc_context_t *context, void *ptr);
typedef int (*urc_context_open_t)(urc_context_t *context, const char *path, urc_file_t **pfile);

struct urc_context {
    listnode_t sources;

    urc_context_register_source_t register_source;
    urc_context_register_source_container_t register_source_container;
    urc_context_open_t open;
};

struct urc_file {
    urc_file_read_t read;
    urc_file_seek_t seek;
    urc_file_close_t close;
};

struct urc_source {
    listnode_t node;

    urc_source_open_t open;
};

urc_context_t* urc_context_create(void);

#endif
