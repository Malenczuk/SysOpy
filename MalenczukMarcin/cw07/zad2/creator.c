#define _GNU_SOURCE

#include "hairdressers.h"

void rtminHandler(int);

void intHandler(int);

void prepareFifo();

void prepareSemaphores();

void freeResources(void);

int takePlace();

void getCut(int ctsNum);

volatile int cutsCounter = 0;
Fifo *fifo = NULL;
sem_t *BARBER;
sem_t *FIFO;
sem_t *CHECKER;
sigset_t fullMask;

int main(int argc, char **argv) {
    if (argc != 3) FAILURE_EXIT(3, "Clients creator: specify clients Number and cuts Number");
    int clientsNum = (int) strtol(argv[1], '\0', 10);
    int cutsNum = (int) strtol(argv[2], '\0', 10);
    if (clientsNum < 1) FAILURE_EXIT(3, "Clients creator: Wrong number of Clients!");
    if (cutsNum < 1) FAILURE_EXIT(3, "Clients creator: Wrong number of Cuts!");

    if (atexit(freeResources) == -1) FAILURE_EXIT(3, "Clients creator: atexit failed!");
    if (signal(SIGINT, intHandler) == SIG_ERR) FAILURE_EXIT(3, "Clients creator: setting SIGINT handler failed!");
    if (signal(SIGRTMIN, rtminHandler) == SIG_ERR) FAILURE_EXIT(3, "Clients creator: setting SIGRTMIN handler failed!");


    prepareFifo();
    prepareSemaphores();
    sigfillset(&fullMask);
    sigdelset(&fullMask, SIGRTMIN);
    sigdelset(&fullMask, SIGINT);

    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGRTMIN);
    if (sigprocmask(SIG_BLOCK, &mask, NULL) == -1) FAILURE_EXIT(3, "Clients creator: blocking masked signals failed!");

    for (int i = 0; i < clientsNum; i++) {
        pid_t id = fork();
        if (id == -1) FAILURE_EXIT(3, "Clients creator: Fork failed!");
        if (id == 0) {
            getCut(cutsNum);
            return 0;
        }
    }

    printf("Clients creator: All clients has been created!\n");
    while (1) {
        wait(NULL);
        if (errno == ECHILD) break;
    }

    return 0;
}

void getCut(int ctsNum) {
    while (cutsCounter < ctsNum) {
        if (sem_wait(CHECKER) == -1) FAILURE_EXIT(3, "Client: taking CHECKER failed!");

        if (sem_wait(FIFO) == -1) FAILURE_EXIT(3, "Client: taking FIFO failed!");

        int res = takePlace();

        if (sem_post(FIFO) == -1) FAILURE_EXIT(3, "Client: releasing FIFO failed!");

        if (sem_post(CHECKER) == -1) FAILURE_EXIT(3, "Client: releasing CHECKER failed!");

        if (res != -1) {
            sigsuspend(&fullMask);
            printf("Time: %ld, Client %d just got cut!\n", getMicroTime(), getpid());
            fflush(stdout);
        }
    }
}

int takePlace() {
    int barberStat;
    if (sem_getvalue(BARBER, &barberStat) == -1) FAILURE_EXIT(3, "Client: getting value of BARBER sem failed!");

    pid_t myPID = getpid();

    if (barberStat == 0) {
        if (sem_post(BARBER) == -1) FAILURE_EXIT(3, "Client: awakening barber failed!");
        printf("Time: %ld, Client %d has awakened barber!\n", getMicroTime(), myPID);
        fflush(stdout);

        fifo->chair = myPID;

        return 1;
    } else {
        int res = pushFifo(fifo, myPID);
        if (res == -1) {
            printf("Time: %ld, Client %d couldnt find free place!\n", getMicroTime(), myPID);
            fflush(stdout);
            return -1;
        } else {
            printf("Time: %ld, Client %d took place in the queue!\n", getMicroTime(), myPID);
            fflush(stdout);
            return 0;
        }
    }
}

void prepareFifo() {
    int shmID = shm_open(shmPath, O_RDWR, 0666);
    if (shmID == -1) FAILURE_EXIT(3, "Clients creator: opening shared memory failed!");

//    if (ftruncate(shmID, sizeof(Fifo)) == -1) FAILURE_EXIT(3, "Clients creator: truncating shm failed!");

    void *tmp = mmap(NULL, sizeof(Fifo), PROT_READ | PROT_WRITE, MAP_SHARED, shmID, 0);
    if (tmp == (void *) (-1)) FAILURE_EXIT(3, "Clients creator: attaching shm failed!");
    fifo = (Fifo *) tmp;
}

void prepareSemaphores() {
    BARBER = sem_open(barberPath, O_RDWR);
    if (BARBER == SEM_FAILED) FAILURE_EXIT(3, "Clients creator: creating BARBER semaphore failed!");

    FIFO = sem_open(fifoPath, O_RDWR);
    if (FIFO == SEM_FAILED) FAILURE_EXIT(3, "Clients creator: creating FIFO semaphore failed!");

    CHECKER = sem_open(checkerPath, O_RDWR);
    if (CHECKER == SEM_FAILED) FAILURE_EXIT(3, "Clients creator: creating CHECKER semaphore failed!");
}

void freeResources(void) {
    if (munmap(fifo, sizeof(fifo)) == -1) printf("Clients creator: detaching FIFO shm failed!\n");
    else printf("Clients creator: detached fifo sm!\n");

    if (sem_close(BARBER) == -1) printf("Clients creator: closing BARBER semaphore failed!");
    if (sem_close(FIFO) == -1) printf("Clients creator: closing FIFO semaphore failed!");
    if (sem_close(CHECKER) == -1) printf("Clients creator: closing CHECKER semaphore failed!");
}

void rtminHandler(int signo) {
    cutsCounter++;
}

void intHandler(int signo) {
    exit(2);
}

