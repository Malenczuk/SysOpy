#define _GNU_SOURCE

#include "hairdressers.h"

void intHandler(int);

void rtminHandler(int);

void prepareFifo();

void prepareSemaphores();

void freeResources(void);

int takePlace();

void getCut(int);

Fifo *fifo = NULL;
key_t fifoKey;
sigset_t fullMask;
int shmID = -1;
int SID = -1;
volatile int ctsCounter = 0;

int main(int argc, char **argv) {
    if (argc != 3) FAILURE_EXIT(3, "Clients creator: Specify clients Number and cuts Number!");
    int clientsNum = (int) strtol(argv[1], '\0', 10);
    int cutsNum = (int) strtol(argv[2], '\0', 10);
    if (clientsNum < 1) FAILURE_EXIT(3, "Clients creator: Wrong number of Clients!");
    if (cutsNum < 1) FAILURE_EXIT(3, "Clients creator: Wrong number of Cuts!");

    if (atexit(freeResources) == -1) FAILURE_EXIT(3, "Clients creator: setting atexit failed!");
    if (signal(SIGINT, intHandler) == SIG_ERR) FAILURE_EXIT(3, "Clients creator: setting SIGINT handler failed!");
    if (signal(SIGRTMIN, rtminHandler) == SIG_ERR) FAILURE_EXIT(3, "Clients creator: setting SIGRTMIN handler failed!");


    prepareFifo();
    prepareSemaphores();

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
    while (ctsCounter < ctsNum) {
        struct sembuf sops;
        sops.sem_num = CHECKER;
        sops.sem_op = -1;
        sops.sem_flg = 0;
        if (semop(SID, &sops, 1) == -1) FAILURE_EXIT(3, "Client: taking CHECKER semaphore failed!");

        sops.sem_num = FIFO;
        if (semop(SID, &sops, 1) == -1) FAILURE_EXIT(3, "Client: taking FIFO semaphore failed!");

        int res = takePlace();

        sops.sem_op = 1;
        if (semop(SID, &sops, 1) == -1) FAILURE_EXIT(3, "Client: releasing FIFO semaphore failed!");

        sops.sem_num = CHECKER;
        if (semop(SID, &sops, 1) == -1) FAILURE_EXIT(3, "Client: releasing CHECKER semaphore failed!");

        if (res != -1) {
            sigsuspend(&fullMask);
            printf("Time: %ld, Client %d just got cut!\n", getMicroTime(), getpid());
            fflush(stdout);
        }
    }
}

int takePlace() {
    int barberStat = semctl(SID, 0, GETVAL);
    if (barberStat == -1) FAILURE_EXIT(3, "Client: getting value of BARBER semaphore failed!");

    pid_t myPID = getpid();

    if (barberStat == 0) {
        struct sembuf sops;
        sops.sem_num = BARBER;
        sops.sem_op = 1;
        sops.sem_flg = 0;

        if (semop(SID, &sops, 1) == -1) FAILURE_EXIT(3, "Client: awakening barber failed!");
        printf("Time: %ld, Client %d has awakened barber!\n", getMicroTime(), myPID);
        fflush(stdout);
        if (semop(SID, &sops, 1) == -1) FAILURE_EXIT(3, "Client: awakening barber failed!");

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
    char *path = getenv("HOME");
    if (path == NULL) FAILURE_EXIT(3, "Clients creator: getting environmental variable failed!");

    fifoKey = ftok(path, PROJECT_ID);
    if (fifoKey == -1) FAILURE_EXIT(3, "Clients creator: getting shm key failed!");

    shmID = shmget(fifoKey, 0, 0);
    if (shmID == -1) FAILURE_EXIT(3, "Clients creator: opening shm failed!");

    void *tmp = shmat(shmID, NULL, 0);
    if (tmp == (void *) (-1)) FAILURE_EXIT(3, "Clients creator: attaching shm failed!");
    fifo = (Fifo *) tmp;
}

void prepareSemaphores() {
    SID = semget(fifoKey, 0, 0);
    if (SID == -1) FAILURE_EXIT(3, "Clients creator: opening semaphores failed!");
}

void freeResources(void) {
    if (shmdt(fifo) == -1) printf("Clients creator: detaching FIFO shm failed!\n");
    else printf("Clients creator: detached FIFO shm!\n");
}

void intHandler(int signo) {
    exit(2);
}

void rtminHandler(int signo) {
    ctsCounter++;
}