#ifndef HASHMAP_H
#define HASHMAP_H

#include <stddef.h>
#include <stdbool.h>

#define FREE_VAL(func, data) { \
    if (func != NULL) \
        func(data); \
}

typedef void (*destroy_t)(void* data);

typedef struct entry
{
    char* key;
    void* value;

    struct entry* next;     // 冲突链表
    size_t hash;
} *entry_t;

typedef struct
{
    size_t size;

    entry_t* list;          // 指针数组
    size_t length;

    // 由于不知道类型，当为 NULL 时内存释放操作交由上级处理
    destroy_t val_destroy_func;
} hashmap_t;

unsigned int DJBHash(const char* str, unsigned int length);

hashmap_t* hashmap_new(destroy_t val_destroy_func);
void hashmap_clear(hashmap_t* map);
void hashmap_free(hashmap_t* map);
void* hashmap_put(hashmap_t* map, char* key, void* value);
void* hashmap_get(hashmap_t* map, char* key);
void* hashmap_remove(hashmap_t* map, char* key);
bool hashmap_exists(hashmap_t* map, char* key);

#endif