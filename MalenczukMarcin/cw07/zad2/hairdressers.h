#ifndef HAIRDRESSERS_H
#define HAIRDRESSERS_H

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <ctype.h>
#include <time.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/stat.h>

#define FAILURE_EXIT(code, format, ...) { printf(format, ##__VA_ARGS__); exit(code);}
#define PROJECT_ID 69

const char *shmPath;
const char *barberPath;
const char *fifoPath;
const char *checkerPath;

typedef struct Fifo {
    int max;
    int head;
    int tail;
    pid_t chair;
    pid_t queue[1024];
} Fifo;

void fifoInit(Fifo *fifo, int cn);

pid_t popFifo(Fifo *fifo);

int pushFifo(Fifo *fifo, pid_t x);

long getMicroTime();

#endif
