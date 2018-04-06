#include <stdio.h>
#include <stdlib.h>
#include <zconf.h>
#include <signal.h>

int waiting = 0;
int noChild = 1;
pid_t pid = 0;

void handleTSTP(int signum) {
    if (waiting)
        waiting = 0;
    else {
        printf("\rOczekuję na CTRL+Z - kontynuacja albo CTR+C - zakonczenie programu\n");
        sigset_t mask;
        sigfillset(&mask);
        sigdelset(&mask, SIGTSTP);
        sigdelset(&mask, SIGINT);
        waiting = 1;
        sigsuspend(&mask);
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
                    execl("./date.sh", "./date.sh", NULL);
                    exit(0);
                }
            }
        } else {
            if(!noChild){
                kill(pid, SIGKILL);
                noChild = 1;
            }
        }
    }

    return 0;
}