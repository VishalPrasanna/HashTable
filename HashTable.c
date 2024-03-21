#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <stdarg.h>

#include "HashTable.h"

#define FREE_AFTER_DELETE 1
#define DONT_FREE_AFTER_DELETE 0
#define CURRENT_OCCUPIED 0
#define PREVIOUS_OCCUPIED 1
#define DEFAULT_HT_SIZE 11
#define MAX_HT_SIZE_LIMIT 10000
#define DEFAULT_BLOOM_FLT_SIZE 100000


#define AVAILABLE_TO_ACCESS '1'
#define NOT_AVAILABLE_TO_ACCESS '0'



void resizeOrEvict(HashTable* hashtable);

int squareOf(int num){
    return num * num;
}


unsigned long int generateClhashWithSeed(void* key, size_t size, int seed){
    void * random =  get_random_key_for_clhash(UINT64_C(0x23a23cf5033c3c81) * (uint64_t) seed,UINT64_C(0xb3816f6a2c68e530) * (uint64_t)seed);
    uint64_t hashvalue = clhash(random, (char*)key, size);
    free(random);
    return hashvalue;
}

uint64_t* multiHashing(int num_hashes, void* str, size_t str_len){

    uint64_t * results = (uint64_t*)calloc(sizeof(uint64_t), num_hashes); // using calloc because all bits should be set to null / 0 
    results[0] = generateClhashWithSeed(str, str_len, 1);
    for(int i = 1; i < num_hashes; ++i){
        results[i] = generateClhashWithSeed(str, str_len, i+1); 
    }
    
    return results; 
}

void handleKeyValueWithAction(Action_On_Key_Value action_on_key_value, void* key, void* value_ptr, HashTable* hashtable){
    if(value_ptr == NULL){
        return;
    }
    else if(action_on_key_value == SaveToEvictionFile){
        fseek(hashtable->evicted_file_ptr, 0, SEEK_END);
        char available_status = AVAILABLE_TO_ACCESS;
        fwrite(&available_status, sizeof(char), 1, hashtable->evicted_file_ptr);
        fwrite(key, hashtable->key_size, 1, hashtable->evicted_file_ptr);
        hashtable->handleKeyValuePtr(key, value_ptr, action_on_key_value, hashtable); 
        add_value_bloom_filter(hashtable->bloom_flt, key, hashtable->key_size);
    }
    else if(action_on_key_value == SaveToFileAndFreeMemory){
        hashtable->handleKeyValuePtr(key, value_ptr, action_on_key_value, hashtable);  
    }
    else if(action_on_key_value == FreeMemory){
        hashtable->handleKeyValuePtr(key, value_ptr, action_on_key_value, NULL);
    }
}


HashTable* setupHashTable(unsigned int initial_size, unsigned int max_ht_size_limit, size_t key_size, void (*handleKeyValuePtr) (void* key, void* value_ptr, Action_On_Key_Value action_on_key_value, HashTable* hashtable), Pair* generateKeyValuePtr(size_t key_size, FILE* file_ptr), char* filepath, uint64_t (*hashFunction) (void* value, size_t size), CollisionHandling collision_handling, char file_format){

    HashTable* hashtable = (HashTable*)calloc(1, sizeof(HashTable));

    unsigned int adding_ht_size = (initial_size == 0)? DEFAULT_HT_SIZE : initial_size;
    hashtable->max_ht_size_limit = (max_ht_size_limit == 0)? MAX_HT_SIZE_LIMIT : max_ht_size_limit;
    hashtable->key_size = key_size;

    hashtable->handleKeyValuePtr = handleKeyValuePtr;

    hashtable->array_of_bucket =  (Bucket**)calloc(adding_ht_size, sizeof(void*));
    hashtable->current_hashtable_size = 0;
    hashtable->current_hashtable_size += adding_ht_size;

    hashtable->no_element_ht = 0;
    hashtable->no_evicted_element = 0;
    hashtable->no_evicted_element_removed = 0;
    hashtable->current_bloom_filter_size = DEFAULT_BLOOM_FLT_SIZE;

    hashtable->collision_handling = collision_handling;
    hashtable->ht_hashFunction = hashFunction;
    hashtable->generateKeyValuePtr = generateKeyValuePtr;

    hashtable->bloom_flt = (bloom_filter*)calloc(1, sizeof(bloom_filter));
    bloom_filter_init(hashtable->bloom_flt, hashtable->current_bloom_filter_size, 0.05, multiHashing);
    

    hashtable->file_format = file_format;

    hashtable->filepath = (char*)calloc(1, strlen(filepath)+ 5);
    strcpy(hashtable->filepath, filepath);

    char* local_fileName = (char*)calloc(100, strlen(filepath) + 10);
    strcat(local_fileName, filepath);

    if(file_format == 'T'){
        strcat(local_fileName, ".txt");
    }
    else if(file_format == 'B'){
        strcat(local_fileName, ".bin");
    }

    hashtable->file_ptr = fopen(local_fileName, "wb+");
    free(local_fileName);

    hashtable->eviction_fileName = (char*)calloc(1, strlen(filepath) + 10);
    strcat(hashtable->eviction_fileName, hashtable->filepath);
    strcat(hashtable->eviction_fileName, "temp.bin");

    hashtable->evicted_file_ptr = fopen(hashtable->eviction_fileName, "wb+");

    unsigned int total_element = hashtable->no_element_ht + hashtable->no_evicted_element - hashtable->no_evicted_element_removed;
    
    if(hashtable->file_format == 'T'){
        fprintf(hashtable->file_ptr, "%20zu ", hashtable->key_size);
        fprintf(hashtable->file_ptr, "%11u ", total_element);  
    }
    else if(hashtable->file_format == 'B'){
        fwrite(&hashtable->key_size, sizeof(size_t), 1, hashtable->file_ptr);
        fwrite(&total_element, sizeof(unsigned int), 1, hashtable->file_ptr);  
    }

    // printf("Setup Successfulle\n");
    return hashtable;
}

