#include <stdio.h>
#include <string.h>  
#include <inttypes.h>  
#include <stdlib.h>
#include <math.h> 
#include "bloom_filter.h"  

#define LENGTH 8 
#define LOG_TWO 0.6931471805599453
#define LOG_TWO_SQUARED 0.4804530139182

static uint64_t * hashing_by_default(int num_hashes, void * value, size_t value_len); 
static uint64_t fnv_1a(const char * key);  
static void calculate_k_optimal(bloom_filter * bloomFilter); 

static uint64_t * hashing_by_default(int num_hashes, void * value, size_t value_len){
    // uint64_t * results = (uint64_t*)calloc(sizeof(uint64_t), num_hashes); // using calloc because all bits should be set to null / 0 
    // char key[17] = {0};  
    // results[0] = fnv_1a(value);
    // for(int i = 1; i < num_hashes; ++i){
    //     sprintf(key,"%" PRIx64 "", results[i-1]); 
    //     results[i] = fnv_1a(key); 
    // }
    // return results;
    return NULL; 
}

// all okay 
static uint64_t fnv_1a(const char * key) 
{
    int length = strlen(key); 
    uint64_t fnv_offset =  14695981039346656073ULL;  
    for(int i = 0; i < length; ++i){
        fnv_offset = fnv_offset ^ (unsigned char)key[i];
        fnv_offset = fnv_offset * 1099511628211ULL; 
    }
    return fnv_offset;
}


static void calculate_k_optimal(bloom_filter * bloomFilter){

    long n = bloomFilter->approximate_elements; 
    float p = bloomFilter->false_positivity; 
    uint64_t m = ceil((-n * log(p)) / LOG_TWO_SQUARED);
    unsigned int k = round(LOG_TWO * m  /n);
    bloomFilter->number_of_hashing_func = k; 
    bloomFilter->size = m; 
    long num_of_char_for_bits = ceil(m /(LENGTH * 1.0));
    bloomFilter->bloom_length = num_of_char_for_bits; 
}


int bloom_filter_init(bloom_filter * bloomFilter, uint64_t approximate_elements, float false_positivity, bloom_hash hash_func)
{
    if(approximate_elements > 0 && approximate_elements < UINT64_MAX && false_positivity > 0.0 && false_positivity < 1.0)
    {
        bloomFilter->approximate_elements = approximate_elements; 
        bloomFilter->false_positivity = false_positivity;
        calculate_k_optimal(bloomFilter); 
        bloomFilter->number_of_elements_added = 0; 
        bloomFilter->bloom_filter_vector = (unsigned char *)calloc(bloomFilter->bloom_length + 1,sizeof(char)); 
        bloom_filter_set_hash_function(bloomFilter, hash_func); 
        reset_bloom_filter(bloomFilter);
        return BLOOM_SUCCESS;
    }
    return BLOOM_FAILURE;

}

// all okay 
void bloom_filter_set_hash_function(bloom_filter * bloomFilter, bloom_hash hashing_func)
{
    if(hashing_func == NULL){
        bloomFilter->hashing_function = hashing_by_default;
    }else{
        bloomFilter->hashing_function = hashing_func; 
    }

}


// 7  // all okay 
int add_value_bloom_filter(bloom_filter * bloomFilter, void* value, size_t value_len){

    uint64_t * hash_array = bloom_filter_calculate_hashes(bloomFilter,value, bloomFilter->number_of_hashing_func, value_len); 
    for(unsigned int i = 0; i < bloomFilter->number_of_hashing_func; ++i){

        unsigned long index = (hash_array[i] % bloomFilter->size)/8;
        int bit_at_index = (hash_array[i] % bloomFilter->size)%8; 
        bloomFilter->bloom_filter_vector[index] |= (1 << bit_at_index);  

    }
    free(hash_array);
    bloomFilter->number_of_elements_added++; 
    return BLOOM_SUCCESS; 
}


