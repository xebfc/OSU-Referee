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

static void _resize(hashmap_t* map)
{
    // 超过最大值就不再扩容了，随它碰撞去吧
    if (map->length >= MAXIMUM_CAPACITY)
        return;

    //size_t old_length = map->length;
    size_t new_length = map->length << 1; // 没超过最大值，就扩充为原来的2倍
    entry_t* new_list = ENTRY_LIST_MALLOC(new_length);
    if (new_list == NULL)
        return;

    size_t i = map->length;
    while (i-- > 0)
    {
        entry_t e = *(map->list + i);
        if (e == NULL)
            continue;
        *(map->list + i) = NULL;

        if (e->next == NULL)
        {
            (*(new_list + INDEX_FOR(e->hash, new_length)) = e);
            continue;
        }

        entry_t n,
            loHead = NULL, loTail = NULL,
            hiHead = NULL, hiTail = NULL;
        do
        {
            n = e->next;

            // 原索引
            if ((e->hash & map->length) == 0)
            {
                if (loTail == NULL)
                    loHead = e;
                else
                    loTail->next = e;
                loTail = e;
            }
            // 原索引 + old_length
            else
            {
                if (hiTail == NULL)
                    hiHead = e;
                else
                    hiTail->next = e;
                hiTail = e;
            }
        } while ((e = n) != NULL);

        if (loTail != NULL)
        {
            loTail->next = NULL;
            *(new_list + i) = loHead;
        }

        if (hiTail != NULL)
        {
            hiTail->next = NULL;
            *(new_list + i + map->length) = hiHead;
        }
    }

    map->length = new_length;
    free(map->list);
    map->list = new_list;
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
    e->key = STR_MALLOC(strlen(key) + 1);
    if (e->key == NULL)
    {
        free(e);
        return;
    }
    strcpy(e->key, key);
    e->value = value;

    // 为了避免循环，这里采用入栈的方式进行存储
    e->next = *(map->list + i);
    *(map->list + i) = e;

    if (++map->size > map->length)
        _resize(map);
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

    // 脏值处理
    //int i = DEFAULT_INITIAL_CAPACITY;
    //while (i-- > 0)
    //    *(map->list + i) = NULL;
    memset(map->list, NULL, DEFAULT_INITIAL_CAPACITY);

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
    while (i-- > 0)
    {
        entry_t e = *(map->list + i);
        if (e == NULL)
            continue;
        *(map->list + i) = NULL;

        entry_t prev;
        do
        {
            free(e->key);
            e->key = NULL;
            FREE_VAL(map->val_destroy_func, e->value);
            e->value = NULL;

            prev = e;
            e = e->next;
            prev->next = NULL;
            free(prev);
        } while (e != NULL);
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