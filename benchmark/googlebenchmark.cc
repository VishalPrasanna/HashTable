#include <benchmark/benchmark.h>
#include <iostream>
#include "HashTable.h"



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
    Teacher* teacher_value_ptr = (Teacher*)value_ptr;
    printf("Value:%u %s %hu %f %u %f %lu %s %u %s %hu %f %u %f %u %s\n", teacher_value_ptr->teacher_id, teacher_value_ptr->name, teacher_value_ptr->age, teacher_value_ptr->attendance,
                    teacher_value_ptr->phone_no, teacher_value_ptr->salary, teacher_value_ptr->account_no, teacher_value_ptr->address  ,
                    teacher_value_ptr->s.student_id, teacher_value_ptr->s.name, teacher_value_ptr->s.age, teacher_value_ptr->s.attendance, teacher_value_ptr->s.phone_no, teacher_value_ptr->s.cgpa, teacher_value_ptr->s.register_no, teacher_value_ptr->s.address);
}


void* generateValuePtrInitial(FILE* file_ptr){

    Teacher* teacher_value_ptr = (Teacher*)calloc(1, sizeof(Teacher));
    int t_name_size = 0, s_name_size, s_address_size;
    
    fscanf(file_ptr, "%u ", &teacher_value_ptr->teacher_id);

    fscanf(file_ptr, "%d ", &t_name_size);
    char* t_name = (char*)calloc(1, t_name_size + 1);                    
    fscanf(file_ptr, "%s %hu %f %u %f %lu %s %u ", t_name, &teacher_value_ptr->age, &teacher_value_ptr->attendance,
                    &teacher_value_ptr->phone_no, &teacher_value_ptr->salary, &teacher_value_ptr->account_no, teacher_value_ptr->address,
                    &teacher_value_ptr->s.student_id);

    fscanf(file_ptr, "%d ", &s_name_size);
    char* s_name = (char*)calloc(1, s_name_size +1); 
    fscanf(file_ptr, "%s %hu %f %u %f %u ", s_name, &teacher_value_ptr->s.age, &teacher_value_ptr->s.attendance, &teacher_value_ptr->s.phone_no, &teacher_value_ptr->s.cgpa, &teacher_value_ptr->s.register_no);
    
    fscanf(file_ptr, "%d ", &s_address_size);
    char* s_address = (char*)calloc(1, s_address_size + 1); 
    fscanf(file_ptr, "%s\n", s_address);
    
    teacher_value_ptr->name = t_name;
    teacher_value_ptr->s.name = s_name;
    teacher_value_ptr->s.address = s_address;
    return teacher_value_ptr;
}


Pair* generateKeyValuePtr(size_t key_size, FILE* file_ptr){
    
    Teacher* teacher_value_ptr = (Teacher*)calloc(1, sizeof(Teacher));

    size_t total_dt_size;
    fread(&total_dt_size, sizeof(size_t), 1, file_ptr);

    size_t unsure_size = total_dt_size - (__SIZEOF_INT__ * 5 + __SIZEOF_SHORT__ * 2 + __SIZEOF_FLOAT__ * 4 + __SIZEOF_LONG__ + sizeof(char)* 21 + key_size);
    char* unsure_ptr = (char*)calloc(1, unsure_size);
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

    char* t_name_sure_ptr = (char*)malloc(indexing_sure_ptr);
    memcpy(t_name_sure_ptr, unsure_ptr, indexing_sure_ptr);
    free(unsure_ptr);
    unsure_ptr = NULL;
    unsure_size -= indexing_sure_ptr;
    unsure_ptr = (char*)calloc(1, unsure_size);
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

    char* s_name_sure_ptr =  (char*)malloc(indexing_sure_ptr);
    memcpy(s_name_sure_ptr, unsure_ptr, indexing_sure_ptr);
    free(unsure_ptr);
    unsure_ptr = NULL;
    unsure_size -= indexing_sure_ptr;
    unsure_ptr = (char*)calloc(1, unsure_size);
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

    Pair* pair = (Pair*)calloc(1, sizeof(Pair));
    pair->key = key;
    pair->value_ptr = teacher_value_ptr;
    return pair;
}


