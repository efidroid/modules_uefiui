#include <common.h>
#include <lib/strhashmap.h>

static bool_t str_eq(void *key_a, void *key_b)
{
    return !AsciiStrCmp((const char *)key_a, (const char *)key_b);
}

/* use djb hash unless we find it inadequate */
#ifdef __clang__
__attribute__((no_sanitize("integer")))
#endif
static int str_hash_fn(void *str)
{
    uint32_t hash = 5381;
    char *p;

    for (p = str; p && *p; p++)
        hash = ((hash << 5) + hash) + *p;
    return (int)hash;
}

Hashmap* strHashmapCreate(size_t initialCapacity) {
    return hashmapCreate(initialCapacity, str_hash_fn, str_eq);
}
