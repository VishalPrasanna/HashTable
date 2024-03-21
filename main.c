#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <clhash.h>
#include "./benchmark/clockbenchmark.c"
#include "HashTable.h"


void displayChainingHashTable(HashTable* hashtable){
    printf("\n%u %u\n", hashtable->no_element_ht, hashtable->current_hashtable_size);
    Bucket** array_of_bucket = hashtable->array_of_bucket;
    printf("\nHashkey     Bucket_adr          node_adr    Key      Value_adr       Next Node\n");
    for(int i = 0; i < hashtable->current_hashtable_size; i++){
        Bucket* bucket = array_of_bucket[i];

        if(bucket == NULL){
            printf("%5d %16p %16p\n", i, array_of_bucket+i, array_of_bucket[i]);
        }
        else{
            printf("%5d %16p ", i, array_of_bucket+i);
            while(bucket != NULL){
                    printf("%16p  %6d %16p  %16p ---> ", bucket, *(int*)bucket->key, bucket->value, bucket->nextNode);

                bucket = bucket->nextNode;
            }
            printf("\n");
        }
    }
}

void displayProbingHashTable(HashTable* hashtable){
    printf("\n%u %u\n", hashtable->no_element_ht, hashtable->current_hashtable_size);        
    // Bucket** array_Of_Bucket = (Bucket**)hashtable->array_of_bucket;
    // printf("\nindex       Bucket_adr      Pointing_adr     Key        Value_     status   Count\n");

    // for(int i = 0; i < hashtable->current_hashtable_size ; i++){
    //     Bucket* index_bucket = array_Of_Bucket[i]; 
    //     if(index_bucket == NULL){
    //         printf("%5d %16p  %16p\n", i, array_Of_Bucket+i, index_bucket);
    //     } 
    //     else{
    //         if(index_bucket->occupied_status == 1){
    //             printf("%5d %16p  %16p %6p  %15p %5d %5d\n",i, array_Of_Bucket+i, index_bucket, index_bucket->key, index_bucket->value, index_bucket->occupied_status, index_bucket->access_count);
    //         }
    //         else{
    //                 printf("%5d %16p  %16p %5d  %16p %5d %5d\n",i, array_Of_Bucket+i, index_bucket, *(int*)index_bucket->key, index_bucket->value, index_bucket->occupied_status, index_bucket->access_count);
    //         }
    //     }
    // }
}



uint64_t customHashFunction(void* value, size_t size){
    void * random =  get_random_key_for_clhash(UINT64_C(0x23a23cf5033c3c81),UINT64_C(0xb3816f6a2c68e530));
    uint64_t hashvalue = clhash(random, value, size);
    free(random);
    return hashvalue;
}   