void closeHashTable(int hashtableN, ...){

    va_list hashtable_arr;

    va_start(hashtable_arr, hashtableN);
    
    unsigned int total_element = 0;

    while(hashtableN--){
        HashTable* hashtable = (HashTable*)va_arg(hashtable_arr, HashTable*);
        unsigned int ht_size = hashtable->current_hashtable_size;
        for(unsigned int i = 0; i < ht_size; i++){
            Bucket* bucket_ptr = hashtable->array_of_bucket[i];
            if(hashtable->collision_handling != Chaining && bucket_ptr != NULL){
                if(bucket_ptr->occupied_status == CURRENT_OCCUPIED && bucket_ptr->value != NULL){
                    handleKeyValueWithAction(SaveToFileAndFreeMemory, bucket_ptr->key, bucket_ptr->value, hashtable);
                    total_element += 1;
                }
                free(bucket_ptr->key);
                free(bucket_ptr);
            }
            else if(hashtable->collision_handling == Chaining){
                while (bucket_ptr != NULL)
                {
                    Bucket* temp = bucket_ptr;
                    bucket_ptr = bucket_ptr->nextNode;

                    if(temp->value != NULL){
                        handleKeyValueWithAction(SaveToFileAndFreeMemory, temp->key, temp->value, hashtable);
                        total_element += 1;
                    }
                    free(temp->key);
                    free(temp);
                }
            }
        }

        FILE* evicted_file_ptr =  hashtable->evicted_file_ptr;
        fflush(evicted_file_ptr);
        fseek(evicted_file_ptr, 0, SEEK_SET);
        while(1){
            char available_status = fgetc(evicted_file_ptr);
            if(available_status == EOF || (available_status != AVAILABLE_TO_ACCESS && available_status != NOT_AVAILABLE_TO_ACCESS)){
                break;
            }
            void* key_in_file = calloc(1, hashtable->key_size);
            fread(key_in_file , hashtable->key_size, 1, evicted_file_ptr);

            int keye = *(int*)key_in_file;
            size_t total_store_size = 0;
            if(hashtable->file_format == 'T'){
                fscanf(evicted_file_ptr, "%zu ", &total_store_size);
            }
            else if(hashtable->file_format == 'B'){
                fread(&total_store_size, sizeof(size_t), 1, evicted_file_ptr);
            }
            if(available_status == AVAILABLE_TO_ACCESS){
                char* tranporting_buf;
                if(hashtable->file_format == 'T'){
                    fprintf(hashtable->file_ptr, "%20zu ", total_store_size);
                    tranporting_buf = (char*)calloc(1, total_store_size + 1);
                    
                    fgets(tranporting_buf, total_store_size + 1, evicted_file_ptr);
                }
                else if(hashtable->file_format == 'B'){    
                    fwrite(&total_store_size, sizeof(size_t), 1, hashtable->file_ptr);
                    tranporting_buf = (char*)calloc(1, total_store_size);
                    fread(tranporting_buf, total_store_size, 1, evicted_file_ptr);
                }
                fwrite(tranporting_buf, total_store_size, 1, hashtable->file_ptr);
                total_element += 1;
                free(tranporting_buf);
            }
            else{
                fseek(evicted_file_ptr, total_store_size, SEEK_CUR);
            }
            free(key_in_file);
        }


        fseek(hashtable->file_ptr, 0, SEEK_SET);

        if(hashtable->file_format == 'T'){
            fprintf(hashtable->file_ptr, "%20zu ", hashtable->key_size);
            fprintf(hashtable->file_ptr, "%11u ", total_element);  
        }
        else if(hashtable->file_format == 'B'){
            fwrite(&hashtable->key_size, sizeof(size_t), 1, hashtable->file_ptr);
            fwrite(&total_element, sizeof(unsigned int), 1, hashtable->file_ptr);  
        }


        free_bloom_filter(hashtable->bloom_flt);
        free(hashtable->bloom_flt);
        free(hashtable->array_of_bucket);
        free(hashtable->filepath);
        fclose(hashtable->file_ptr);
        fclose(hashtable->evicted_file_ptr);
        remove(hashtable->eviction_fileName);
        free(hashtable->eviction_fileName);
        free(hashtable);
        hashtable = NULL;
    }
    va_end(hashtable_arr);
}