// 6 all okay 
int is_present_bloom_filter(bloom_filter * bloomFilter,  void * value, size_t value_len)
{
    uint64_t * hash_array = bloom_filter_calculate_hashes(bloomFilter, value, bloomFilter->number_of_hashing_func, value_len);

    int res = BLOOM_SUCCESS; 
    for(unsigned int i = 0; i < bloomFilter->number_of_hashing_func; ++i){ 
       int isSet = bloomFilter->bloom_filter_vector[(hash_array[i] % bloomFilter->size)/8] & (1 << (hash_array[i] % bloomFilter->size)%8); 
        // printf("is Set = > %d",isSet);
        if(isSet == 0){
            res = BLOOM_FAILURE;
            break;
        }
    }
    free(hash_array);
    return res; 
}

// 5 
uint64_t * bloom_filter_calculate_hashes(bloom_filter * bloomFilter, void* value, unsigned int num_hashes, size_t value_len)
{
    return bloomFilter->hashing_function(num_hashes, value, value_len); 
}

// all okay 
void stats_bloom_filter(bloom_filter * bloomFilter)
{
    // for(unsigned long int i = 0; i < bloomFilter->bloom_length; ++i){
    //     printf("%c",bloomFilter->bloom_filter_vector[i]);
    // }
    printf("\n");
    float curr;
    printf("\n\n");
    printf("\x1B[36m+---------------------Bloom Filter Health--------------------+\x1B[0m\n"); 
    printf(" #\x1B[34m Number of bits alloted       : \t\t\x1B[32m%" PRIu64 " bits ~ %" PRIu64 " bytes \n \x1B[0m",bloomFilter->size,(int64_t)bloomFilter->bloom_length);
    printf("#\x1B[34m Approximate elements alloted : \t\t\x1B[32m%" PRIu64 "\n \x1B[0m",bloomFilter->approximate_elements);
    printf("#\x1B[34m Number of hash function (K)  : \t\t\x1B[32m%d\n \x1B[0m", bloomFilter->number_of_hashing_func);
    printf("#\x1B[34m Max false positive rate      : \t\t\x1B[32m%3f\n \x1B[0m",bloomFilter->false_positivity);
    printf("#\x1B[34m Bloom filter vector length   : \t\t\x1B[32m%ld\n \x1B[0m",bloomFilter->bloom_length); 
    printf("#\x1B[34m Number of elements           : \t\t\x1B[32m%" PRIu64 "\n \x1B[0m",bloomFilter->number_of_elements_added);
    printf("#\x1B[34m Estimated Elements added     : \t\t\x1B[32m%" PRIu64 "\n \x1B[0m",estimating_elements(bloomFilter)); 
    printf("#\x1B[34m Current-False Positivity Rate: \t\t\x1B[32m%3f\n \x1B[0m",curr = get_current_positivity_rate(bloomFilter));
    printf("#\x1B[34m Number of set bits in array  : \t\t\x1B[32m%" PRIu64 "\n \x1B[0m",count_bits_set(bloomFilter)); 
    if(curr*100 < 100*(bloomFilter->false_positivity)){
    printf("#\x1B[34m Health Status                : \t\t\x1B[36mHealthy\n\x1B[0m");
    }else if(curr*100 >= 100*(bloomFilter->false_positivity) && curr*100 <= (bloomFilter->false_positivity * 100 + (bloomFilter->false_positivity/2) * 100)){
    printf("#\x1B[34m Health Status                : \t\t\x1B[33mAverage\n\x1B[0m");
    }
    else if(curr*100 > (bloomFilter->false_positivity*3/2)*100){
    printf("#\x1B[34m Health Status                : \t\t\x1B[31mPoor\n\x1B[0m");
    }
    printf("+\x1B[36m-------------------------------------------------------------+\n\n\n\x1B[0m");
}

// 4  all okay 
float get_current_positivity_rate(bloom_filter * bloomFilter){
    int product = bloomFilter->number_of_hashing_func * bloomFilter->number_of_elements_added; 
    double power = -product / (float) bloomFilter->size; 
    double e_power_value = exp(power);
    return pow((1-e_power_value), bloomFilter->number_of_hashing_func);
}


// all okay 
int reset_bloom_filter(bloom_filter * bloomFilter)
{
    for(unsigned long  i = 0; i < bloomFilter->bloom_length; ++i){
        bloomFilter->bloom_filter_vector[i] = 0;
    }
    bloomFilter->number_of_elements_added = 0;
    return BLOOM_SUCCESS;  
}


