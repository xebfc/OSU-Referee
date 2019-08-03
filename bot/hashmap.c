#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "hashmap.h"
#include "strb.h"

#define DEFAULT_INITIAL_CAPACITY 1 << 4 // 默认初始容量，2的4次方 = 16
#define MAXIMUM_CAPACITY 1 << 30 // 最大容量，2的30次方

#define HALF_INT_BIT (sizeof(int) / 2 * 8)
// 当length总是2的n次方时，h & (len - 1)运算等价于对length取模
#define INDEX_FOR(h, len) (h & (len - 1))
#define ENTRY_LIST_MALLOC(len) ( \
    (entry_t*)malloc((len) * sizeof(entry_t)) \
)
#define FREE_VAL(func, data) { \
    if (func != NULL) \
        func(data); \
}

// Times33
unsigned int DJBHash(const char* str, unsigned int length)
{
    unsigned int hash = 5381;
    unsigned int i = 0;

    for (i = 0; i < length; ++str, ++i)
    {
        hash = ((hash << 5) + hash) + (*str); // 33 = 31 + 1 = 2的5次方 + 1
    }

    return hash;
}

// JDK1.8
static size_t JDKHash(char* key)
{
    size_t h, length = strlen(key);
    return key == NULL ? 0 : (h = DJBHash(key, length)) ^ (h >> HALF_INT_BIT);
}

static void _resize()
{

}

static entry_t _get_entry(hashmap_t* map, char* key)
{
    if (map == NULL)
        goto end;

    size_t hash = JDKHash(key),
        i = INDEX_FOR(hash, map->length);

    entry_t e = *(map->list + i);
    while (e != NULL)
    {
        if (e->hash == hash && strcmp(e->key, key) == 0)
            return e;

        e = e->next;
    }

end:
    return NULL;
}

static void _add_entry(hashmap_t* map, char* key, void* value)
{
    if (map == NULL)
        return;

    size_t hash = JDKHash(key),
        i = INDEX_FOR(hash, map->length);

    entry_t e = (entry_t)malloc(sizeof(struct entry));
    if (e == NULL)
        return;

    e->hash = hash;
    e->key = (char*)malloc(STR_SIZEOF(strlen(key) + 1));
    e->value = value;

    *(map->list + i) = &e;

    if (++map->size > map->length)
        _resize();
}

//-------------------------------------------------------------------------------

hashmap_t* hashmap_new(destroy_t val_destroy_func)
{
    hashmap_t* map = (hashmap_t*)malloc(sizeof(hashmap_t));
    if (map == NULL)
        goto err;

    map->size = 0;
    map->list = ENTRY_LIST_MALLOC(DEFAULT_INITIAL_CAPACITY);
    if (map->list == NULL)
        goto err;
    map->length = DEFAULT_INITIAL_CAPACITY;
    map->val_destroy_func = val_destroy_func;

    return map;

err:
    hashmap_free(map);
    return NULL;
}

void hashmap_clear(hashmap_t* map)
{
    if (map == NULL)
        return;

    size_t i = map->length;
    while (i > 0)
    {
        entry_t e = *(map->list + --i);
        if (e == NULL)
            continue;
        *(map->list + i) = NULL;

        entry_t prev;
        while (e != NULL)
        {
            free(e->key);
            e->key = NULL;
            FREE_VAL(map->val_destroy_func, e->value);
            e->value = NULL;

            prev = e;
            e = e->next;
            prev->next = NULL;
            free(prev);
        }
    }
    map->size = 0;
}

void hashmap_free(hashmap_t* map)
{
    if (map == NULL)
        return;

    if (map->list != NULL)
    {
        hashmap_clear(map);
        free(map->list);
        map->list = NULL;
    }

    free(map);
}

void* hashmap_put(hashmap_t* map, char* key, void* value)
{
    if (map == NULL)
        goto end;

    entry_t e = _get_entry(map, key);
    if (e != NULL)
    {
        void* old = e->value;
        e->value = value;
        return old;
    }

    _add_entry(map, key, value);

end:
    return NULL;
}

void* hashmap_get(hashmap_t* map, char* key)
{
    entry_t e = _get_entry(map, key);
    return e != NULL ? e->value : NULL;
}

void* hashmap_remove(hashmap_t* map, char* key)
{
    if (map == NULL)
        goto end;

    size_t hash = JDKHash(key),
        i = INDEX_FOR(hash, map->length);

    entry_t e = *(map->list + i),
        prev = e;
    while (e != NULL)
    {
        if (e->hash == hash && strcmp(e->key, key) == 0)
        {
            map->size--;

            if (prev == e)
                (*(map->list + i) = e->next); // 外层括号用于避免将指针运算符*识别为乘法
            else
                prev->next = e->next;

            e->next = NULL;
            free(e->key);
            e->key = NULL;
            void* old = e->value;
            e->value = NULL;
            free(e);
            return old;
        }

        prev = e;
        e = e->next;
    }

end:
    return NULL;
}

bool hashmap_exists(hashmap_t* map, char* key)
{
    return _get_entry(map, key) != NULL;
}