uint64_t defaultHashFunction(void* value, size_t size){
    void * random =  get_random_key_for_clhash(UINT64_C(0x23a23cf5033c3c81),UINT64_C(0xb3816f6a2c68e530));
    uint64_t hashvalue = clhash(random, (char*)value, size);
    free(random);
    return hashvalue;
}

unsigned int primeNumber(unsigned int num, bool nextPrime){
    unsigned int  prime = 2;
    bool isPrimeFinded = false;
    while(!isPrimeFinded){
        if(nextPrime){
            num += 1;
        }
        else{
            num -= 1;
        }

        bool isprime = true;
        for(unsigned int j = 2; j < (num / 2); j++){
            if(num % j == 0){
                isprime = false;
                break;
            }
        }
        if(isprime){
            isPrimeFinded = true;
        }
    }
    return num;
}

uint64_t secondHashFunction(HashTable* hashtable, void* key){
    uint64_t prime = 1;
    prime = primeNumber(hashtable->current_hashtable_size, 0);
    return prime - (defaultHashFunction(key, hashtable->key_size) % (prime));
}

unsigned int generateHashKey(HashTable* hashtable, void* key){

    uint64_t hash_value = 0;
    if(hashtable->ht_hashFunction != NULL){
        hash_value = hashtable->ht_hashFunction(key, hashtable->key_size);
    }
    else{
        hash_value = defaultHashFunction(key, hashtable->key_size);
    }
    return hash_value % (unsigned long int)hashtable->current_hashtable_size;
}

void* storeInHeapMemory(void* value, size_t size){

    void* address = malloc(size);
    memcpy(address, value, size);
    return address;
}

void* finding(HashTable* hashtable, void* key, unsigned int hash_key){
    Bucket** array_of_bucket = (Bucket**)hashtable->array_of_bucket;

    if(hashtable->collision_handling == Chaining){
        Bucket* index_bucket = array_of_bucket[hash_key];
        if(index_bucket == NULL){
            return NULL;
        }

        while(index_bucket != NULL){            
            hashtable->no_of_collision += 1;
            if(memcmp(index_bucket->key, key, hashtable->key_size) == 0){
                index_bucket->access_count += 1;
                return index_bucket;
            }

            index_bucket = index_bucket->nextNode;
        }
    }
    else if(hashtable->collision_handling != Chaining){
        unsigned int iterate_hash_key = hash_key;
        unsigned int collision_occur = 0;

        unsigned int second_hash_key = 0;
        if(hashtable->collision_handling == Double){
            second_hash_key = secondHashFunction(hashtable, key);
        }

        while(array_of_bucket[iterate_hash_key] != NULL){
            Bucket* indexed_bucket = array_of_bucket[iterate_hash_key];
            if(indexed_bucket->occupied_status == CURRENT_OCCUPIED && memcmp(indexed_bucket->key, key, hashtable->key_size) == 0){
                indexed_bucket->access_count += 1;
                return indexed_bucket;
            }
            hashtable->no_of_collision += 1;
            collision_occur += 1;
            unsigned int hash_key_inc = (hashtable->collision_handling == Linear)? collision_occur : 
                                        (hashtable->collision_handling == Quadratic)? squareOf(collision_occur) :
                                        collision_occur * second_hash_key;
            iterate_hash_key =  (hash_key + hash_key_inc) % hashtable->current_hashtable_size;
            if(hashtable->collision_handling == Linear  && iterate_hash_key == hash_key  ||
                hashtable->collision_handling == Quadratic && collision_occur > hashtable->current_hashtable_size ||
                hashtable->collision_handling == Double && collision_occur > hashtable->collision_handling){

                // printf("Key is not Present\n");
                return NULL;
            }
        }
    }
    return NULL;
}


