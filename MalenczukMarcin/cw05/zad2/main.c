#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <sys/wait.h>
#include <errno.h>

#define FAILURE_EXIT(code, format, ...) { fprintf(stderr, format, ##__VA_ARGS__); exit(code);}
volatile int slavesNotReady = 0;
volatile pid_t masterPID;
volatile int fifoReady = 0;

void rtHandler(int signo, siginfo_t *info, void *context) {
    if (signo == SIGRTMIN) {
        kill(masterPID, SIGRTMIN);
    }
    else if (signo == SIGRTMIN + 1)
        fifoReady = 1;
    else if (signo == SIGRTMIN + 2) {
        write(1, "Aborting\n", 10);
        exit(2);
    }
}

int main(int argc, char *argv[]) {
    struct sigaction act;
    sigemptyset(&act.sa_mask);
    act.sa_flags = SA_SIGINFO;
    act.sa_sigaction = rtHandler;

    if (argc < 4) FAILURE_EXIT(1, "To little Arguments.\n");
    int N = (int) strtol(argv[3], '\0', 10);
    slavesNotReady = N;

    if (sigaction(SIGRTMIN, &act, NULL) == -1) FAILURE_EXIT(1, "Main: Can't catch SIGRTMIN\n");
    if (sigaction(SIGRTMIN+1, &act, NULL) == -1) FAILURE_EXIT(1, "Main: Can't catch SIGRTMIN+1\n");
    if (sigaction(SIGRTMIN+2, &act, NULL) == -1) FAILURE_EXIT(1, "Main: Can't catch SIGRTMIN+2\n");

    printf("Main: Crating Master\n");
    pid_t masterPID = fork();
    if (masterPID < 0) FAILURE_EXIT(1, "Error forking Master process\n");
    if (!masterPID) {
        execlp("./master", "./master", argv[1], NULL);
        FAILURE_EXIT(1, "Error creating Master\n");
    }

    while (!fifoReady) pause();

    for (int i = 0; i < N; i++) {
        printf("Main: Creating %d slave", i + 1);
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