unsigned int customHashFunctionInt(void* key, size_t size){
    unsigned int i = *(int*)key;
    return i;
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

void displayValuePtr(void* value_ptr){
    Teacher* teacher_value_ptr = value_ptr;
    printf("Value:%u %s %hu %f %u %f %lu %s %u %s %hu %f %u %f %u %s\n", teacher_value_ptr->teacher_id, teacher_value_ptr->name, teacher_value_ptr->age, teacher_value_ptr->attendance,
                    teacher_value_ptr->phone_no, teacher_value_ptr->salary, teacher_value_ptr->account_no, teacher_value_ptr->address  ,
                    teacher_value_ptr->s.student_id, teacher_value_ptr->s.name, teacher_value_ptr->s.age, teacher_value_ptr->s.attendance, teacher_value_ptr->s.phone_no, teacher_value_ptr->s.cgpa, teacher_value_ptr->s.register_no, teacher_value_ptr->s.address);
}


void displayKeyPtr(void* key_ptr){
    unsigned int key = *(unsigned int*)key_ptr;
    printf("Key:%u ", key);
}


void* generateValuePtrInitial(FILE* file_ptr){

    Teacher* teacher_value_ptr = (Teacher*)calloc(1, sizeof(Teacher));
    unsigned int t_name_size = 0, s_name_size, s_address_size;
    
    fscanf(file_ptr, "%u ", &teacher_value_ptr->teacher_id);

    fscanf(file_ptr, "%u ", &t_name_size);

    char* t_name = (char*)calloc(1, t_name_size + 2);                    
    fscanf(file_ptr, "%s %hu %f %u %f %lu %s %u ", t_name, &teacher_value_ptr->age, &teacher_value_ptr->attendance,
                    &teacher_value_ptr->phone_no, &teacher_value_ptr->salary, &teacher_value_ptr->account_no, teacher_value_ptr->address,
                    &teacher_value_ptr->s.student_id);

    fscanf(file_ptr, "%u ", &s_name_size);
    char* s_name = (char*)calloc(1, s_name_size + 2); 
    fscanf(file_ptr, "%s %hu %f %u %f %u ", s_name, &teacher_value_ptr->s.age, &teacher_value_ptr->s.attendance, &teacher_value_ptr->s.phone_no, &teacher_value_ptr->s.cgpa, &teacher_value_ptr->s.register_no);
    
    fscanf(file_ptr, "%u ", &s_address_size);
    char* s_address = (char*)calloc(1, s_address_size + 2); 
    fscanf(file_ptr, "%s\n", s_address);

    // printf("%u %u %u\n", t_name_size, s_name_size, s_address_size);
    
    teacher_value_ptr->name = t_name;
    teacher_value_ptr->s.name = s_name;
    teacher_value_ptr->s.address = s_address;
    return teacher_value_ptr;
}


Pair* generateKeyValuePtrWithTxt(size_t key_size, FILE* file_ptr){

    Teacher* teacher_value_ptr = (Teacher*)calloc(1, sizeof(Teacher));
    size_t total_dt_size = 0, stored_value_size = 0;

    fscanf(file_ptr, "%zu ", &stored_value_size);
    fscanf(file_ptr, "%zu ", &total_dt_size);
    size_t unsure_size = total_dt_size - (__SIZEOF_INT__ * 5 + __SIZEOF_SHORT__ * 2 + __SIZEOF_FLOAT__ * 4 + __SIZEOF_LONG__ + sizeof(char)* 21  + key_size);
    char* unsure_ptr = (char*)calloc(1, unsure_size);
    int indexing_sure_ptr = 0;

    void* key = (char*)calloc(1, key_size);
    fscanf(file_ptr, "%u ", (unsigned int*)key);
    fscanf(file_ptr, "%u ", &teacher_value_ptr->teacher_id);

    while (1){
        char temp = fgetc(file_ptr);
        unsure_ptr[indexing_sure_ptr] = temp;
        indexing_sure_ptr++;
        if(temp == '\0')
            break;
    }

    char* t_name_sure_ptr = (char*)malloc(indexing_sure_ptr);
    memcpy(t_name_sure_ptr, unsure_ptr, indexing_sure_ptr);
    free(unsure_ptr);

    unsure_ptr = NULL;
    unsure_size -= indexing_sure_ptr;
    unsure_ptr = (char*)calloc(1, unsure_size);
    indexing_sure_ptr = 0; 

    fscanf(file_ptr, "%hu %f %u %f %lu %s ", &teacher_value_ptr->age, &teacher_value_ptr->attendance, &teacher_value_ptr->phone_no, &teacher_value_ptr->salary, &teacher_value_ptr->account_no, teacher_value_ptr->address);
    fscanf(file_ptr, "%u ", &teacher_value_ptr->s.student_id);

    while(1){
        char temp = fgetc(file_ptr);
        unsure_ptr[indexing_sure_ptr] = temp;
        indexing_sure_ptr++;
        if(temp == '\0')
            break;
    }

    char* s_name_sure_ptr = (char*)malloc(indexing_sure_ptr);
    memcpy(t_name_sure_ptr, unsure_ptr, indexing_sure_ptr);
    free(unsure_ptr);
    unsure_ptr = NULL;
    unsure_size -= indexing_sure_ptr;
    unsure_ptr = (char*)calloc(1, unsure_size);
    indexing_sure_ptr = 0;

    fscanf(file_ptr, "%hu %f %u %f %u ", &teacher_value_ptr->s.age, &teacher_value_ptr->s.attendance, &teacher_value_ptr->s.phone_no, &teacher_value_ptr->s.cgpa, &teacher_value_ptr->s.register_no);
    
    while (1){
        char temp = fgetc(file_ptr);
        unsure_ptr[indexing_sure_ptr] = temp;
        indexing_sure_ptr++;
        if(temp == '\0')
            break;
    }
    char* s_address_sure_ptr = (char*)malloc(indexing_sure_ptr);
    memcpy(t_name_sure_ptr, unsure_ptr, indexing_sure_ptr);
    free(unsure_ptr);
    teacher_value_ptr->name = t_name_sure_ptr;
    teacher_value_ptr->s.name = s_name_sure_ptr;
    teacher_value_ptr->s.address = s_address_sure_ptr;

    Pair* pair = (Pair*)calloc(1, sizeof(Pair));
    pair->key = key;
    pair->value_ptr = teacher_value_ptr;
    return pair;
}

Pair* generateKeyValuePtrWithBin(size_t key_size, FILE* file_ptr){
    
    Teacher* teacher_value_ptr = calloc(1, sizeof(Teacher));

    size_t total_dt_size;
    fread(&total_dt_size, sizeof(size_t), 1, file_ptr);

    size_t unsure_size = total_dt_size - (__SIZEOF_INT__ * 5 + __SIZEOF_SHORT__ * 2 + __SIZEOF_FLOAT__ * 4 + __SIZEOF_LONG__ + sizeof(char)* 21 + key_size);
    char* unsure_ptr = calloc(1, unsure_size);
    int indexing_sure_ptr = 0;

    void* key = calloc(1, key_size);
    fread(key, key_size, 1, file_ptr);
    fread(&teacher_value_ptr->teacher_id, sizeof(unsigned int), 1, file_ptr);

    while (1){
        char temp = fgetc(file_ptr);
        unsure_ptr[indexing_sure_ptr] = temp;
        indexing_sure_ptr++;
        if(temp == '\0')
            break;
    }

    char* t_name_sure_ptr = malloc(indexing_sure_ptr);
    memcpy(t_name_sure_ptr, unsure_ptr, indexing_sure_ptr);
    free(unsure_ptr);
    unsure_ptr = NULL;
    unsure_size -= indexing_sure_ptr;
    unsure_ptr = calloc(1, unsure_size);
    indexing_sure_ptr = 0; 

    fread(&teacher_value_ptr->age, sizeof(unsigned short int), 1, file_ptr);
    fread(&teacher_value_ptr->attendance, sizeof(float), 1, file_ptr);
    fread(&teacher_value_ptr->phone_no, sizeof(unsigned int), 1, file_ptr);
    fread(&teacher_value_ptr->salary, sizeof(float), 1, file_ptr);
    fread(&teacher_value_ptr->account_no, sizeof(unsigned long int), 1, file_ptr);
    fread(&teacher_value_ptr->address, sizeof(char)*21, 1, file_ptr);
    fread(&teacher_value_ptr->s.student_id, sizeof(unsigned int), 1, file_ptr);
    
    while(1){
        char temp = fgetc(file_ptr);
        unsure_ptr[indexing_sure_ptr] = temp;
        indexing_sure_ptr++;
        if(temp == '\0')
            break;
    }

    char* s_name_sure_ptr =  malloc(indexing_sure_ptr);
    memcpy(t_name_sure_ptr, unsure_ptr, indexing_sure_ptr);
    free(unsure_ptr);
    unsure_ptr = NULL;
    unsure_size -= indexing_sure_ptr;
    unsure_ptr = calloc(1, unsure_size);
    indexing_sure_ptr = 0;
    
    fread(&teacher_value_ptr->s.age, sizeof(unsigned short int), 1, file_ptr);
    fread(&teacher_value_ptr->s.attendance, sizeof(float), 1, file_ptr);
    fread(&teacher_value_ptr->s.phone_no, sizeof(unsigned int), 1, file_ptr);
    fread(&teacher_value_ptr->s.cgpa, sizeof(float), 1, file_ptr);
    fread(&teacher_value_ptr->s.register_no, sizeof(unsigned int), 1, file_ptr);

    while(1){
        char temp = fgetc(file_ptr);
        unsure_ptr[indexing_sure_ptr] = temp;
        indexing_sure_ptr++;
        if(temp == '\0')
            break;
    }
    char* s_address_sure_ptr = unsure_ptr;
    teacher_value_ptr->name = t_name_sure_ptr;
    teacher_value_ptr->s.name = s_name_sure_ptr;
    teacher_value_ptr->s.address = s_address_sure_ptr;

    Pair* pair = calloc(1, sizeof(Pair));
    pair->key = key;
    pair->value_ptr = teacher_value_ptr;
    return pair;
}

void handleKeyValuePtrWithTxt(void* key, void* value_ptr, Action_On_Key_Value action_on_key_value, HashTable* hashtable){

    Teacher *teacher_key = key;
    Teacher *teacher_value_ptr = value_ptr;

    if(action_on_key_value == SaveToEvictionFile || action_on_key_value == SaveToFileAndFreeMemory){
        
        FILE* file_ptr;

        if(action_on_key_value == SaveToEvictionFile){
            file_ptr = hashtable->evicted_file_ptr;
        }
        else{
            file_ptr = hashtable->file_ptr;
        }

        size_t stored_value_size = 0;
        size_t total_dt_size_cont_mem = 0;

        total_dt_size_cont_mem = (__SIZEOF_INT__ * 5 + __SIZEOF_SHORT__ * 2 + __SIZEOF_FLOAT__ * 4 + __SIZEOF_LONG__ + sizeof(char)* 21 + hashtable->key_size );  

        size_t size_of_t_name = strlen(teacher_value_ptr->name)+1;
        size_t size_of_s_name = strlen(teacher_value_ptr->s.name)+1;
        size_t size_of_s_address = strlen(teacher_value_ptr->s.address)+1;

        total_dt_size_cont_mem += (size_of_t_name + size_of_s_name + size_of_s_address);

        size_t temp = fprintf(file_ptr, "%20zu ", stored_value_size);
        stored_value_size +=  fprintf(file_ptr, "%zu ", total_dt_size_cont_mem);

        stored_value_size += fprintf(file_ptr, "%u ", *(unsigned int*)key);

        stored_value_size += fprintf(file_ptr, "%u %s%c %hu %f %u %f %lu %s%c %u %s%c %hu %f %u %f %u %s%c", teacher_value_ptr->teacher_id, teacher_value_ptr->name, 0, teacher_value_ptr->age, teacher_value_ptr->attendance,
        teacher_value_ptr->phone_no, teacher_value_ptr->salary, teacher_value_ptr->account_no, teacher_value_ptr->address,0,
        teacher_value_ptr->s.student_id, teacher_value_ptr->s.name, 0,  teacher_value_ptr->s.age, teacher_value_ptr->s.attendance, teacher_value_ptr->s.phone_no, teacher_value_ptr->s.cgpa, teacher_value_ptr->s.register_no, teacher_value_ptr->s.address, 0);
            
        fseek(file_ptr, -(temp + stored_value_size), SEEK_CUR);
        fprintf(file_ptr, "%20zu", stored_value_size);
        fseek(file_ptr, 0, SEEK_END);
    }

    if(action_on_key_value == FreeMemory || action_on_key_value == SaveToFileAndFreeMemory ){
        free(teacher_value_ptr->name);
        free(teacher_value_ptr->s.name);
        free(teacher_value_ptr->s.address);
        free(teacher_value_ptr);
        value_ptr = NULL;
    }
}

void handleKeyValuePtrWithBin(void* key, void* value_ptr, Action_On_Key_Value action_on_key_value, HashTable* hashtable){

    Teacher* teacher_value_ptr = value_ptr;
    
    if(action_on_key_value == SaveToEvictionFile || action_on_key_value == SaveToFileAndFreeMemory){
        
        FILE* file_ptr;
        
        if(action_on_key_value == SaveToEvictionFile){
            file_ptr = hashtable->evicted_file_ptr;
        }
        else{
            file_ptr = hashtable->file_ptr;
        }

        size_t total_dt_size_cont_mem = 0;
        total_dt_size_cont_mem = (__SIZEOF_INT__ * 5 + __SIZEOF_SHORT__ * 2 + __SIZEOF_FLOAT__ * 4 + __SIZEOF_LONG__ + sizeof(char)* 21 + hashtable->key_size) ;    
        size_t size_of_t_name = strlen(teacher_value_ptr->name)+1;
        size_t size_of_s_name = strlen(teacher_value_ptr->s.name)+1;
        size_t size_of_s_address = strlen(teacher_value_ptr->s.address)+1;
        total_dt_size_cont_mem += (size_of_t_name + size_of_s_name + size_of_s_address);

        fwrite(&total_dt_size_cont_mem, sizeof(size_t), 1, file_ptr);
        
        fwrite(key, hashtable->key_size, 1, hashtable->evicted_file_ptr);

        fwrite(&teacher_value_ptr->teacher_id, sizeof(unsigned int), 1, file_ptr);
        fwrite(teacher_value_ptr->name, strlen(teacher_value_ptr->name)+1, 1, file_ptr);
        fwrite(&teacher_value_ptr->age, sizeof(unsigned short int), 1, file_ptr);
        fwrite(&teacher_value_ptr->attendance, sizeof(float), 1, file_ptr);
        fwrite(&teacher_value_ptr->phone_no, sizeof(unsigned int), 1, file_ptr);
        fwrite(&teacher_value_ptr->salary, sizeof(float), 1, file_ptr);
        fwrite(&teacher_value_ptr->account_no, sizeof(unsigned long int), 1, file_ptr);
        fwrite(&teacher_value_ptr->address, sizeof(char)*21, 1, file_ptr);

        fwrite(&teacher_value_ptr->s.student_id, sizeof(unsigned int), 1, file_ptr);
        fwrite(teacher_value_ptr->s.name, strlen(teacher_value_ptr->s.name)+1, 1, file_ptr);
        fwrite(&teacher_value_ptr->s.age, sizeof(unsigned short int), 1, file_ptr);
        fwrite(&teacher_value_ptr->s.attendance, sizeof(float), 1, file_ptr);
        fwrite(&teacher_value_ptr->s.phone_no, sizeof(unsigned int), 1, file_ptr);
        fwrite(&teacher_value_ptr->s.cgpa, sizeof(float), 1, file_ptr);
        fwrite(&teacher_value_ptr->s.register_no, sizeof(unsigned int), 1, file_ptr);
        fwrite(teacher_value_ptr->s.address, strlen(teacher_value_ptr->s.address)+1, 1, file_ptr);
    }

    if(action_on_key_value == FreeMemory || action_on_key_value == SaveToFileAndFreeMemory){
        free(teacher_value_ptr->name);
        free(teacher_value_ptr->s.name);
        free(teacher_value_ptr->s.address);
        free(teacher_value_ptr);
        value_ptr = NULL;
    }
}


void handleKeyValuePtr(void* key, void* value_ptr, Action_On_Key_Value action_on_key_value, HashTable* hashtable){

    Teacher* teacher_value_ptr = value_ptr;
    
    if(action_on_key_value == SaveToEvictionFile || action_on_key_value == SaveToFileAndFreeMemory){

        size_t total_dt_size_cont_mem = 0;
        total_dt_size_cont_mem = (__SIZEOF_INT__ * 5 + __SIZEOF_SHORT__ * 2 + __SIZEOF_FLOAT__ * 4 + __SIZEOF_LONG__ + sizeof(char)* 21 + hashtable->key_size) ;    
        size_t size_of_t_name = strlen(teacher_value_ptr->name)+1;
        size_t size_of_s_name = strlen(teacher_value_ptr->s.name)+1;
        size_t size_of_s_address = strlen(teacher_value_ptr->s.address)+1;
        total_dt_size_cont_mem += (size_of_t_name + size_of_s_name + size_of_s_address);

        void* cont_mem_ptr = calloc(1, total_dt_size_cont_mem);
        void* cpy_cont_mem_ptr = cont_mem_ptr;
        memcpy(cont_mem_ptr, &total_dt_size_cont_mem, sizeof(size_t));
        cont_mem_ptr += sizeof(size_t);

        memcpy(cont_mem_ptr, key, hashtable->key_size);
        cont_mem_ptr += hashtable->key_size;

        memcpy(cont_mem_ptr, &teacher_value_ptr->teacher_id, sizeof(unsigned int));
        cont_mem_ptr += sizeof(unsigned int);

        memcpy(cont_mem_ptr, teacher_value_ptr->name, size_of_t_name);
        cont_mem_ptr += size_of_t_name;

        memcpy(cont_mem_ptr, &teacher_value_ptr->age, sizeof(unsigned short int));
        cont_mem_ptr += sizeof(unsigned short int);

        memcpy(cont_mem_ptr, &teacher_value_ptr->attendance, sizeof(float));
        cont_mem_ptr += sizeof(float);

        memcpy(cont_mem_ptr, &teacher_value_ptr->phone_no, sizeof(unsigned int));
        cont_mem_ptr += sizeof(unsigned int);

        memcpy(cont_mem_ptr, &teacher_value_ptr->salary, sizeof(float));
        cont_mem_ptr += sizeof(float);

        memcpy(cont_mem_ptr, &teacher_value_ptr->account_no, sizeof(unsigned long int));
        cont_mem_ptr += sizeof(unsigned long int);

        memcpy(cont_mem_ptr, teacher_value_ptr->address, sizeof(char)*21);
        cont_mem_ptr += sizeof(char)*21;

        memcpy(cont_mem_ptr, &teacher_value_ptr->s.student_id, sizeof(unsigned int));
        cont_mem_ptr += sizeof(unsigned int);

        memcpy(cont_mem_ptr, teacher_value_ptr->s.name, size_of_s_name);
        cont_mem_ptr += size_of_s_name;

        memcpy(cont_mem_ptr, &teacher_value_ptr->s.age, sizeof(unsigned short int));
        cont_mem_ptr += sizeof(unsigned short int);

        memcpy(cont_mem_ptr, &teacher_value_ptr->s.attendance, sizeof(float));
        cont_mem_ptr += sizeof(float);

        memcpy(cont_mem_ptr, &teacher_value_ptr->s.phone_no, sizeof(unsigned int));
        cont_mem_ptr += sizeof(unsigned int);

        memcpy(cont_mem_ptr, &teacher_value_ptr->s.cgpa, sizeof(float));
        cont_mem_ptr += sizeof(float);

        memcpy(cont_mem_ptr, &teacher_value_ptr->s.register_no, sizeof(unsigned int));
        cont_mem_ptr += sizeof(unsigned int);

        memcpy(cont_mem_ptr, teacher_value_ptr->s.address, size_of_s_address);
        cont_mem_ptr += size_of_s_address;

    }

    if(action_on_key_value == FreeMemory || action_on_key_value == SaveToFileAndFreeMemory){
        free(teacher_value_ptr->name);
        free(teacher_value_ptr->s.name);
        free(teacher_value_ptr->s.address);
        free(teacher_value_ptr);
        value_ptr = NULL;
    }
}


int main(){

    clock_t start = clock();

    CollisionHandling collision_handling = 1;
    unsigned int no_of_operation = 10000;
    unsigned int  table_size_limit = 1000;

    Collision ar_opr[4], opr_analysis = {0, 0, 0, 0};
    unsigned int other_collision = 0;

        FILE* random_file_ptr = fopen("./files/RandomData.txt", "r");
        FILE* file_ptr = fopen("./files/Data.txt", "r");

        HashTable* hashtable = setupHashTable(0, table_size_limit, sizeof(unsigned int), handleKeyValuePtrWithBin, generateKeyValuePtrWithBin, "./files/benchmarkelements", NULL, collision_handling, 'T');
        
        for(unsigned int i = 0; i < 20000; i++){
            void* value_ptr = generateValuePtrInitial(file_ptr);
            operationOnHashTable(&i, value_ptr, hashtable, Insert);
        }

        for(unsigned int i = 0; i < 5000; i++){
            printf("%d \n", i);
            operationOnHashTable(&i, NULL, hashtable, Delete);
        }


        other_collision = hashtable->other_collision;
        
        closeHashTable(1, hashtable);
        fclose(random_file_ptr);
        fclose(file_ptr);

    printf("\n%f\n", ((double)(clock() - start))/ CLOCKS_PER_SEC);

    printf("\nEND\n");
    return 0;
}
    