void* insertion(HashTable* hashtable, Bucket* bucket, unsigned int hash_key){
    Bucket** array_of_bucket = (Bucket**)hashtable->array_of_bucket;

    if(hashtable->collision_handling == Chaining){
        
        Bucket* insert_bucket = (Bucket*)calloc(1, sizeof(Bucket));
        insert_bucket->key = bucket->key;
        insert_bucket->value = bucket->value;
        insert_bucket->nextNode = NULL;
        insert_bucket->access_count = bucket->access_count;

        Bucket* indexed_bucket = array_of_bucket[hash_key];

        if(indexed_bucket == NULL){
            array_of_bucket[hash_key] = insert_bucket;
            hashtable->no_element_ht += 1;
            return array_of_bucket[hash_key];
        }
        else if(memcmp(indexed_bucket->key, bucket->key, hashtable->key_size) == 0){
            // printf("Key is Already Present\n");
            free(insert_bucket);
            return NULL;
        }
        while(indexed_bucket->nextNode != NULL){
            hashtable->no_of_collision += 1;
            Bucket* temp = indexed_bucket->nextNode;
            if(memcmp(temp->key, bucket->key, hashtable->key_size) == 0){
                // printf("Key is Already Present\n");
                free(insert_bucket);
                return NULL;
            }
            indexed_bucket = indexed_bucket->nextNode;
        }
        hashtable->no_element_ht += 1;
        indexed_bucket->nextNode = insert_bucket;
        return indexed_bucket->nextNode;
    }
    else if(hashtable->collision_handling != Chaining){

        unsigned int iterated_index = hash_key;
        unsigned int collision_occur = 0;
        
        unsigned int second_hash_key = 0;
        if(hashtable->collision_handling == Double){
            second_hash_key = secondHashFunction(hashtable, bucket->key);
        }
        bool empty_space = false;
        unsigned int empty_bucket_index = 0;

        while(array_of_bucket[iterated_index] != NULL){
            // printf("Collision\n");
            if((array_of_bucket[iterated_index]->occupied_status == CURRENT_OCCUPIED) && (memcmp(array_of_bucket[iterated_index]->key, bucket->key, hashtable->key_size) == 0)){
                return NULL;
            }
            else if(empty_space == false && array_of_bucket[iterated_index]->occupied_status == PREVIOUS_OCCUPIED){
                empty_bucket_index = iterated_index;
                empty_space = true;
            }

            hashtable->no_of_collision += 1;
            collision_occur += 1;
            unsigned int inc_hash_key = (hashtable->collision_handling == Linear) ? collision_occur : 
                                        (hashtable->collision_handling == Quadratic)? squareOf(collision_occur) : 
                                        collision_occur * second_hash_key;
            iterated_index =  (hash_key + inc_hash_key) % hashtable->current_hashtable_size;
            if(hashtable->collision_handling == Linear && iterated_index == hash_key ||
                hashtable->collision_handling == Quadratic && collision_occur > hashtable->current_hashtable_size ||
                hashtable->collision_handling == Double && collision_occur > hashtable->current_hashtable_size){
                    if(empty_space == true){
                        // Space is avaliable to store
                        break;
                    }
                    else{
                        printf("Insertion fail try different Key or Incease the table size\n");
                        return NULL;
                    }
            }
        }

        hashtable->no_element_ht += 1;
        if(empty_space == true){
            array_of_bucket[empty_bucket_index]->key = bucket->key;
            array_of_bucket[empty_bucket_index]->value = bucket->value;
            array_of_bucket[empty_bucket_index]->occupied_status = CURRENT_OCCUPIED;
            array_of_bucket[empty_bucket_index]->access_count = bucket->access_count;
            return array_of_bucket[empty_bucket_index];
        }
        else{
            Bucket* new_bucket = (Bucket*)calloc(1, sizeof(Bucket));
            new_bucket->key = bucket->key;
            new_bucket->value = bucket->value;
            new_bucket->occupied_status = CURRENT_OCCUPIED;
            new_bucket->access_count = bucket->access_count;
            array_of_bucket[iterated_index] = new_bucket;
            return array_of_bucket[iterated_index];
        }   
    }
    return NULL;
}




bool deletion(HashTable* hashtable, void* key, unsigned int hash_key, bool memory_free){

    if(hashtable->collision_handling != Chaining){
        Bucket* return_bucket = (Bucket*)finding(hashtable, key, hash_key);
        if(return_bucket == NULL){
            // printf("The Key is not Present\n");
            return false;
        }
        if(memory_free == FREE_AFTER_DELETE){
            free(return_bucket->key);
            handleKeyValueWithAction(FreeMemory, NULL, return_bucket->value, hashtable);
        }
        return_bucket->key = NULL;
        return_bucket->value = NULL;
        return_bucket->occupied_status = PREVIOUS_OCCUPIED;
        return_bucket->access_count = 0;
        hashtable->no_element_ht -= 1;
        return true;
    }
    else if(hashtable->collision_handling == Chaining){
        Bucket* indexed_bucket = hashtable->array_of_bucket[hash_key];

        if(indexed_bucket != NULL && memcmp(indexed_bucket->key, key, hashtable->key_size) == 0){
            hashtable->array_of_bucket[hash_key] = indexed_bucket->nextNode;
            if(memory_free == FREE_AFTER_DELETE){
                free(indexed_bucket->key);
                handleKeyValueWithAction(FreeMemory, NULL, indexed_bucket->value, hashtable);
                free(indexed_bucket);
            }
            hashtable->no_element_ht -= 1;
            // printf("Deleted Successfully\n");
            return true;
        }

        while(indexed_bucket != NULL){

            hashtable->no_of_collision += 1;

            Bucket* next_bucket_node = indexed_bucket->nextNode;
            if(next_bucket_node != NULL && memcmp(next_bucket_node->key, key, hashtable->key_size) == 0){
                indexed_bucket->nextNode = next_bucket_node->nextNode;
                if(memory_free == FREE_AFTER_DELETE){
                    free(next_bucket_node->key);
                    handleKeyValueWithAction(FreeMemory, NULL, indexed_bucket->value, hashtable);
                    free(next_bucket_node);
                }
                hashtable->no_element_ht -= 1;
                // printf("Deleted Successfully\n");
                return true;
            }
            indexed_bucket = indexed_bucket->nextNode;
        }
    }
    return false;
}


void* update(HashTable* hashtable, Bucket* bucket, unsigned int hash_key){
    Bucket* existing_bucket = (Bucket*)finding(hashtable, bucket->key, hash_key);
    if(existing_bucket == NULL){
        // printf("The key is not found cannot update\n");
        return NULL;
    }
    if(bucket->value != NULL){
        handleKeyValueWithAction(FreeMemory, NULL, existing_bucket->value, hashtable);
        existing_bucket->access_count += 1;
        existing_bucket->value = bucket->value;        
    } 
    return existing_bucket;
}


