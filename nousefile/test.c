#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bloom_filter.h"
#include <clhash.h>

unsigned long int generateClhash(void* key, size_t size, int seed){
    void * random =  get_random_key_for_clhash(UINT64_C(0x23a23cf5033c3c81) * (uint64_t) seed,UINT64_C(0xb3816f6a2c68e530) * (uint64_t)seed);
    uint64_t hashvalue = clhash(random, key, size);
    free(random);
    return hashvalue;
}   

uint64_t* hash_func(int num_hashes, void* str, size_t str_len){

    uint64_t * results = (uint64_t*)calloc(sizeof(uint64_t), num_hashes); // using calloc because all bits should be set to null / 0 
    for(int i = 0; i < num_hashes; ++i){
        results[i] = generateClhash(str, str_len, i+1); 
    }
    return results; 
}



void main(){
    
    bloom_filter* bf = calloc(1, sizeof(bloom_filter));

    bloom_filter_init(bf, 100000, .05, hash_func);

    int i = 0;

    int i1 =11 ;

    for( i = 0; i < 100126; i++){
        if( add_value_bloom_filter(bf, &i, 4) == 1){
            // printf("Done\n");
        }
    }

    if( is_present_bloom_filter(bf, &i1, 4) == 1){
        // printf("YES\n");
    }
    else{
        // printf("No\n");
    }
    // printf("%d %d ", i, i1);

   
    // free_bloom_filter(bf);
    // reset_bloom_filter(bf);
    stats_bloom_filter(bf);
    free(bf);
} 