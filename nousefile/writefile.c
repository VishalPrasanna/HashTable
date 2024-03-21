#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

typedef struct student{
    unsigned int student_id;
    char* name;
    unsigned short int age;
    float attendance;
    unsigned int phone_no;
    float cgpa;
    unsigned int register_no;
    char address[21];
}Student;


typedef struct teacher{
    unsigned int teacher_id;
    char* name;
    unsigned short int age;
    float attendance;
    unsigned int phone_no;
    float salary;
    unsigned long int account_no;
    char address[21];
}Teacher;

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
    printf("HEllo");

    FILE* file_ptr = fopen("../files/Data.txt", "w");
    perror("E");

    for(unsigned int i = 1; i <= 200000; i++){

        Student s;
        Teacher t;

        s.student_id = i;
        t.teacher_id = i;

        int name_len = rand() % 5 + 8;
        s.name = generateString(name_len);
        t.name = generateString(name_len);

        s.age = rand() % 50 + 10;
        t.age = rand() % 50 + 20;

        s.attendance = (rand() % 10000) / 100.0;
        t.attendance = (rand() % 10000) / 100.0;

        s.phone_no = rand();
        t.phone_no = rand();

        s.cgpa = rand() % 100 / 10.0;
        t.salary = rand() % 1000000 / 10.0;

        s.register_no = rand();
        t.account_no = rand();

        char* str1 = generateString(20);
        strncpy(s.address, str1, 21);
        strncpy(t.address, str1, 21);
        free(str1);
        
        fprintf(file_ptr, "%5u %5d %15s %5u %8.2f %12u %8.2f %12ld %s ",t.teacher_id, name_len, t.name, t.age, t.attendance,
                                                    t.phone_no, t.salary, t.account_no, t.address);
        fprintf(file_ptr, "%5u %5d %15s %6u %8.2f %12u %6.2f %13u %d %s \n", s.student_id, name_len, s.name, s.age, s.attendance,
                                                         s.phone_no, s.cgpa, s.register_no, 20, s.address);
        
        // fprintf(file_ptr, "%5u %15s %5u %8.2f %12u %8.2f %12ld %s ",t.teacher_id, t.name, t.age, t.attendance,
        //                                             t.phone_no, t.salary, t.account_no, t.address);
        // fprintf(file_ptr, "%5u %15s %6u %8.2f %12u %6.2f %13u %s \n", s.student_id, s.name, s.age, s.attendance,
        //                                                  s.phone_no, s.cgpa, s.register_no, s.address);
        free(s.name);
        free(t.name);
    }
    fclose(file_ptr);
}
