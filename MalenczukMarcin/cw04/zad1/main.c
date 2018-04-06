#include <stdio.h>
#include <stdlib.h>
#include <zconf.h>
#include <signal.h>

int waiting = 0;

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
void handleINT(int signum){
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

    while(1) {
        system("date");
        sleep(1);
    }

    return 0;
}