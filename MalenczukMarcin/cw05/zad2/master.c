#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <signal.h>

#define FAILURE_EXIT(code, format, ...) { fprintf(stderr, format, ##__VA_ARGS__); exit(code);}
#define BUFF_SIZE 256

int allReady = 0;

void rtHandler(int signo, siginfo_t *info, void *context) {
    if (signo == SIGRTMIN) allReady = 0;
}

int main(int argc, char *argv[]) {
    struct sigaction act;
    sigemptyset(&act.sa_mask);
    act.sa_flags = SA_SIGINFO;
    act.sa_sigaction = rtHandler;

    if (argc < 2) {
        kill(getppid(), SIGRTMIN + 2);
        FAILURE_EXIT(1, "Master: To little Arguments.\n");
    }
    if (sigaction(SIGRTMIN, &act, NULL) == -1) {
        kill(getppid(), SIGRTMIN + 2);
        FAILURE_EXIT(1, "Master: Can't catch SIGRTMIN\n");
    }

    if (mkfifo(argv[1], S_IRUSR | S_IWUSR) == -1) {
        kill(getppid(), SIGRTMIN + 2);
        FAILURE_EXIT(1, "Master: Error creating FIFO\n");
    }
    FILE *fifo = fopen(argv[1], "r");
    if (!fifo) {
        kill(getppid(), SIGRTMIN + 2);
        FAILURE_EXIT(1, "Master: Error opening FIFO\n")
    }
    kill(getppid(), SIGRTMIN + 1);

    while (!allReady) pause();

    char buffer[BUFF_SIZE];
    while (fgets(buffer, BUFF_SIZE, fifo) != NULL) {
        write(1, buffer, strlen(buffer));
    }
    printf("Master: Ended reading FIFO\n");
    fclose(fifo);
    if (remove(argv[1])) {
        kill(getppid(), SIGRTMIN + 2);
        FAILURE_EXIT(1, "Master: Error deleting FIFO\n");
    }
    return 0;
}