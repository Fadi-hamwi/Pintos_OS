#ifndef ARGS
    #define ARGS
    #include "my_helper_args.h"
#endif 
#include"my_helper_function.h"

char *my_helper_function(helper_args_t *args) {
    int i;
    for(i = 0; args->string[i] != '\0'; i++) {
        if(args->string[i] == '/') {
            return &args->string[i+1];
        }
    }
    return args->string;
}