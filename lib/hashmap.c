/*
 * Copyright (C) 2007 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <lib/hashmap.h>

typedef struct Entry Entry;
struct Entry {
    void* key;
    int hash;
    void* value;
    Entry* next;
};

struct Hashmap {
    Entry** buckets;
    size_t bucketCount;
    int (*hash)(void* key);
    bool_t (*equals)(void* keyA, void* keyB);
    size_t size;
};

Hashmap* hashmapCreate(size_t initialCapacity,
        int (*hash)(void* key), bool_t (*equals)(void* keyA, void* keyB)) {
    ASSERT(hash != NULL);
    ASSERT(equals != NULL);
    
    Hashmap* map = AllocatePool(sizeof(Hashmap));
    if (map == NULL) {
        return NULL;
    }
    
    // 0.75 load factor.
    size_t minimumBucketCount = initialCapacity * 4 / 3;
    map->bucketCount = 1;
    while (map->bucketCount <= minimumBucketCount) {
        // Bucket count must be power of 2.
        map->bucketCount <<= 1; 
    }

    map->buckets = AllocateZeroPool(map->bucketCount * sizeof(Entry*));
    if (map->buckets == NULL) {
        FreePool(map);
        return NULL;
    }
    
    map->size = 0;

    map->hash = hash;
    map->equals = equals;
    
    return map;
}

/**
 * Hashes the given key.
 */
#ifdef __clang__
__attribute__((no_sanitize("integer")))
#endif
static inline int hashKey(Hashmap* map, void* key) {
    int h = map->hash(key);

    // We apply this secondary hashing discovered by Doug Lea to defend
    // against bad hashes.
    h += ~(h << 9);
    h ^= (((unsigned int) h) >> 14);
    h += (h << 4);
    h ^= (((unsigned int) h) >> 10);
       
    return h;
}

size_t hashmapSize(Hashmap* map) {
    return map->size;
}

static inline size_t calculateIndex(size_t bucketCount, int hash) {
    return ((size_t) hash) & (bucketCount - 1);
}

static void expandIfNecessary(Hashmap* map) {
    // If the load factor exceeds 0.75...
    if (map->size > (map->bucketCount * 3 / 4)) {
        // Start off with a 0.33 load factor.
        size_t newBucketCount = map->bucketCount << 1;
        Entry** newBuckets = AllocateZeroPool(newBucketCount * sizeof(Entry*));
        if (newBuckets == NULL) {
            // Abort expansion.
            return;
        }
        
        // Move over existing entries.
        size_t i;
        for (i = 0; i < map->bucketCount; i++) {
            Entry* entry = map->buckets[i];
            while (entry != NULL) {
                Entry* next = entry->next;
                size_t index = calculateIndex(newBucketCount, entry->hash);
                entry->next = newBuckets[index];
                newBuckets[index] = entry;
                entry = next;
            }
        }

        // Copy over internals.
        if (map->buckets)
            FreePool(map->buckets);
        map->buckets = newBuckets;
        map->bucketCount = newBucketCount;
    }
}

void hashmapFree(Hashmap* map) {
    size_t i;
    for (i = 0; i < map->bucketCount; i++) {
        Entry* entry = map->buckets[i];
        while (entry != NULL) {
            Entry* next = entry->next;
            FreePool(entry);
            entry = next;
        }
    }
    if (map->buckets)
        FreePool(map->buckets);
    FreePool(map);
}

#ifdef __clang__
__attribute__((no_sanitize("integer")))
#endif
/* FIXME: relies on signed integer overflow, which is undefined behavior */
int hashmapHash(void* key, size_t keySize) {
    int h = keySize;
    char* data = (char*) key;
    size_t i;
    for (i = 0; i < keySize; i++) {
        h = h * 31 + *data;
        data++;
    }
    return h;
}

static Entry* createEntry(void* key, int hash, void* value) {
    Entry* entry = AllocatePool(sizeof(Entry));
    if (entry == NULL) {
        return NULL;
    }
    entry->key = key;
    entry->hash = hash;
    entry->value = value;
    entry->next = NULL;
    return entry;
}

static inline bool_t equalKeys(void* keyA, int hashA, void* keyB, int hashB,
        bool_t (*equals)(void*, void*)) {
    if (keyA == keyB) {
        return TRUE;
    }
    if (hashA != hashB) {
        return FALSE;
    }
    return equals(keyA, keyB);
}

