#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <memory.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/resource.h>

#define ARGS_MAX 64
#define LINE_MAX 256


void setLimits(char const *cpuArg, char const *memArg){
    long int cpuLimit = strtol(cpuArg, NULL, 10);
    long int memLimit = strtol(memArg, NULL, 10) * 1048576;

    struct rlimit cpuRLimit;
    struct rlimit memRLimit;

    cpuRLimit.rlim_max = (rlim_t) cpuLimit;
    cpuRLimit.rlim_cur = (rlim_t) cpuLimit;
    memRLimit.rlim_max = (rlim_t) memLimit;
    memRLimit.rlim_cur = (rlim_t) memLimit;

    if(setrlimit(RLIMIT_CPU, &cpuRLimit) == -1)
        printf("Unable to make cpu limit!\n");
//    if(setrlimit(RLIMIT_AS, &memRLimit) == -1)
//        printf("Unable to make memory limit!\n");
    if(setrlimit(RLIMIT_DATA, &memRLimit) == -1)
        printf("Unable to make memory limit!\n");
    if(setrlimit(RLIMIT_STACK, &memRLimit) == -1)
        printf("Unable to make memory limit!\n");
}

long int getTime(struct timeval *t) {
    return (long int)t->tv_sec * 1000000 + (long int)t->tv_usec;
}

void printfTime(struct rusage fstTime, struct rusage sndTime){
    long int sysDiff = getTime(&sndTime.ru_stime) - getTime(&fstTime.ru_stime);
    long int usrDiff = getTime(&sndTime.ru_utime) - getTime(&fstTime.ru_utime);
    printf("┌─────────────────────┐\n");
    printf("│ System: %10.6lfs │\n", (double) sysDiff / 1000000);
    printf("│ User:   %10.6lfs │\n", (double) usrDiff / 1000000);
    printf("└─────────────────────┘\n");
}

int main(int argc, char const *argv[]) {
    if(argc < 4) {
        fprintf(stderr, "To little Arguments\n");
        exit(1);
    }
    FILE* file = fopen(argv[1], "r");
    if(!file) {
        fprintf(stderr, "Error while opening file %s\n", argv[1]);
        exit(1);
    }
    char r0[LINE_MAX];
    char r1[LINE_MAX];
    char *args[ARGS_MAX];
    int argNum = 0, status = 0;
    struct rusage usage[2];
    while(fgets(r0, LINE_MAX, file)){
        strcpy(r1, r0);
        argNum = 0;
        while((args[argNum] = strtok(argNum == 0 ? r1 : NULL, " \n\t\0")) != NULL) {
            if (argNum >= ARGS_MAX)
                fprintf(stderr, "Command exceeds maximum number (%d) of arguments: %s\n", ARGS_MAX - 2, r0);
            argNum ++;
        }
        if(argNum == 0) continue;

        getrusage(RUSAGE_CHILDREN, &usage[0]);

        pid_t pid = fork();
        if(!pid) {
            setLimits(argv[2], argv[3]);
            execvp(args[0], args);
            printf("Command: %s \nNot found\n", r1);
            exit(1);
        }

        wait(&status);
        if(status) {
            fprintf(stderr, "Error while running command: %swith exit code: %d\n", r0, status);
            break;
        }

        getrusage(RUSAGE_CHILDREN, &usage[1]);

        printfTime(usage[0], usage[1]);
    }
    fclose(file);
    return 0;
}