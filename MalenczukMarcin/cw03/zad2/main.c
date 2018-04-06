#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <memory.h>


#define ARGS_MAX 64
#define LINE_MAX 256

int main(int argc, char const *argv[]) {
    if(argc < 2) {
        fprintf(stderr, "To little Arguments.\n");
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
    while(fgets(r0, LINE_MAX, file)){
        strcpy(r1, r0);
        argNum = 0;
        while((args[argNum] = strtok(argNum == 0 ? r1 : NULL, " \n\t\0")) != NULL) {
            if (argNum >= ARGS_MAX)
                fprintf(stderr, "Command exceeds maximum number (%d) of arguments: %s\n", ARGS_MAX - 2, r0);
            argNum ++;
        }
        if(argNum == 0) continue;

        pid_t pid = fork();
        if(!pid) {
            execvp(args[0], args);
            printf("Command: %s \nNot found\n", r1);
            exit(1);
        }

        wait(&status);
        if(status) {
            fprintf(stderr, "Error while running command: %swith exit code: %d\n", r0, status);
            break;
        }
    }
    fclose(file);
    return 0;
}