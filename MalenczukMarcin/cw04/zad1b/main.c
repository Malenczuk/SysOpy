#include <stdio.h>
#include <stdlib.h>
#include <zconf.h>
#include <signal.h>

#define FAILURE_EXIT(code, format, ...) { fprintf(stderr, format, ##__VA_ARGS__); exit(code);}

int waiting = 0;
int noChild = 1;
pid_t pid = 0;

void handleTSTP(int signum) {
    if (waiting)
        waiting = 0;
    else {
        printf("\rOczekuję na CTRL+Z - kontynuacja albo CTR+C - zakonczenie programu\n");
        waiting = 1;
    }
}

void handleINT(int signum) {
    printf("\rOdebrano sygnał SIGINT\n");
    exit(0);
}

int main() {
    signal(SIGTSTP, &handleTSTP);

    struct sigaction act;
    act.sa_handler = handleINT;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    sigaction(SIGINT, &act, NULL);

    while (1) {
        if (!waiting) {
            if (noChild) {
                noChild = 0;
                pid = fork();
                if (!pid) {
                    execlp("./date.sh", "./date.sh", NULL);
                    FAILURE_EXIT(1,"Error while exec.\n");
                }
            }
        } else {
            if(!noChild){
                kill(pid, SIGKILL);
                noChild = 1;
            }
        }
    }
}