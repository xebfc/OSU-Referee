#ifndef HASHMAP_H
#define HASHMAP_H

#include <stddef.h>
#include <stdbool.h>

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
} hashmap_t;

hashmap_t* hashmap_new();
void hashmap_clear(hashmap_t* map);
void hashmap_free(hashmap_t* map);
void* hashmap_put(hashmap_t* map, char* key, void* value); // 均以copy的方式存储
void* hashmap_get(hashmap_t* map, char* key);
void* hashmap_remove(hashmap_t* map, char* key); // 由于是copy返回的值别忘记free
bool hashmap_exists(hashmap_t* map, char* key);

#endif