typedef struct teacher{
    unsigned int teacher_id;
    char* name;
    unsigned short int age;
    float attendance;
    unsigned int phone_no;
    float salary;
    unsigned long int account_no;
    char address[21];
    struct Student{
        unsigned int student_id;
        char* name;
        unsigned short int age;
        float attendance;
        unsigned int phone_no;
        float cgpa;
        unsigned int register_no;
        char* address;
    } s;
}Teacher;

static void displayValuePtr(void* value_ptr){
    Teacher* teacher_value_ptr = (Teacher*)value_ptr;
    printf("%u %s %hu %f %u %f %lu %s %u %s %hu %f %u %f %u %s....", teacher_value_ptr->teacher_id, teacher_value_ptr->name, teacher_value_ptr->age, teacher_value_ptr->attendance,
                    teacher_value_ptr->phone_no, teacher_value_ptr->salary, teacher_value_ptr->account_no, teacher_value_ptr->address ,
                    teacher_value_ptr->s.student_id, teacher_value_ptr->s.name, teacher_value_ptr->s.age, teacher_value_ptr->s.attendance, teacher_value_ptr->s.phone_no, teacher_value_ptr->s.cgpa, teacher_value_ptr->s.register_no, teacher_value_ptr->s.address);
}


bool operationOnEvictedFile(HashTable* hashtable, void* key, Operation operation){

    if(hashtable->no_evicted_element - hashtable->no_evicted_element_removed == 0){
        return 0;
    }
    FILE* evicted_file =  hashtable->evicted_file_ptr;
    fseek(evicted_file, 0, SEEK_SET);
    while(1){
        char available_status = fgetc(evicted_file);
        if(available_status == EOF){
            break;
        }

        void* key_in_file = calloc(1, hashtable->key_size);
        fread(key_in_file , hashtable->key_size, 1, evicted_file);

        if(available_status == AVAILABLE_TO_ACCESS && memcmp(key, key_in_file, hashtable->key_size) == 0){
            free(key_in_file);
            fseek(evicted_file, -(hashtable->key_size+1), SEEK_CUR);
            char temp = NOT_AVAILABLE_TO_ACCESS;
            if(operation == Delete){
                hashtable->no_evicted_element_removed += 1;
                fwrite(&temp, sizeof(char), 1, evicted_file);
                return 1;
            }
            else if(operation != Delete){
                hashtable->no_evicted_element_removed += 1;
                fwrite(&temp, sizeof(char), 1, evicted_file);
                fseek(evicted_file, hashtable->key_size, SEEK_CUR);
                Pair* pair_ptr = hashtable->generateKeyValuePtr(hashtable->key_size, evicted_file);
                operationOnHashTable(pair_ptr->key, pair_ptr->value_ptr, hashtable, Insert);
                free(pair_ptr->key);
                free(pair_ptr);
                return 1;        
            }
            break;
        }
        hashtable->other_collision += 1;

        free(key_in_file);
        size_t total_store_size;

        if(hashtable->file_format == 'T'){
            fscanf(evicted_file, "%zu ", &total_store_size);
        }
        else if(hashtable->file_format == 'B'){
            fread(&total_store_size, sizeof(size_t), 1, evicted_file);
        }
        fseek(evicted_file, total_store_size, SEEK_CUR);
    }
    // printf("hello");
    return 0;
}


bool searchKeyInEvictedFile(HashTable* hashtable, void* key){
    if(is_present_bloom_filter(hashtable->bloom_flt, key, hashtable->key_size) == BLOOM_SUCCESS){
        return true;
    }
    else{
        return false;
    }
}


void* insertOnHashTable(HashTable* hashtable, void* key, void* value, unsigned int hash_key){

    resizeOrEvict(hashtable);

    if(searchKeyInEvictedFile(hashtable, key) && operationOnEvictedFile(hashtable, key, Insert)){
        // printf("The insertion is unsuccessfull key found At Eviction File\n");
        handleKeyValueWithAction(FreeMemory, NULL, value, hashtable);
        hashtable->insert.unsuccess_opr += 1;
        hashtable->insert.unsuccess_collision += hashtable->no_of_collision;
        return NULL;
    }


    void* value_ptr = NULL;
    Bucket* bucket_ptr = NULL;

    void* key_cpy = storeInHeapMemory(key, hashtable->key_size);

    Bucket* bucket = (Bucket*)calloc(1, sizeof(Bucket));
    bucket->key = key_cpy;
    bucket->value = value;
    bucket->access_count = 0;
    
    bucket_ptr = (Bucket*)insertion(hashtable, bucket, hash_key);

    if(bucket_ptr == NULL){
        free(key_cpy);
        handleKeyValueWithAction(FreeMemory, NULL, value, hashtable);
        // printf("The insertion is unsuccessfull\n");
        hashtable->insert.unsuccess_opr += 1;
        hashtable->insert.unsuccess_collision += hashtable->no_of_collision;
    }
    else if(bucket_ptr != NULL){
        value_ptr = &bucket_ptr->value;
        // printf("The insertion is successfull\n");
        hashtable->insert.success_opr += 1;
        hashtable->insert.success_collision += hashtable->no_of_collision;
    }
    free(bucket);
    return value_ptr;
}


