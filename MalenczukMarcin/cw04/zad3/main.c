#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>
#include <memory.h>

#define FAILURE_EXIT(code, format, ...) { fprintf(stderr, format, ##__VA_ARGS__); exit(code);}
#define WRITE_MSG(format, ...) { char buffer[255]; sprintf(buffer, format, ##__VA_ARGS__); write(1, buffer, strlen(buffer));}

volatile int TYPE;
volatile int receivedByChild;
volatile int receivedFromChild;
volatile pid_t child;

void childHandler(int signum, siginfo_t *info, void *context) {
    if (signum == SIGINT) {
        WRITE_MSG("Signals received by child: %d\n", receivedByChild);
        exit(SIGINT);
    }

    if (info->si_pid != getppid()) return;

    if (TYPE == 1 || TYPE == 2) {
        if (signum == SIGUSR1) {
            receivedByChild++;
            kill(getppid(), SIGUSR1);
            WRITE_MSG("Child: SIGUSR1 received and sent back\n")
        } else if (signum == SIGUSR2) {
            receivedByChild++;
            WRITE_MSG("Child: SIGUSR2 received Terminating\n")
            WRITE_MSG("Signals received by child: %d\n", receivedByChild);
            exit(SIGUSR2);
        }
    } else if (TYPE == 3) {
        if (signum == SIGRTMIN) {
            receivedByChild++;
            kill(getppid(), SIGRTMIN);
            WRITE_MSG("Child: SIGRTMIN received and sent back\n")
        } else if (signum == SIGRTMAX) {
            receivedByChild++;
            WRITE_MSG("Child: SIGRTMAX received Terminating\n")
            WRITE_MSG("Signals received by child: %d\n", receivedByChild);
            exit(SIGRTMAX);
        }
    }
}

void motherHandler(int signum, siginfo_t *info, void *context) {
    if (signum == SIGINT) {
        kill(child, SIGKILL);
        return;
    }
    if (info->si_pid != child) return;

    if ((TYPE == 1 || TYPE == 2) && signum == SIGUSR1) {
        receivedFromChild++;
        WRITE_MSG("Mother: Received SIGUSR1 form Child\n");
    } else if (TYPE == 3 && signum == SIGRTMIN) {
        receivedFromChild++;
        WRITE_MSG("Mother: Received SIGRTMIN from Child\n");
    }
}


void childProcess() {
    struct sigaction act;
    sigemptyset(&act.sa_mask);
    act.sa_flags = SA_SIGINFO;
    act.sa_sigaction = childHandler;

    if (sigaction(SIGINT, &act, NULL) == -1) FAILURE_EXIT(1, "Can't catch SIGINT\n");
    if (sigaction(SIGUSR1, &act, NULL) == -1) FAILURE_EXIT(1, "Can't catch SIGUSR1\n");
    if (sigaction(SIGUSR2, &act, NULL) == -1) FAILURE_EXIT(1, "Can't catch SIGUSR2\n");
    if (sigaction(SIGRTMIN, &act, NULL) == -1) FAILURE_EXIT(1, "Can't catch SIGRTMIN\n");
    if (sigaction(SIGRTMAX, &act, NULL) == -1) FAILURE_EXIT(1, "Can't catch SIGRTMAX\n");

    while (1) {

    }
}

void motherProcess(int L) {
    struct sigaction act;
    sigemptyset(&act.sa_mask);
    act.sa_flags = SA_SIGINFO;
    act.sa_sigaction = motherHandler;

    if (sigaction(SIGINT, &act, NULL) == -1) FAILURE_EXIT(1, "Can't catch SIGINT\n");
    if (sigaction(SIGUSR1, &act, NULL) == -1) FAILURE_EXIT(1, "Can't catch SIGUSR1\n");
    if (sigaction(SIGRTMIN, &act, NULL) == -1) FAILURE_EXIT(1, "Can't catch SIGRTMIN\n");

    if (TYPE == 1) {
        for (int i = 0; i < L; i++) {
            WRITE_MSG("Mother: Sending SIGUSR1\n");
            kill(child, SIGUSR1);
        }
        WRITE_MSG("Mother: Sending SIGUSR2\n");
        kill(child, SIGUSR2);
    } else if (TYPE == 2) {
        union sigval val;
        for (int i = 0; i < L; i++) {
            WRITE_MSG("Mother: Sending SIGUSR1\n");
            sigqueue(child, SIGUSR1, val);
        }
        WRITE_MSG("Mother: Sending SIGUSR2\n");
        sigqueue(child, SIGUSR2, val);
    } else if (TYPE == 3) {
        for (int i = 0; i < L; i++) {
            WRITE_MSG("Mother: Sending SIGRTMIN\n");
            kill(child, SIGRTMIN);
        }
        WRITE_MSG("Mother: Sending SIGRTMAX\n");
        kill(child, SIGRTMAX);
    }

    wait(NULL);
}


int main(int argc, char *argv[]) {
    if (argc < 3) FAILURE_EXIT(1, "Wrong execution. Use ./main VAL_L VAL_TYPE\n");
    int L = (int) strtol(argv[1], '\0', 10);
    TYPE = (int) strtol(argv[2], '\0', 10);

    if (L < 1) FAILURE_EXIT(1, "Wrong L Argument\n")
    if (TYPE < 1 || TYPE > 3) FAILURE_EXIT(1, "Wrong Type Argument\n")

    child = fork();
    if (!child) {
        printf("1\n");
        childProcess();
    }
    else motherProcess(L);
    printf("Signals sent: %d\n", L + 1);
    printf("Signals received from child: %d\n", receivedFromChild);
    return 0;
}
