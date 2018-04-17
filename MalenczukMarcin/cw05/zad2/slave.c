#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <time.h>
#include <signal.h>

#define FAILURE_EXIT(code, format, ...) { fprintf(stderr, format, ##__VA_ARGS__); exit(code);}
#define BUFF_SIZE 256

int main(int argc, char *argv[]) {
    srand((unsigned int) (time(NULL) * getpid()));
    FILE *date;

    if (argc < 3) FAILURE_EXIT(1, "To little Arguments.\n");

    int N = (int) strtol(argv[2], '\0', 10);

    int fifo = open(argv[1], O_WRONLY);
    if (fifo < 0) FAILURE_EXIT(1, "Error opening FIFO\n");

    printf("Slave %d: %d\n", N, getpid());

    char buffer[2][BUFF_SIZE];
    for (int i = 0; i < N; i++) {
        date = popen("date", "r");
        fgets(buffer[0], BUFF_SIZE, date);
        sprintf(buffer[1], "Slave: %d - %s", getpid(), buffer[0]);
        write(fifo, buffer[1], strlen(buffer[1]));
        fclose(date);
        sleep((unsigned int) (rand() % 4 + 2));
    }

    close(fifo);

    return 0;
}