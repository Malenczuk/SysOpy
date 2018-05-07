#define _GNU_SOURCE

#include "hairdressers.h"


void intHandler(int);

void clearResources(void);

void prepareFifo(int chairNum);

void prepareSemaphores();

void doBarberWork();

void cut(pid_t pid);

pid_t takeChair(struct sembuf *);

Fifo *fifo = NULL;
key_t fifoKey;
int shmID = -1;
int SID = -1;

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
    struct sembuf sops;
    sops.sem_flg = 0;
    while (1) {
        sops.sem_num = BARBER;
        sops.sem_op = -1;

        if (semop(SID, &sops, 1) == -1) FAILURE_EXIT(3, "Barber: taking BARBER semaphore failed!");

        pid_t toCut = takeChair(&sops);
        cut(toCut);

        while (1) {
            sops.sem_num = FIFO;
            sops.sem_op = -1;
            if (semop(SID, &sops, 1) == -1) FAILURE_EXIT(3, "Barber: taking FIFO semaphore failed!");
            toCut = popFifo(fifo);

            if (toCut != -1) {
                sops.sem_op = 1;
                if (semop(SID, &sops, 1) == -1) FAILURE_EXIT(3, "Barber: releasing FIFO semaphore failed!");
                cut(toCut);
            } else {
                printf("Time: %ld, Barber: going to sleep...\n", getMicroTime());
                fflush(stdout);
                sops.sem_num = BARBER;
                sops.sem_op = -1;
                if (semop(SID, &sops, 1) == -1) FAILURE_EXIT(3, "Barber: taking BARBER semaphore failed!");

                sops.sem_num = FIFO;
                sops.sem_op = 1;
                if (semop(SID, &sops, 1) == -1) FAILURE_EXIT(3, "Barber: releasing FIFO semaphore failed!");
                break;
            }
        }
    }
}

pid_t takeChair(struct sembuf *sops) {
    sops->sem_num = FIFO;
    sops->sem_op = -1;
    if (semop(SID, sops, 1) == -1) FAILURE_EXIT(3, "Barber: taking FIFO semaphore failed!");

    pid_t toCut = fifo->chair;

    sops->sem_op = 1;
    if (semop(SID, sops, 1) == -1) FAILURE_EXIT(3, "Barber: releasing FIFO semaphore failed!");

    return toCut;
}

void cut(pid_t pid) {
    printf("Time: %ld, Barber: preparing to cut %d\n", getMicroTime(), pid);
    fflush(stdout);

    kill(pid, SIGRTMIN);

    printf("Time: %ld, Barber: finished cutting %d\n", getMicroTime(), pid);
    fflush(stdout);
}

void prepareFifo(int chairNum) {
    char *path = getenv("HOME");
    if (path == NULL) FAILURE_EXIT(3, "Barber: Getting environmental variable failed!");

    fifoKey = ftok(path, PROJECT_ID);
    if (fifoKey == -1) FAILURE_EXIT(3, "Barber: getting shm key failed!");

    shmID = shmget(fifoKey, sizeof(Fifo), IPC_CREAT | IPC_EXCL | 0666);
    if (shmID == -1) FAILURE_EXIT(3, "Barber: creating shm failed!");

    void *tmp = shmat(shmID, NULL, 0);
    if (tmp == (void *) (-1)) FAILURE_EXIT(3, "Barber: attaching shm failed!");
    fifo = (Fifo *) tmp;

    fifoInit(fifo, chairNum);
}

void prepareSemaphores() {
    SID = semget(fifoKey, 4, IPC_CREAT | IPC_EXCL | 0666);
    if (SID == -1) FAILURE_EXIT(3, "Barber: creating semaphores failed!");

    for (int i = 1; i < 3; i++)
        if (semctl(SID, i, SETVAL, 1) == -1) FAILURE_EXIT(3, "Barber: setting semaphores failed!");

    if (semctl(SID, 0, SETVAL, 0) == -1) FAILURE_EXIT(3, "Barber: setting semaphores failed!");
}

void clearResources(void) {
    if (shmdt(fifo) == -1) printf("Barber: detaching fifo shm failed!\n");
    else printf("Barber: detached fifo sm!\n");

    if (shmctl(shmID, IPC_RMID, NULL) == -1) printf("Barber: deleting fifo shm failed!\n");
    else printf("Barber: deleted fifo sm!\n");

    if (semctl(SID, 0, IPC_RMID) == -1) printf("Barber: deleting semaphores failed!");
    else printf("Barber: deleted semaphores!\n");
}

void intHandler(int signo) {
    exit(2);
}