// 3 
static uint64_t getSetCount(unsigned char s){
    uint64_t count = 0; 
    int i = 0;
    while((1 << i) < 255){
        if(s & (1 << i)){
          count++;
        }
        i++;
    }
    return count;
}

// 2 
uint64_t count_bits_set(bloom_filter * bloomFilter){
    uint64_t count = 0;
    for(int i = 0; i < bloomFilter->bloom_length ; ++i){
        count +=  getSetCount(bloomFilter->bloom_filter_vector[i]);
    }
    return count;
}

// 1 
uint64_t estimating_elements(bloom_filter * bloomFilter){

    uint64_t m = bloomFilter->size;
    uint64_t count = 0;
    for(unsigned long i = 0; i < bloomFilter->bloom_length; ++i){
        count +=  getSetCount(bloomFilter->bloom_filter_vector[i]);
    }
    uint64_t x = count;
    unsigned int k = bloomFilter->number_of_hashing_func; 

    double log_n = log(1 - ((double) x / (double) m));
    return (uint64_t)-(((double) m / k) * log_n);
}


int free_bloom_filter(bloom_filter * bloomFilter){
    free(bloomFilter->bloom_filter_vector);
    bloomFilter->bloom_filter_vector = NULL; 
    bloomFilter->bloom_length = 0;
    bloomFilter->approximate_elements = 0; 
    bloomFilter->false_positivity = 0; 
    bloomFilter->hashing_function = NULL; 
    bloomFilter->number_of_elements_added = 0; 
    bloomFilter->size = 0; 
    bloomFilter->number_of_hashing_func = 0;
    return BLOOM_SUCCESS;
}

bloom_filter * union_bloom_filters(bloom_filter * source1, bloom_filter * source2){

    if(source1->size == source2->size)
    {
        bloom_filter * b1u2 = (bloom_filter *)malloc(sizeof(bloom_filter)); 
        b1u2->size = source1->size; 
        b1u2->bloom_length = source1->bloom_length;        
        b1u2->bloom_filter_vector = (unsigned char *)calloc(source1->bloom_length + 1, sizeof(unsigned char));
        b1u2->false_positivity = source1->false_positivity;
        for(unsigned long i = 0; i < source1->bloom_length; ++i){
            b1u2->bloom_filter_vector[i] = source1->bloom_filter_vector[i] | source2->bloom_filter_vector[i];  
        }
        b1u2->approximate_elements = source1->approximate_elements;
        b1u2->hashing_function = source1->hashing_function;
        b1u2->number_of_hashing_func = source1->number_of_hashing_func;
        b1u2->number_of_elements_added = estimating_elements(b1u2);
        return b1u2;
    }
    return NULL;
}  



bloom_filter * intersection_bloom_filters(bloom_filter * source1, bloom_filter * source2){

     if(source1->size == source2->size)
    {
        bloom_filter * b1u2 = (bloom_filter *)malloc(sizeof(bloom_filter)); 
        b1u2->size = source1->size; 
        b1u2->bloom_length = source1->bloom_length;        
        b1u2->bloom_filter_vector = (unsigned char *)calloc(source1->bloom_length + 1, sizeof(unsigned char));
        b1u2->false_positivity = source1->false_positivity;
        for(unsigned long i = 0; i < source1->bloom_length; ++i){
            b1u2->bloom_filter_vector[i] = source1->bloom_filter_vector[i] & source2->bloom_filter_vector[i];  
        }
        b1u2->approximate_elements = source1->approximate_elements;
        b1u2->hashing_function = source1->hashing_function;
        b1u2->number_of_hashing_func = source1->number_of_hashing_func;
        b1u2->number_of_elements_added = estimating_elements(b1u2);
        return b1u2;
    }
    return NULL;
} 


long double jaccard_index_bloom_filters(bloom_filter * source1, bloom_filter * source2){
    bloom_filter * union_of_two = union_bloom_filters(source1, source2); 
    bloom_filter * intersection_of_two = intersection_bloom_filters(source1, source2); 
    // printf("estimated number of elements in intersection = %"PRIu64", union = %"PRIu64" \n",intersection_of_two->number_of_elements_added, union_of_two->number_of_elements_added);
    long double result = (long double)(intersection_of_two->number_of_elements_added)/(long double)(union_of_two->number_of_elements_added);
    free(union_of_two);
    free(intersection_of_two); 
    return result;
}