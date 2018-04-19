#define _GNU_SOURCE

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

#include "communication.h"

#define FAILURE_EXIT(code, format, ...) { fprintf(stderr, format, ##__VA_ARGS__); exit(code);}

void registerClient(key_t privateKey);
void rqMirror(Msg *msg);
void rqCalc(Msg *msg);
void rqTime(Msg *msg);
void rqEnd(Msg *msg);

int sessionID = -2;
int publicID = -1;
int privateID = -1;

int getQueueID(char* path, int ID){
    int key = ftok(path, ID);
    if(key == -1) FAILURE_EXIT(3, "Generation of key failed!");

    int QueueID = msgget(key, 0);
    if(QueueID == -1) FAILURE_EXIT(3, "Opening queue failed!");

    return QueueID;
}

void rmQueue(void){
    if(privateID > -1){
        if(msgctl(privateID, IPC_RMID, NULL) == -1){
            printf("There was some error deleting clients's queue!\n");
        }
        else printf("Client's queue deleted successfully!\n");
    }
}

void intHandler(int signo){
    exit(2);
}

int main(void){
    if(atexit(rmQueue) == -1) FAILURE_EXIT(3, "Registering client's atexit failed!");
    if(signal(SIGINT, intHandler) == SIG_ERR) FAILURE_EXIT(3, "Registering INT failed!");

    char* path = getenv("HOME");
    if(path == NULL) FAILURE_EXIT(3, "Getting enviromental variable 'HOME' failed!");

    publicID = getQueueID(path, PROJECT_ID);

    key_t privateKey = ftok(path, getpid());
    if(privateKey == -1) FAILURE_EXIT(3, "Generation of private key failed!");

    privateID = msgget(privateKey, IPC_CREAT | IPC_EXCL | 0666);
    if(privateID == -1) FAILURE_EXIT(3, "Creation of private queue failed!");

    registerClient(privateKey);

    char cmd[20];
    Msg msg;
    while(1){
        msg.senderPID = getpid();
        printf("Enter your request: ");
        if(fgets(cmd, 20, stdin) == NULL){
            printf("Error reading your command!\n");
            continue;
        }
        int n = strlen(cmd);
        if(cmd[n-1] == '\n') cmd[n-1] = 0;


        if(strcmp(cmd, "mirror") == 0){
            rqMirror(&msg);
        }else if(strcmp(cmd, "calc") == 0){
            rqCalc(&msg);
        }else if(strcmp(cmd, "time") == 0){
            rqTime(&msg);
        }else if(strcmp(cmd, "end") == 0){
            rqEnd(&msg);
        }else if(strcmp(cmd, "q") == 0){
            exit(0);
        }else printf("Wrong command!\n");
    }
    return 0;
}

void registerClient(key_t privateKey){
    Msg msg;
    msg.mtype = LOGIN;
    msg.senderPID = getpid();
    sprintf(msg.cont, "%d", privateKey);

    if(msgsnd(publicID, &msg, MSG_SIZE, 0) == -1) FAILURE_EXIT(3, "LOGIN request failed!");
    if(msgrcv(privateID, &msg, MSG_SIZE, 0, 0) == -1) FAILURE_EXIT(3, "catching LOGIN response failed!");
    if(sscanf(msg.cont, "%d", &sessionID) < 1) FAILURE_EXIT(3, "scanning LOGIN response failed!");
    if(sessionID < 0) FAILURE_EXIT(3, "Server cannot have more clients!");

    printf("Client registered! My session nr is %d!\n", sessionID);
}

void rqMirror(Msg *msg){
    msg->mtype = MIRROR;
    printf("Enter string of characters to Mirror: ");
    if(fgets(msg->cont, MAX_CONT_SIZE, stdin) == NULL){
        printf("Too many characters!\n");
        return;
    }
    if(msgsnd(publicID, msg, MSG_SIZE, 0) == -1) FAILURE_EXIT(3, "MIRROR request failed!");
    if(msgrcv(privateID, msg, MSG_SIZE, 0, 0) == -1) FAILURE_EXIT(3, "catching MIRROR response failed!");
    printf("%s", msg->cont);
}
void rqCalc(Msg *msg){
    msg->mtype = CALC;
    printf("ADDITIONAL OPTIONS:\n"
           " 'obase=n;' - set output to base n\n"
           " 'ibase=n;' - set input to base n\n"
           " 'scale=n;' - set floating point precision to n\n"
           "Enter expression to calculate: ");
    if(fgets(msg->cont, MAX_CONT_SIZE, stdin) == NULL){
        printf("Too many characters!\n");
        return;
    }
    if(msgsnd(publicID, msg, MSG_SIZE, 0) == -1) FAILURE_EXIT(3, "CALC request failed!");
    if(msgrcv(privateID, msg, MSG_SIZE, 0, 0) == -1) FAILURE_EXIT(3, "catching CALC response failed!");
    printf("%s", msg->cont);
}
void rqTime(Msg *msg){
    msg->mtype = TIME;

    if(msgsnd(publicID, msg, MSG_SIZE, 0) == -1) FAILURE_EXIT(3, "TIME request failed!");
    if(msgrcv(privateID, msg, MSG_SIZE, 0, 0) == -1) FAILURE_EXIT(3, "catching TIME response failed!");
    printf("%s\n", msg->cont);
}
void rqEnd(Msg *msg){
    msg->mtype = END;

    if(msgsnd(publicID, msg, MSG_SIZE, 0) == -1) FAILURE_EXIT(3, "END request failed!");
}