void deleteOnHashTable(HashTable* hashtable, void* key, unsigned int hash_key){

    bool result = deletion(hashtable, key, hash_key, FREE_AFTER_DELETE);

    if(result == true){
        // printf("The Deletion is successfull\n");
        hashtable->deleted.success_opr += 1;
        hashtable->deleted.success_collision += hashtable->no_of_collision;
    }
    else{
        // Check whether the file is present or not;
        // Delete and return the value 
        if(searchKeyInEvictedFile(hashtable, key) && operationOnEvictedFile(hashtable, key, Delete)){
            // printf("Deleted From evicted file successfully\n");
            hashtable->deleted.success_opr += 1;
            hashtable->deleted.success_collision += hashtable->no_of_collision;
            return;
        }
        // printf("The Deletion is unsuccessfull\n");
        hashtable->deleted.unsuccess_opr += 1;
        hashtable->deleted.unsuccess_collision += hashtable->no_of_collision;
    }
}


void* readOnHashTable(HashTable* hashtable, void* key, unsigned int hash_key){

    Bucket* bucket_ptr = NULL;
    bucket_ptr = (Bucket*)finding(hashtable, key, hash_key);
    if(bucket_ptr == NULL){
        // printf("The Read is unsuccessfull\n");
        if(searchKeyInEvictedFile(hashtable, key) && operationOnEvictedFile(hashtable, key, Read)){
            // printf("The found in Evicted file added Successfully\n");
            bucket_ptr = (Bucket*)finding(hashtable, key, hash_key);
            hashtable->read.success_opr += 1;
            hashtable->read.success_collision += hashtable->no_of_collision;
        }
        hashtable->read.unsuccess_opr += 1;
        hashtable->read.unsuccess_collision += hashtable->no_of_collision;
    }
    else{
        // printf("The Read is successfull\n");
        hashtable->read.success_opr += 1;
        hashtable->read.success_collision += hashtable->no_of_collision;
    }

    return bucket_ptr;
}

void* updateOnHashTable(HashTable* hashtable, void* key, void* value, unsigned int hash_key){

    
    void* value_ptr = NULL;
    Bucket* bucket_ptr = NULL;

    Bucket* bucket = (Bucket*)calloc(1, sizeof(Bucket));
    bucket->key = key;
    bucket->value = value; 

    bucket_ptr = (Bucket*)update(hashtable, bucket, hash_key);
    if(bucket_ptr == NULL){
        // printf("The update is unsuccessfull\n");
        if(searchKeyInEvictedFile(hashtable, key) && operationOnEvictedFile(hashtable, key, Update)){
            bucket_ptr = (Bucket*)update(hashtable, bucket, hash_key);
            // printf("The found in Evicted file added Successfully\n");
            value_ptr = &bucket_ptr->value;
            hashtable->update.success_opr += 1;
            hashtable->update.success_collision += hashtable->no_of_collision;
            free(bucket);
            return value_ptr;
        }
        hashtable->update.unsuccess_opr += 1;
        hashtable->update.unsuccess_collision += hashtable->no_of_collision;
        handleKeyValueWithAction(FreeMemory, NULL, value, hashtable);
    }

    else{
        value_ptr = &bucket_ptr->value;
        hashtable->update.success_opr += 1;
        hashtable->update.success_collision += hashtable->no_of_collision;
        // printf("The update is successfull\n");
    }
    free(bucket);
    return value_ptr;
}


void resizeHashTable(HashTable* hashtable, unsigned int adding_size){


    unsigned int allocating_size = (adding_size == 0) ? primeNumber(hashtable->current_hashtable_size*2, 1): adding_size + hashtable->current_hashtable_size;

    unsigned int old_ht_size = hashtable->current_hashtable_size;

    Bucket** old_array_of_bucket = hashtable->array_of_bucket;
    hashtable->array_of_bucket = NULL;
    hashtable->array_of_bucket = (Bucket**)calloc(allocating_size, sizeof(void*));
    hashtable->current_hashtable_size = allocating_size;
    hashtable->no_element_ht = 0;

    hashtable->other_collision += old_ht_size;
    if(hashtable->collision_handling == Linear || hashtable->collision_handling == Quadratic || hashtable->collision_handling == Double){
        for(int i = 0; i < old_ht_size; i++){
            Bucket* bucket = old_array_of_bucket[i];
            if(bucket != NULL && bucket->occupied_status != PREVIOUS_OCCUPIED){
                unsigned int hash_key = generateHashKey(hashtable, bucket->key);
                insertion(hashtable, bucket, hash_key);
            }
            if(bucket != NULL){
                free(bucket);
            }
        }
        free(old_array_of_bucket);
    }
    else if(hashtable->collision_handling == Chaining){
        for(int i = 0; i < old_ht_size; i++){
            Bucket* bucket = old_array_of_bucket[i];
            while(bucket != NULL){
                unsigned int hash_key = generateHashKey(hashtable, bucket->key);
                insertion(hashtable, (Bucket*)bucket, hash_key);
                Bucket* temp = bucket;
                bucket = bucket->nextNode;
                free(temp);
            }
        }
        free(old_array_of_bucket);
    }
}


