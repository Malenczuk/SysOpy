#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <memory.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/resource.h>

#define ARGS_MAX 64
#define LINE_MAX 256

void setLimits(char *cpuArg, char *memArg){
    long int cpuLimit = strtol(cpuArg, NULL, 10);
    long int memLimit = strtol(memArg, NULL, 10) * 1048576;

    struct rlimit cpuRLimit;
    struct rlimit memRLimit;

    cpuRLimit.rlim_max = (rlim_t) cpuLimit;
    cpuRLimit.rlim_cur = (rlim_t) cpuLimit;
    memRLimit.rlim_max = (rlim_t) memLimit;
    memRLimit.rlim_cur = (rlim_t) memLimit;

    if(setrlimit(RLIMIT_CPU, &cpuRLimit) == -1){
        printf("Unable to make cpu limit!\n");
    }
    if(setrlimit(RLIMIT_DATA, &memRLimit) == -1){
        printf("Unable to make memory limit!\n");
    }
    if(setrlimit(RLIMIT_STACK, &memRLimit) == -1){
        printf("Unable to make memory limit!\n");
    }

}
int main(int argc, char const *argv[]) {
    if(argc < 4) exit(1);
    FILE* file = fopen(argv[1], "r");
    if(!file) exit(1);
    char r0[LINE_MAX];
    char *args[ARGS_MAX];
    int argNum = 0;
    while(fgets(r0, LINE_MAX, file)){
        argNum = 0;
        while((args[argNum++] = strtok(argNum == 0 ? r0 : NULL, " \n\t")) != NULL){
            if(argNum + 1 >= ARGS_MAX){
                fprintf(stderr, "Command exceeds maximum number (%d) of arguments: %s\nWith arguments:", ARGS_MAX , args[0]);
                for(int i = 1; i < argNum; i++) fprintf(stderr, " %s", args[i]);
                fprintf(stderr, "\n");
            }
        };
        pid_t pid = vfork();
        if(!pid) {
            execvp(args[0], args);
        }
        int status;
        wait(&status);
        if(status){
            fprintf(stderr, "Error while running command: %s\nWith arguments:", args[0]);
            for(int i = 1; i < argNum; i++) fprintf(stderr, " %s", args[i]);
            fprintf(stderr, "\n");
        }
    }
    fclose(file);
    return 0;
}