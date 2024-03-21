#include<stdbool.h>
#include <clhash.h>
#include "bloom_filter.h"


typedef enum operation {
    Insert = 1,
    Delete = 2,
    Read = 3,
    Update = 4
} Operation;

typedef enum collisionhandling {
    Linear = 1,
    Double = 2,
    Quadratic = 3,
    Chaining = 4
} CollisionHandling;

typedef enum action_on_key_value{
    SaveToEvictionFile = 1,
    FreeMemory = 2,
    SaveToFileAndFreeMemory = 3
} Action_On_Key_Value;

typedef struct bucket
{
    void* key;
    void* value;
    union{
        bool occupied_status;
        struct bucket* nextNode;
    };
    int access_count;
}Bucket;


typedef struct pair{
    void* key;
    void* value_ptr;
} Pair;

typedef struct collision{
    unsigned int success_opr;
    unsigned int unsuccess_opr;
    unsigned int success_collision;
    unsigned int unsuccess_collision;
} Collision;

typedef struct continue_memory{
    size_t size;
    void* continue_value;
} Continue_Memroy;

typedef struct hashtable {
    Bucket** array_of_bucket;
    size_t key_size;
    unsigned int max_ht_size_limit;
    unsigned int no_element_ht;
    unsigned int current_hashtable_size;
    unsigned int no_evicted_element;
    unsigned int no_evicted_element_removed;
    unsigned int current_bloom_filter_size;
    char file_format;
    char* filepath;
    FILE* file_ptr;
    char* eviction_fileName;
    FILE* evicted_file_ptr;
    uint64_t (*ht_hashFunction)(void* key, size_t size);
    void (*handleKeyValuePtr)(void* key, void* value_ptr, Action_On_Key_Value action_on_key_value, struct hashtable* hashtable);
    Pair* (*generateKeyValuePtr)(size_t key_size, FILE* file_ptr);
    bloom_filter* bloom_flt;
    CollisionHandling collision_handling;

    Collision insert, deleted, read, update;
    unsigned int other_collision;
    unsigned int  no_of_collision;
} HashTable;

HashTable* generateHashTableWithFile(char* file, Pair* (*generateKeyValuePtrFromFile)(size_t key_size, FILE* file_ptr), char file_format, unsigned int initial_size, unsigned int max_ht_size_limit, void (*handleKeyValuePtr) (void* key, void* value_ptr, Action_On_Key_Value action_on_key_value, HashTable* hashtable), Pair* (*generateKeyValuePtr)(size_t key_size, FILE* file_ptr), char* newFileName, uint64_t (*hashFunction) (void* value, size_t size), CollisionHandling collision_handling, char new_format);

HashTable* setupHashTable(unsigned int initial_size, unsigned int max_ht_size_limit, size_t key_size, void (*handleKeyValuePtr) (void* key, void* value_ptr, Action_On_Key_Value action_on_key_value, HashTable* hashtable), Pair* generateKeyAndValuePtr(size_t key_size, FILE* file_ptr), char* filepath, uint64_t (*hashFunction) (void* value, size_t size), CollisionHandling collision_handling, char file_format);

void* operationOnHashTable(void* key, void* value, HashTable* hashtable, Operation operation);

void closeHashTable(int hashtableN, ...);

void iterateHashTable(HashTable* hashtable, unsigned int iterator, void (*keyFunction)(void* key_ptr), void (*valueFunction)(void* value_ptr));
