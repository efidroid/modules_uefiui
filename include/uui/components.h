#ifndef _UUI_COMPONENTS_H
#define _UUI_COMPONENTS_H

#include <lib/hashmap.h>
#include <list.h>
#include <uui/views/view.h>
#include <uui/resources.h>

struct uui_view;

typedef struct {
    listnode_t paths;
    Hashmap *components;
    urc_context_t *urc;
} uui_comp_context_t;

typedef struct uui_component uui_component_t;

typedef struct uui_view* (*uui_component_allocate_view_t)(uui_component_t *component);

typedef enum {
    UUI_COMPONENT_VALUETYPE_BOOLEAN,
    UUI_COMPONENT_VALUETYPE_STRING,
    UUI_COMPONENT_VALUETYPE_NUMBER,
    UUI_COMPONENT_VALUETYPE_FP_NUMBER,
    UUI_COMPONENT_VALUETYPE_IDENTIFIER,
    UUI_COMPONENT_VALUETYPE_COLOR,
} uui_component_valuetype_t;

typedef struct uui_component_value uui_component_value_t;
struct uui_component_value {
    uui_component_valuetype_t type;
    union {
        char *str;
        int64_t i64;
        double fp;
        bool_t boolean;
        uui_pixel_t color;
    } u;
};

struct uui_component {
    uui_component_allocate_view_t allocate_view;
};

typedef struct {
    uui_component_t component;

    Hashmap *default_properties;
    uui_component_t *parent_component;
    listnode_t children;

    // parse-time information
    listnode_t node;
    char *typestr;
} uui_component_parsed_t;

uui_comp_context_t* uui_comp_create_context(void);
int uui_comp_add_path(uui_comp_context_t *context, const char *path);
uui_component_t* uui_component_load(uui_comp_context_t *context, const char *id);
uui_component_t* uui_component_get(uui_comp_context_t *context, const char *id);
void uui_comp_free_context(uui_comp_context_t *context);

#endif