void evictElement(HashTable* hashtable){

    // findLeastUsed - iterate
    Bucket* least_used = NULL;
    int least_used_count = INT_MAX;
    unsigned int least_used_index = 0;

    unsigned int ht_size = hashtable->current_hashtable_size;
    hashtable->other_collision += ht_size;
    for(unsigned int i = 0; i < ht_size; i++){
        Bucket* bucket_ptr = hashtable->array_of_bucket[i];
        if(hashtable->collision_handling != Chaining && bucket_ptr != NULL){
            if(bucket_ptr->occupied_status != PREVIOUS_OCCUPIED){
                if(least_used_count > bucket_ptr->access_count){
                    least_used = bucket_ptr;
                    least_used_index = i;
                    least_used_count = bucket_ptr->access_count;
                }
            }
        }
        else if(hashtable->collision_handling == Chaining){
            while (bucket_ptr != NULL)
            {

                if(least_used_count > bucket_ptr->access_count){
                    least_used = bucket_ptr;
                    least_used_index = i;
                    least_used_count = bucket_ptr->access_count;
                }

                bucket_ptr = bucket_ptr->nextNode;
            }
        }
    }

    // Store in File
    hashtable->no_evicted_element += 1;
    handleKeyValueWithAction(SaveToEvictionFile, least_used->key, least_used->value, hashtable);
    
    // Delete that Index  
    deletion(hashtable, least_used->key, least_used_index, FREE_AFTER_DELETE);
}


void refreshingEvictionFileBloomFlt(HashTable* hashtable){
    FILE* evicted_file_ptr =  hashtable->evicted_file_ptr;
    fseek(evicted_file_ptr, 0, SEEK_SET);
    char* new_temp_file_name = (char*)calloc(1, strlen(hashtable->filepath)+ 10);

    strcat(new_temp_file_name, hashtable->filepath);
    strcat(new_temp_file_name, "temp1.bin");

    FILE* temp_file_ptr = fopen(new_temp_file_name, "wb+");
    hashtable->other_collision += hashtable->no_evicted_element;
    hashtable->no_evicted_element = 0;
    hashtable->no_evicted_element_removed = 0;

    while(1){
        char available_status = fgetc(evicted_file_ptr);
        if(available_status == EOF || (available_status != AVAILABLE_TO_ACCESS && available_status != NOT_AVAILABLE_TO_ACCESS)){
            break;
        }
        void* key_in_file = calloc(1, hashtable->key_size);
        fread(key_in_file , hashtable->key_size, 1, evicted_file_ptr);
        size_t total_store_size = 0;

        if(hashtable->file_format == 'T'){
            fscanf(evicted_file_ptr, "%zu ", &total_store_size);
        }
        else if(hashtable->file_format == 'B'){
            fread(&total_store_size, sizeof(size_t), 1, evicted_file_ptr);
        }

        if(available_status == AVAILABLE_TO_ACCESS){
            hashtable->no_evicted_element += 1;
            add_value_bloom_filter(hashtable->bloom_flt, key_in_file, hashtable->key_size);

            fwrite(&available_status, sizeof(char), 1, temp_file_ptr);
            fwrite(key_in_file, hashtable->key_size, 1, temp_file_ptr);
            char* tranporting_buf;
            if(hashtable->file_format == 'T'){
                fprintf(temp_file_ptr, "%20zu ", total_store_size);
                tranporting_buf = (char*)calloc(1, total_store_size + 1);
                fgets(tranporting_buf, total_store_size + 1, evicted_file_ptr);
            }
            else if(hashtable->file_format == 'B'){    
                fwrite(&total_store_size, sizeof(size_t), 1, temp_file_ptr);
                tranporting_buf = (char*)calloc(1, total_store_size);
                fread(tranporting_buf, total_store_size, 1, evicted_file_ptr);
            }
            fwrite(tranporting_buf, total_store_size, 1, temp_file_ptr);
            free(tranporting_buf);
        }
        else{
            fseek(evicted_file_ptr, total_store_size, SEEK_CUR);
        }
        free(key_in_file);
    }   
    fclose(hashtable->evicted_file_ptr);
    hashtable->evicted_file_ptr = temp_file_ptr;
    remove(hashtable->eviction_fileName);
    rename(new_temp_file_name, hashtable->eviction_fileName);
    free(new_temp_file_name);
}



