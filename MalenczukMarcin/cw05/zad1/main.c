#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>

#define FAILURE_EXIT(code, format, ...) { fprintf(stderr, format, ##__VA_ARGS__); exit(code);}
#define ARGS_MAX 64
#define CMDS_MAX 64
#define LINE_MAX 256
int pipes[2][2];

int main(int argc, char *argv[]) {
    if (argc < 2) FAILURE_EXIT(1, "To little Arguments.\n");
    FILE *file = fopen(argv[1], "r");
    if (!file) FAILURE_EXIT(1, "Error while opening file %s\n", argv[1]);

    char r0[LINE_MAX], r1[LINE_MAX];
    char *args[ARGS_MAX], *cmds[CMDS_MAX];
    int cmdNum = 0, argNum = 0, lineNum = 0;

    while (fgets(r0, LINE_MAX, file)) {
        lineNum++;
        strcpy(r1, r0);
        cmdNum = 0;
        while ((cmds[cmdNum] = strtok(cmdNum == 0 ? r1 : NULL, "|\n")) != NULL)
            if (++cmdNum >= CMDS_MAX) FAILURE_EXIT(1, "Line %d exceeds maximum number (%d) of pipes", lineNum, CMDS_MAX)
        if (!cmdNum) continue;

        int k;
        for (k = 0; k < cmdNum; k++) {
            argNum = 0;
            while ((args[argNum] = strtok(argNum == 0 ? cmds[k] : NULL, " \t\n")) != NULL)
                if (++argNum >= ARGS_MAX) FAILURE_EXIT(1, "Command %d exceeds maximum number (%d) of args", k, ARGS_MAX)
            if (!argNum) continue;

            if (k > 1) {
                close(pipes[k % 2][0]);
                close(pipes[k % 2][1]);
            }
            if (pipe(pipes[k % 2]))
                printf("Couldn't pipe at number %d!\n", k);

            pid_t cp = fork();
            if (cp == -1) FAILURE_EXIT(1, "Couldnt fork child process %d!\n", k);
            if (cp == 0) {
                if (k < cmdNum - 1) {
                    close(pipes[k % 2][0]);
                    if (dup2(pipes[k % 2][1], 1) < 0) FAILURE_EXIT(3, "Couldnt set writing at number %d!\n", k);
                    wait(NULL);
                }
                if (k != 0) {
                    close(pipes[(k + 1) % 2][1]);
                    if (dup2(pipes[(k + 1) % 2][0], 0) < 0) FAILURE_EXIT(3, "Couldnt set reading at number %d!\n", k);
                }
                execvp(args[0], args);
                FAILURE_EXIT(1, "ERROR EXECUTING CHILD PROCESS %d!\n", k);
            }
        }
        close(pipes[k % 2][0]);
        close(pipes[k % 2][1]);
        while (wait(NULL)) {
            if (errno == ECHILD) {
                printf("\n");
                break;
            }
        }
    }
    fclose(file);
    return 0;
}