Continue_Memroy* handleKeyValuePtr(void* key, void* value_ptr, Action_On_Key_Value action_on_key_value, HashTable* hashtable){

    Teacher* teacher_value_ptr = (Teacher*)value_ptr;
    Continue_Memroy* continue_memory = NULL;

    if(action_on_key_value == SaveToEvictionFile || action_on_key_value == SaveToFileAndFreeMemory){

        size_t total_dt_size_cont_mem = 0;
        total_dt_size_cont_mem = (__SIZEOF_INT__ * 5 + __SIZEOF_SHORT__ * 2 + __SIZEOF_FLOAT__ * 4 + __SIZEOF_LONG__ + sizeof(char)* 21 + hashtable->key_size) ;    
        size_t size_of_t_name = strlen(teacher_value_ptr->name)+1;
        size_t size_of_s_name = strlen(teacher_value_ptr->s.name)+1;
        size_t size_of_s_address = strlen(teacher_value_ptr->s.address)+1;
        total_dt_size_cont_mem += (size_of_t_name + size_of_s_name + size_of_s_address);

        char* cont_mem_ptr = (char*)calloc(1, total_dt_size_cont_mem);
        void* cpy_cont_mem_ptr = cont_mem_ptr;

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

        continue_memory = (Continue_Memroy*) calloc(1, sizeof(Continue_Memroy));
        continue_memory->cont_size = total_dt_size_cont_mem;
        continue_memory->cont_mem_ptr = cpy_cont_mem_ptr;
    }

    if(action_on_key_value == FreeMemory || action_on_key_value == SaveToFileAndFreeMemory){
        free(teacher_value_ptr->name);
        free(teacher_value_ptr->s.name);
        free(teacher_value_ptr->s.address);
        free(teacher_value_ptr);
        value_ptr = NULL;
    }
    return continue_memory;
}



using namespace std;

// Define another benchmark
static void BM_RandomAccess(benchmark::State& state) {

    CollisionHandling collision_handling = (CollisionHandling)state.range(0);
    unsigned int initial_insert_elements = state.range(1);
    unsigned int no_of_operation = state.range(2);
    unsigned int  table_size_limit = state.range(3);

    Collision ar_opr[4] = {0}, opr_analysis = {0, 0, 0, 0};
    unsigned int other_collision = 0;
    unsigned int total_elements_in_file = 0;

    for (auto _ : state){

        FILE* random_file_ptr = fopen("./files/RandomData.txt", "r");
        FILE* file_ptr = fopen("./files/Data.txt", "r");

        HashTable* hashtable = setupHashTable(0, table_size_limit, sizeof(unsigned int), handleKeyValuePtr, generateKeyValuePtr, (char*)"./files/benchmarkHT", NULL, collision_handling);
            
        for(unsigned int i = 0; i < initial_insert_elements; i++){
            void* value_ptr = generateValuePtrInitial(file_ptr);
            operationOnHashTable(&i, value_ptr, hashtable, Insert);
        }
        for(unsigned int i = 0; i < no_of_operation; i++){
            unsigned int seq_key, random_key, random_opr;
            fscanf(random_file_ptr, "%u %u %u\n", &seq_key, &random_key, &random_opr);
            int key = random_key;
            Operation opr = (Operation)random_opr;
            if(opr == Insert || opr == Update){
                void* value_ptr = generateValuePtrInitial(file_ptr);
                operationOnHashTable(&key, value_ptr, hashtable, opr);
            }
            else if(opr == Delete || opr == Read){
                operationOnHashTable(&key, NULL, hashtable, opr);
            }
        }


        other_collision = hashtable->other_collision;
        ar_opr[0] = hashtable->insert;
        ar_opr[1] = hashtable->deleted;
        ar_opr[2] = hashtable->read;
        ar_opr[3] = hashtable->update;
            
        closeHashTable(1, hashtable);
        fclose(random_file_ptr);
        fclose(file_ptr);


        HashTable* hashtable_RL = generateHashTableWithFile((char*)"./files/benchmarkHT.bin", generateKeyValuePtr, 0, 1000000, handleKeyValuePtr, generateKeyValuePtr, (char*)"./files/NewbenchmarkHT", NULL, Chaining);
        total_elements_in_file = hashtable_RL->no_element_ht;
        closeHashTable(1, hashtable_RL);
    }

    state.counters[" Handling "] = collision_handling;
    state.counters[" Init Elms "] = initial_insert_elements;
    state.counters[" Total Oprs "] = no_of_operation;
    state.counters[" Max HT size "] = table_size_limit;

    for(int i = 0; i < 4; i++){
        opr_analysis.success_opr += ar_opr[i].success_opr;
        opr_analysis.success_collision += ar_opr[i].success_collision;
        opr_analysis.unsuccess_opr += ar_opr[i].unsuccess_opr;
        opr_analysis.unsuccess_collision += ar_opr[i].unsuccess_collision;
    }
    // state.counters[" Total Elms" ] = total_elements_in_file;
    state.counters[" success "] = opr_analysis.success_opr;
    state.counters[" s_collision "] = opr_analysis.success_collision;
    state.counters[" unsuccess "] = opr_analysis.unsuccess_opr;
    state.counters[" u_collision"] = opr_analysis.unsuccess_collision;
    state.counters[" Other Collision "] = other_collision;
}

BENCHMARK(BM_RandomAccess)->ArgsProduct({{1, 2, 3, 4}, {10000}, {1000, 10000}, {1000, 100000}})->Iterations(1);

BENCHMARK_MAIN();