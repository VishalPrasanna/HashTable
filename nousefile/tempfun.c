#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>



char* generateString(int s){
    char* string = malloc((s+1)*sizeof(char));
    for(int i = 0; i < s; i++){
        int num = rand() % 26 + 97;
        char c = num;
        strncpy(string+i, &c, 1);
    }
    string[s] = '\0';
    return string;
}

void main(){

    srand(time(0));

    FILE* file_ptr = fopen("../files/RandomData.txt", "w");
    perror("E");

    for(unsigned int i = 1; i <= 1000000; i++){
        unsigned int opr = rand() % 4 + 1;
        unsigned int random_key = rand() % 1000000;
        fprintf(file_ptr, "%u %u %u\n", i, random_key, opr);
    }
    fclose(file_ptr);
}
