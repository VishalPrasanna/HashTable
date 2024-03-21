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


Pair* generateKeyValuePtrWithTxt(size_t key_size, FILE* file_ptr){

    Teacher* teacher_value_ptr = (Teacher*)calloc(1, sizeof(Teacher));
    size_t total_dt_size = 0, stored_value_size = 0;
    // fread(&stored_value_size, sizeof(size_t), 1, file_ptr);
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
    char* t_name_sure_ptr = (char*)realloc(unsure_ptr, indexing_sure_ptr);
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
    char* s_name_sure_ptr = (char*)realloc(unsure_ptr, indexing_sure_ptr);
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
    char* s_address_sure_ptr = (char*)realloc(unsure_ptr, indexing_sure_ptr);
    teacher_value_ptr->name = t_name_sure_ptr;
    teacher_value_ptr->s.name = s_name_sure_ptr;
    teacher_value_ptr->s.address = s_address_sure_ptr;

    Pair* pair = (Pair*)calloc(1, sizeof(Pair));
    pair->key = key;
    pair->value_ptr = teacher_value_ptr;
    return pair;
}


void handleKeyValuePtrWithTxt(void* key, void* value_ptr, Action_On_Key_Value action_on_key_value, HashTable* hashtable){

    void *teacher_key = key;
    Teacher *teacher_value_ptr = (Teacher*)value_ptr;

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
        teacher_value_ptr->phone_no, teacher_value_ptr->salary, teacher_value_ptr->account_no, teacher_value_ptr->address, 0,
        teacher_value_ptr->s.student_id, teacher_value_ptr->s.name, 0,  teacher_value_ptr->s.age, teacher_value_ptr->s.attendance, teacher_value_ptr->s.phone_no, teacher_value_ptr->s.cgpa, teacher_value_ptr->s.register_no, teacher_value_ptr->s.address, 0);
            
        fseek(file_ptr, -(temp + stored_value_size), SEEK_CUR);
        fprintf(file_ptr, "%20zu", stored_value_size);
        fseek(file_ptr, 0, SEEK_END);
    }


    if(action_on_key_value == FreeMemory || action_on_key_value == SaveToFileAndFreeMemory){
        free(teacher_value_ptr->name);
        free(teacher_value_ptr->s.name);
        free(teacher_value_ptr->s.address);
        free(teacher_value_ptr);
        value_ptr = NULL;
    }
}




using namespace std;

// Define another benchmark
static void BM_RandomAccess(benchmark::State& state) {

    CollisionHandling collision_handling = (CollisionHandling)state.range(0);
    unsigned int no_of_operation = state.range(1);
    unsigned int  table_size_limit = state.range(2);

    Collision ar_opr[4], opr_analysis = {0, 0, 0, 0};
    unsigned int other_collision = 0;

    for (auto _ : state){

        FILE* random_file_ptr = fopen("./files/RandomData.txt", "r");
        FILE* file_ptr = fopen("./files/Data.txt", "r");

        HashTable* hashtable = setupHashTable(0, table_size_limit, sizeof(unsigned int), handleKeyValuePtrWithTxt, generateKeyValuePtrWithTxt, (char*)"./files/benchmarkelements", NULL, collision_handling, 'T');
    
        for(unsigned int i = 0; i < no_of_operation; i++){
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

        ar_opr[0] = hashtable->insert;
        ar_opr[1] = hashtable->deleted;
        ar_opr[2] = hashtable->read;
        ar_opr[3] = hashtable->update;

        other_collision = hashtable->other_collision;

        
        closeHashTable(1, hashtable);
        fclose(random_file_ptr);
        fclose(file_ptr);
    }

    state.counters[" Handling "] = collision_handling;
    state.counters[" Total Operation "] = no_of_operation;
    state.counters[" Max HT size "] = table_size_limit;

    for(int i = 0; i < 4; i++){
        opr_analysis.success_opr += ar_opr[i].success_opr;
        opr_analysis.success_collision += ar_opr[i].success_collision;
        opr_analysis.unsuccess_opr += ar_opr[i].unsuccess_opr;
        opr_analysis.unsuccess_collision += ar_opr[i].unsuccess_collision;
    }
    state.counters[" success "] = opr_analysis.success_opr;
    state.counters[" s_collision "] = opr_analysis.success_collision;
    state.counters[" unsuccess "] = opr_analysis.unsuccess_opr;
    state.counters[" u_collision "] = opr_analysis.unsuccess_collision;
    state.counters[" Other Collision "] = other_collision;
}

BENCHMARK(BM_RandomAccess)->ArgsProduct({{1, 2, 3, 4}, {10000}, {10000}})->Iterations(5);

BENCHMARK_MAIN();