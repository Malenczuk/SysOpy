#include "hairdressers.h"

const char *shmPath = "/shm";
const char *barberPath = "/barber";
const char *fifoPath = "/fifo";
const char *checkerPath = "/checker";

void fifoInit(Fifo *fifo, int cn) {
    fifo->max = cn;
    fifo->head = -1;
    fifo->tail = 0;
    fifo->chair = 0;
}

int isEmptyFifo(Fifo *fifo) {
    if (fifo->head == -1) return 1;
    else return 0;
}

int isFullFifo(Fifo *fifo) {
    if (fifo->head == fifo->tail) return 1;
    else return 0;
}

pid_t popFifo(Fifo *fifo) {
    if (isEmptyFifo(fifo) == 1) return -1;

    fifo->chair = fifo->queue[fifo->head++];
    if (fifo->head == fifo->max) fifo->head = 0;

    if (fifo->head == fifo->tail) fifo->head = -1;

    return fifo->chair;
}

int pushFifo(Fifo *fifo, pid_t x) {
    if (isFullFifo(fifo) == 1) return -1;
    if (isEmptyFifo(fifo) == 1)
        fifo->head = fifo->tail;

    fifo->queue[fifo->tail++] = x;
    if (fifo->tail == fifo->max) fifo->tail = 0;
    return 0;
}

long getMicroTime() {
    struct timespec marker;
    if (clock_gettime(CLOCK_MONOTONIC, &marker) == -1) FAILURE_EXIT(3, "Getting time failed!");
    return marker.tv_nsec / 1000;
}
