#include <stdio.h>
#include<unistd.h>
#include <sys/resource.h>
typedef struct rlimit Rlimit;

int main() {
    pid_t pid = getpid();
    Rlimit stackLimit, processLimit, maxFileDescLimit, newProcessLimit;
    processLimit.rlim_cur = 2782;
    prlimit(pid, RLIMIT_STACK, NULL, &stackLimit);
    prlimit(pid, RLIMIT_NPROC, &processLimit, NULL);
    prlimit(pid, RLIMIT_NOFILE, NULL, &maxFileDescLimit);
    
    printf("stack size: %ld\n", stackLimit.rlim_cur);
    printf("process limit: %ld\n", processLimit.rlim_cur);
    printf("max file descriptors: %ld\n", maxFileDescLimit.rlim_cur);
    return 0;
}
