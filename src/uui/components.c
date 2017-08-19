#include <common.h>
#include <uui/components.h>
#include <uui/views/rect.h>
#include <uui/layouts/linear.h>
#include <lib/strhashmap.h>

typedef struct {
    listnode_t node;
    char *path;
} path_item_t;

uui_component_parsed_t * uui_comp_internal_parse_component(uui_comp_context_t *context, const char *filename, Hashmap **pimports);

static const char *components_fext = ".uui";

static uui_view_t* uui_component_view_allocate_view(uui_component_t *component) {
    uui_view_t *view = uui_view_create();
    return view;
}

static uui_view_t* uui_component_rect_allocate_view(uui_component_t *component) {
    uui_view_rect_t *rect = uui_view_rect_create();
    return &rect->view;
}

static uui_view_t* uui_component_linearlayout_allocate_view(uui_component_t *component) {
    uui_layout_linear_t *linear = uui_layout_linear_create();
    return &linear->viewgroup.view;
}

uui_comp_context_t* uui_comp_create_context(void) {
    uui_comp_context_t *context = AllocateZeroPool(sizeof(uui_comp_context_t));
    if (!context) return NULL;

    list_initialize(&context->paths);
    context->components = strHashmapCreate(5);
    ASSERT(context->components);
    context->urc = urc_context_create();
    ASSERT(context->urc);

    uui_component_t *comp = AllocateZeroPool(sizeof(uui_component_t));
    ASSERT(comp);
    comp->allocate_view = uui_component_view_allocate_view;
    hashmapPut(context->components, AsciiStrDup("views.View"), comp);

    comp = AllocateZeroPool(sizeof(uui_component_t));
    ASSERT(comp);
    comp->allocate_view = uui_component_rect_allocate_view;
    hashmapPut(context->components, AsciiStrDup("views.Rect"), comp);

    comp = AllocateZeroPool(sizeof(uui_component_t));
    ASSERT(comp);
    comp->allocate_view = uui_component_linearlayout_allocate_view;
    hashmapPut(context->components, AsciiStrDup("layouts.LinearLayout"), comp);

    return context;
}

int uui_comp_add_path(uui_comp_context_t *context, const char *path) {
    path_item_t *item = AllocatePool(sizeof(path_item_t));
    if (!item)
        return -1;

    item->path = AsciiStrDup(path);
    list_add_tail(&context->paths, &item->node);

    return 0;
}

static int component_name_to_filename(const char *components_dir, char *pathbuf, const char *id) {
    size_t i;
    size_t components_dir_len = AsciiStrLen(components_dir);
    size_t id_len = AsciiStrLen(id);
    size_t fext_len = AsciiStrLen(components_fext);

    if (components_dir_len + 1 + id_len + fext_len + 1 >= 4096)
        return -1;

    AsciiStrCpyS(pathbuf, 4096, components_dir);
    AsciiStrCatS(pathbuf, 4096, "/");
    for (i=0; i<id_len; i++) {
        char c = id[i];

        if (!( (c>='a' && c<='z') || (c>='A' && c<='Z') || (c>='0' && c<='9')  || c=='.' )) {
            return -1;
        }

        if (c=='.')
            c = '/';

        pathbuf[components_dir_len + 1 + i] = c;
    }
    pathbuf[components_dir_len + 1 + id_len] = '\0';

    AsciiStrCatS(pathbuf, 4096, components_fext);

    return 0;
}

static bool_t tryload_import(void *_key, void *_value, void *_context) {
    uui_comp_context_t *context = _context;
    //const char *alias = _key;
    const char *importpath = _value;

    if (!uui_component_get(context, importpath)) {
        uui_component_t *comp = uui_component_load(context, importpath);
        if (!comp) {
            DEBUG((DEBUG_ERROR, "import %"FMTS" not found\n", importpath));
            ASSERT(0);
            return FALSE;
        }
    }

    return TRUE;
}

static bool_t comp_setprop(void *_key, void *_value, void *_context) {
    uui_view_t *view = _context;
    const char *name = _key;
    uui_component_value_t *value = _value;

    int rc = view->comp_set_property(view, name, value);
    if(rc) {
        if (!AsciiStrnCmp(name, "layout_", 7)) {
            hashmapPut(view->comp_layout_properties, AsciiStrDup(name), value);
        }
        else {
            DEBUG((DEBUG_ERROR, "can't set prop %"FMTS"\n", name));
        }
    }

    return TRUE;
}

static uui_view_t* uui_component_parsed_allocate_view(uui_component_t *_component) {
    uui_component_parsed_t* component = containerof(_component, uui_component_parsed_t, component);

    ASSERT(component->parent_component);
    ASSERT(component->parent_component->allocate_view);
    uui_view_t *view = component->parent_component->allocate_view(component->parent_component);
    if (!view) return NULL;

    if (view->comp_set_property) {
        hashmapForEach(component->default_properties, comp_setprop, view);
    }

    if (view->comp_add_child_view) {
        uui_component_parsed_t *child;
        list_for_every_entry(&component->children, child, uui_component_parsed_t, node) {
            uui_view_t *childview = child->component.allocate_view(&child->component);
            ASSERT(childview);
            view->comp_add_child_view(view, childview);
        }

    }

    return view;
}

static void fill_parent_components(uui_comp_context_t *context, Hashmap *imports, uui_component_parsed_t *comp) {
    comp->component.allocate_view = uui_component_parsed_allocate_view;
    const char* parentid = hashmapGet(imports, comp->typestr);
    if (!parentid) {
        DEBUG((DEBUG_ERROR, "Unknown type %"FMTS"\n", comp->typestr));
        ASSERT(0);
    }
    uui_component_t *parent = uui_component_get(context, parentid);
    if (!parent) {
        DEBUG((DEBUG_ERROR, "Unknown type %"FMTS"\n", comp->typestr));
        ASSERT(0);
    }
    comp->parent_component = parent;

    uui_component_parsed_t *child;
    list_for_every_entry(&comp->children, child, uui_component_parsed_t, node) {
        fill_parent_components(context, imports, child);
    }
}

uui_component_t* uui_component_load(uui_comp_context_t *context, const char *id) {
    int rc;
    char pathbuf[4096];
    uui_component_parsed_t *parsed_component;
    Hashmap *imports = NULL;

    path_item_t *item;
    list_for_every_entry(&context->paths, item, path_item_t, node) {
        // get filename
        rc = component_name_to_filename(item->path, pathbuf, id);
        if (rc) continue;

        parsed_component = uui_comp_internal_parse_component(context, pathbuf, &imports);
        if(!parsed_component) return NULL;

        // register component in context
        hashmapPut(context->components, AsciiStrDup(id), &parsed_component->component);

        // load missing imports
        hashmapForEach(imports, tryload_import, context);

        // fill 'parent_component' by resolving typestr from imports and context
        fill_parent_components(context, imports, parsed_component);

        return &parsed_component->component;
    }

    return NULL;
}

uui_component_t* uui_component_get(uui_comp_context_t *context, const char *id) {
    return hashmapGet(context->components, (void*)id);
}

static bool_t remove_component(void *key, void *value, void *_context) {
    uui_comp_context_t *context = _context;
    hashmapRemove(context->components, key);
    FreePool(key);
    FreePool(value);

    return TRUE;
}

void uui_comp_free_context(uui_comp_context_t *context) {
    while (!list_is_empty(&context->paths)) {
        path_item_t *item = list_remove_tail_type(&context->paths, path_item_t, node);

        FreePool(item->path);
        FreePool(item);
    }

    hashmapForEach(context->components, remove_component, context);
    hashmapFree(context->components);
}