void resizeOrEvict(HashTable* hashtable){

    // HashTable Resize or Evict Element
    float load_factor = (float)hashtable->no_element_ht / (float)hashtable->current_hashtable_size;
    if(((hashtable->collision_handling == Linear || hashtable->collision_handling == Quadratic) && load_factor >= 0.5) ||
        (hashtable->collision_handling == Double && load_factor >= 0.75) || 
        (hashtable->collision_handling == Chaining && load_factor >= 0.75) ){
            if(primeNumber(hashtable->current_hashtable_size*2, 1) < hashtable->max_ht_size_limit ){ 
                resizeHashTable(hashtable, 0);
            }
            else{
                // printf("Evict Called\n");
                evictElement(hashtable);
            }
    }
    
    // Increasing the Bloom filter size
    // if(hashtable->no_evicted_element > hashtable->current_bloom_filter_size && get_current_positivity_rate(hashtable->bloom_flt) > 0.05){
    //         hashtable->current_bloom_filter_size *= 2;
    //         free_bloom_filter(hashtable->bloom_flt);
    //         bloom_filter_init(hashtable->bloom_flt, hashtable->current_bloom_filter_size, 0.05, multiHashing);
    //         refreshingEvictionFileBloomFlt(hashtable);
    //         printf("Bloom Blooming\n\n");
    // }
    // Refreshing Bloom Filter and Eviction File
    if(hashtable->no_evicted_element > hashtable->current_bloom_filter_size){
        printf("The Eviction file size is beyond bloom filter size.\n Operation on Hash Table going to inefficient.\n");
    }
    if(hashtable->no_evicted_element != 0){
        float evict_element_valid_element_ratio = (float)hashtable->no_evicted_element_removed / (float)hashtable->no_evicted_element;
        if(evict_element_valid_element_ratio > 0.5 || get_current_positivity_rate(hashtable->bloom_flt) > 0.05){
            reset_bloom_filter(hashtable->bloom_flt);
            refreshingEvictionFileBloomFlt(hashtable);
        }
    }
}

void* operationOnHashTable(void* key, void* value, HashTable* hashtable, Operation operation){
    if(key == NULL || hashtable == NULL){
        printf("Invalid Key or Hash Table\n");
        return NULL;
    }
    void* return_ptr = NULL;
    unsigned int hash_key = generateHashKey(hashtable, key);

    hashtable->no_of_collision = 0;

    switch (operation)
    {
    case 1:
        return_ptr = insertOnHashTable(hashtable, key, value, hash_key);
        break;
    case 2:
        deleteOnHashTable(hashtable, key, hash_key);
        break;
    case 3:
        return_ptr = readOnHashTable(hashtable, key, hash_key);
        break;
    case 4: 
        return_ptr = updateOnHashTable(hashtable, key, value, hash_key);
        break;
    default:
        break;
    }

    
    return return_ptr;
}

HashTable* generateHashTableWithFile(char* file, Pair* (*generateKeyValuePtrFromFile)(size_t key_size, FILE* file_ptr), char file_format, unsigned int initial_size, unsigned int max_ht_size_limit, void (*handleKeyValuePtr) (void* key, void* value_ptr, Action_On_Key_Value action_on_key_value, HashTable* hashtable), Pair* (*generateKeyValuePtr)(size_t key_size, FILE* file_ptr), char* newFileName, uint64_t (*hashFunction) (void* value, size_t size), CollisionHandling collision_handling, char new_format){
    
    FILE* file_ptr = fopen(file, "rb");

    size_t key_size = 0;

    unsigned int total_elements = 0;

    if(file_format == 'T'){
        fscanf(file_ptr, "%zu ", &key_size);
        fscanf(file_ptr, "%u ", &total_elements);
    }
    else if(file_format == 'B'){
        fread(&key_size, sizeof(size_t), 1, file_ptr);
        fread(&total_elements, sizeof(unsigned int), 1, file_ptr);
    }

    printf("%zu %u. \n", key_size, total_elements);
    HashTable* hashtable = setupHashTable(initial_size, max_ht_size_limit, key_size, handleKeyValuePtr, generateKeyValuePtr, newFileName, NULL, collision_handling, new_format);

    for(int i = 0; i < total_elements; i++){       
        Pair* pair = generateKeyValuePtrFromFile(key_size, file_ptr);
        // printf("%p ", value_ptr);
        // printf("%d ", *(int*)pair->key);
        // displayValuePtr(pair->value_ptr);
        // printf("\n");
        operationOnHashTable(pair->key, pair->value_ptr, hashtable, Insert);
        free(pair->key);
        free(pair);
    }

    fclose(file_ptr);
    return hashtable;
    return NULL;
}

void iterateHashTable(HashTable* hashtable, unsigned int iterator, void (*keyFunction)(void* key_ptr), void (*valueFunction)(void* value_ptr)){
    unsigned int ht_size = hashtable->current_hashtable_size;
    for(unsigned int i = iterator; i < ht_size; i++){
        Bucket* bucket_value_ptr = hashtable->array_of_bucket[i];
        if(hashtable->collision_handling != Chaining && bucket_value_ptr != NULL){
            if(bucket_value_ptr->occupied_status != 1){
                // Write Own Function
                // printf("%p %d %s\n", bucket_value_ptr, *(int*)bucket_value_ptr->key, (char*)bucket_value_ptr->value);
                keyFunction(bucket_value_ptr->key);
                valueFunction(bucket_value_ptr->value);

            }
        }
        else if(hashtable->collision_handling == Chaining){
            while (bucket_value_ptr != NULL)
            {
                // Write Own Function
                // printf("%p  %d %s\n", bucket_value_ptr, *(int*)bucket_value_ptr->key, (char*)bucket_value_ptr->value);
                keyFunction(bucket_value_ptr->key);
                valueFunction(bucket_value_ptr->value);
                bucket_value_ptr = bucket_value_ptr->nextNode;
            }
        }
    }
}