#include<stdio.h>
#ifndef ARGS
    #define ARGS 
    #include"my_helper_args.h"
#endif
#include"my_helper_function.h"


int main(int argc, char **argv) {
    helper_args_t helper_args;
    helper_args.string = argv[0];
    helper_args.target = '/';
    char *result = my_helper_function(&helper_args);
    printf("%s\n", result);
    return 0;
}