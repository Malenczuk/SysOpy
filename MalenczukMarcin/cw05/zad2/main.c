#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <sys/wait.h>
#include <errno.h>

#define FAILURE_EXIT(code, format, ...) { fprintf(stderr, format, ##__VA_ARGS__); exit(code);}
volatile pid_t masterPID;

int main(int argc, char *argv[]) {

    if (argc < 4) FAILURE_EXIT(1, "To little Arguments.\n");
    int N = (int) strtol(argv[3], '\0', 10);

    printf("Main: Crating Master\n");
    pid_t masterPID = fork();
    if (masterPID < 0) FAILURE_EXIT(1, "Error forking Master process\n");
    if (!masterPID) {
        execlp("./master", "./master", argv[1], NULL);
        FAILURE_EXIT(1, "Error creating Master\n");
    }

    sleep(1);

    for (int i = 0; i < N; i++) {
        pid_t slave = fork();
        if (slave < 0) FAILURE_EXIT(1, "Error forking Slave process");
        if (!slave) {
            execlp("./slave", "./slave", argv[1], argv[2], NULL);
            FAILURE_EXIT(1, "Error creating Slave\n")
        }
    }

    while (wait(NULL))
        if (errno == ECHILD) {
            printf("\n");
            break;
        }

    return 0;
}