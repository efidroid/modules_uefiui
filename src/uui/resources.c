#include <common.h>
#include <uui/resources.h>

typedef struct {
    urc_source_t source;

    uint8_t *data;
} urc_source_container_t;

typedef struct {
    urc_file_t file;

    uint8_t *data;
    size_t datasize;
    size_t position;
} urc_file_container_t;

static int urc_file_container_read(urc_file_t *file, void *buf, size_t *psize) {
    urc_file_container_t* ifile = containerof(file, urc_file_container_t, file);
    ASSERT(ifile->position <= ifile->datasize);

    if (!psize)
        return -1;

    size_t tocopy = MIN(ifile->datasize-ifile->position, *psize);
    if (tocopy > 0) {
        CopyMem(buf, ifile->data + ifile->position, tocopy);
        ifile->position += tocopy;
    }
    *psize = tocopy;
    ASSERT(ifile->position <= ifile->datasize);

    return 0;
}

static int urc_file_container_seek(urc_file_t *file, size_t *poffset, int whence) {
    urc_file_container_t* ifile = containerof(file, urc_file_container_t, file);

    if (!poffset)
        return -1;

    size_t offset = *poffset;
    switch (whence) {
        case URC_SEEK_SET:
            if (offset > ifile->datasize)
                return -1;

            ifile->position = offset;
            break;

        case URC_SEEK_CUR:
            if (ifile->position + offset > ifile->datasize)
                return -1;

            ifile->position += offset;
            break;

        case URC_SEEK_END:
            if (offset != 0)
                return -1;

            ifile->position = ifile->datasize;
            break;

        default:
            return -1;
    }

    *poffset = ifile->position;

    return 0;
}

static int urc_file_container_close(urc_file_t *file) {
    urc_file_container_t* ifile = containerof(file, urc_file_container_t, file);

    FreePool(ifile);

    return 0;
}

static int urc_source_container_open(urc_source_t *source, const char *path, urc_file_t **pfile) {
    uint32_t i;
    uint32_t num_files;
    urc_source_container_t* isource = containerof(source, urc_source_container_t, source);
    uint8_t *data = isource->data;

    if (CompareMem(data, "URCC", 4))
        return -1;
    data += 4;

    CopyMem(&num_files, data, sizeof(num_files));
    data +=4;

    for (i=0; i<num_files; i++) {
        uint64_t namesize;
        uint64_t filesize;

        if (CompareMem(data, "URCF", 4))
            return -1;
        data +=4;

        CopyMem(&namesize, data, sizeof(namesize));
        data += 8;
        CopyMem(&filesize, data, sizeof(filesize));
        data += 8;

        if (AsciiStrCmp((const char*)data, path)) {
            data += ALIGN_VALUE(namesize, 4);
            data += ALIGN_VALUE(filesize, 4);
            continue;
        }
        data += ALIGN_VALUE(namesize, 4);

        urc_file_container_t *ifile = AllocateZeroPool(sizeof(urc_file_container_t));
        if (!ifile) return -1;

        ifile->file.read = urc_file_container_read;
        ifile->file.seek = urc_file_container_seek;
        ifile->file.close = urc_file_container_close;
        ifile->data = data;
        ifile->datasize = filesize;
        ifile->position = 0;

        *pfile = &ifile->file;
        return 0;
    }

    return -1;
}

static int urc_context_register_source(urc_context_t *context, urc_source_t *source) {
    list_add_tail(&context->sources, &source->node);
    return 0;
}

static int urc_context_register_source_container(urc_context_t *context, void *ptr) {
    urc_source_container_t *isource = AllocateZeroPool(sizeof(urc_source_container_t));
    if(!isource) return -1;

    isource->source.open = urc_source_container_open;
    isource->data = ptr;

    return context->register_source(context, &isource->source);
}

static int urc_context_open(urc_context_t *context, const char *path, urc_file_t **pfile) {
    urc_source_t *source;
    int rc;
    urc_file_t *file;

    list_for_every_entry(&context->sources, source, urc_source_t, node) {
        file = NULL;
        rc = source->open(source, path, &file);
        if (rc==0 && file) {
            *pfile = file;
            return 0;
        }
    }

    return -1;
}

urc_context_t* urc_context_create(void) {
    urc_context_t *context = AllocateZeroPool(sizeof(urc_context_t));
    if (!context) return NULL;

    list_initialize(&context->sources);
    context->register_source = urc_context_register_source;
    context->register_source_container = urc_context_register_source_container;
    context->open = urc_context_open;

    return context;
}
