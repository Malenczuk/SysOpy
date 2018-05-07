#define _GNU_SOURCE

#include "hairdressers.h"

void intHandler(int signo);

void clearResources(void);

void prepareFifo(int chNum);

void prepareSemaphores();

void doBarberWork();

void cut(pid_t pid);

pid_t takeChair();

Fifo *fifo = NULL;
sem_t *BARBER;
sem_t *FIFO;
sem_t *CHECKER;

int main(int argc, char *argv[]) {
    if (argc != 2) FAILURE_EXIT(3, "Barber: specify chair Number!");
    int chairNum = (int) strtol(argv[1], '\0', 10);
    if (chairNum < 1 || chairNum > 1024) FAILURE_EXIT(3, "Barber: Wrong number of Chairs!");

    if (atexit(clearResources) == -1) FAILURE_EXIT(3, "Barber: setting atexit failed!");
    if (signal(SIGINT, intHandler) == SIG_ERR) FAILURE_EXIT(3, "Barber: setting SIGINT handler failed!");

    prepareFifo(chairNum);
    prepareSemaphores();
    doBarberWork();

    return 0;
}

void doBarberWork() {
    while (1) {
        if (sem_wait(BARBER) == -1) FAILURE_EXIT(3, "Barber: taking BARBER semaphore failed!");
        if (sem_post(BARBER) == -1) FAILURE_EXIT(3, "Barber: setting himself as awaken failed!");

        pid_t toCut = takeChair();
        cut(toCut);

        while (1) {
            if (sem_wait(FIFO) == -1) FAILURE_EXIT(3, "Barber: taking FIFO semaphore failed!");
            toCut = popFifo(fifo);

            if (toCut != -1) {
                if (sem_post(FIFO) == -1) FAILURE_EXIT(3, "Barber: releasing FIFO semaphore failed!");
                cut(toCut);
            } else {
                printf("Time: %ld, Barber: going to sleep...\n", getMicroTime());
                fflush(stdout);

                if (sem_wait(BARBER) == -1) FAILURE_EXIT(3, "Barber: taking BARBER semaphore failed!");

                if (sem_post(FIFO) == -1) FAILURE_EXIT(3, "Barber: releasing FIFO semaphore failed!");
                break;
            }
        }
    }
}

pid_t takeChair() {
    if (sem_wait(FIFO) == -1) FAILURE_EXIT(3, "Barber: taking FIFO semaphore failed!");

    pid_t toCut = fifo->chair;

    if (sem_post(FIFO) == -1) FAILURE_EXIT(3, "Barber: releasing FIFO semaphore failed!");

    return toCut;
}

void cut(pid_t pid) {
    printf("Time: %ld, Barber: preparing to cut %d\n", getMicroTime(), pid);
    fflush(stdout);

    kill(pid, SIGRTMIN);

    printf("Time: %ld, Barber: finished cutting %d\n", getMicroTime(), pid);
    fflush(stdout);
}

void prepareFifo(int chNum) {
    int shmID = shm_open(shmPath, O_CREAT | O_EXCL | O_RDWR, 0666);
    if (shmID == -1) FAILURE_EXIT(3, "Barber: creating shared memory failed!");

    if (ftruncate(shmID, sizeof(Fifo)) == -1) FAILURE_EXIT(3, "Barber: truncating shm failed!");

    void *tmp = mmap(NULL, sizeof(Fifo), PROT_READ | PROT_WRITE, MAP_SHARED, shmID, 0);
    if (tmp == (void *) (-1)) FAILURE_EXIT(3, "Barber: attaching shm failed!");
    fifo = (Fifo *) tmp;

    fifoInit(fifo, chNum);
}

void prepareSemaphores() {
    BARBER = sem_open(barberPath, O_CREAT | O_EXCL | O_RDWR, 0666, 0);
    if (BARBER == SEM_FAILED) FAILURE_EXIT(3, "Barber: creating BARBER semaphore failed!");

    FIFO = sem_open(fifoPath, O_CREAT | O_EXCL | O_RDWR, 0666, 1);
    if (FIFO == SEM_FAILED) FAILURE_EXIT(3, "Barber: creating FIFO semaphore failed!");

    CHECKER = sem_open(checkerPath, O_CREAT | O_EXCL | O_RDWR, 0666, 1);
    if (CHECKER == SEM_FAILED) FAILURE_EXIT(3, "Barber: creating CHECKER semaphore failed!");
}

void clearResources(void) {
    if (munmap(fifo, sizeof(fifo)) == -1) printf("Barber: detaching FIFO shm failed!\n");
    else printf("Barber: detached FIFO shm!\n");

    if (shm_unlink(shmPath) == -1) printf("Barber: deleting FIFO shm failed!\n");
    else printf("Barber: deleted FIFO shm!\n");

    if (sem_close(BARBER) == -1) printf("Barber: closing semaphores failed!");
    if (sem_unlink(barberPath) == -1) printf("Barber: deleting semaphores failed!");

    if (sem_close(FIFO) == -1) printf("Barber: closing semaphores failed!");
    if (sem_unlink(fifoPath) == -1) printf("Barber: deleting semaphores failed!");

    if (sem_close(CHECKER) == -1) printf("Barber: closing semaphores failed!");
    if (sem_unlink(checkerPath) == -1) printf("Barber: deleting semaphores failed!");
}

void intHandler(int signo) {
    exit(2);
}