void* hashmapPut(Hashmap* map, void* key, void* value) {
    int hash = hashKey(map, key);
    size_t index = calculateIndex(map->bucketCount, hash);

    Entry** p = &(map->buckets[index]);
    while (TRUE) {
        Entry* current = *p;

        // Add a new entry.
        if (current == NULL) {
            *p = createEntry(key, hash, value);
            if (*p == NULL) {
                return NULL;
            }
            map->size++;
            expandIfNecessary(map);
            return NULL;
        }

        // Replace existing entry.
        if (equalKeys(current->key, current->hash, key, hash, map->equals)) {
            void* oldValue = current->value;
            current->value = value;
            return oldValue;
        }

        // Move to next entry.
        p = &current->next;
    }
}

void* hashmapGet(Hashmap* map, void* key) {
    int hash = hashKey(map, key);
    size_t index = calculateIndex(map->bucketCount, hash);

    Entry* entry = map->buckets[index];
    while (entry != NULL) {
        if (equalKeys(entry->key, entry->hash, key, hash, map->equals)) {
            return entry->value;
        }
        entry = entry->next;
    }

    return NULL;
}

bool_t hashmapContainsKey(Hashmap* map, void* key) {
    int hash = hashKey(map, key);
    size_t index = calculateIndex(map->bucketCount, hash);

    Entry* entry = map->buckets[index];
    while (entry != NULL) {
        if (equalKeys(entry->key, entry->hash, key, hash, map->equals)) {
            return TRUE;
        }
        entry = entry->next;
    }

    return FALSE;
}

void* hashmapMemoize(Hashmap* map, void* key, 
        void* (*initialValue)(void* key, void* context), void* context) {
    int hash = hashKey(map, key);
    size_t index = calculateIndex(map->bucketCount, hash);

    Entry** p = &(map->buckets[index]);
    while (TRUE) {
        Entry* current = *p;

        // Add a new entry.
        if (current == NULL) {
            *p = createEntry(key, hash, NULL);
            if (*p == NULL) {
                return NULL;
            }
            void* value = initialValue(key, context);
            (*p)->value = value;
            map->size++;
            expandIfNecessary(map);
            return value;
        }

        // Return existing value.
        if (equalKeys(current->key, current->hash, key, hash, map->equals)) {
            return current->value;
        }

        // Move to next entry.
        p = &current->next;
    }
}

void* hashmapRemove(Hashmap* map, void* key) {
    int hash = hashKey(map, key);
    size_t index = calculateIndex(map->bucketCount, hash);

    // Pointer to the current entry.
    Entry** p = &(map->buckets[index]);
    Entry* current;
    while ((current = *p) != NULL) {
        if (equalKeys(current->key, current->hash, key, hash, map->equals)) {
            void* value = current->value;
            *p = current->next;
            FreePool(current);
            map->size--;
            return value;
        }

        p = &current->next;
    }

    return NULL;
}

void hashmapForEach(Hashmap* map, 
        bool_t (*callback)(void* key, void* value, void* context),
        void* context) {
    size_t i;
    for (i = 0; i < map->bucketCount; i++) {
        Entry* entry = map->buckets[i];
        while (entry != NULL) {
            Entry *next = entry->next;
            if (!callback(entry->key, entry->value, context)) {
                return;
            }
            entry = next;
        }
    }
}

size_t hashmapCurrentCapacity(Hashmap* map) {
    size_t bucketCount = map->bucketCount;
    return bucketCount * 3 / 4;
}

size_t hashmapCountCollisions(Hashmap* map) {
    size_t collisions = 0;
    size_t i;
    for (i = 0; i < map->bucketCount; i++) {
        Entry* entry = map->buckets[i];
        while (entry != NULL) {
            if (entry->next != NULL) {
                collisions++;
            }
            entry = entry->next;
        }
    }
    return collisions;
}

int hashmapIntHash(void* key) {
    // Return the key value itself.
    return *((int*) key);
}

bool_t hashmapIntEquals(void* keyA, void* keyB) {
    int a = *((int*) keyA);
    int b = *((int*) keyB);
    return a == b;
}
