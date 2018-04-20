#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <mqueue.h>
#include <ctype.h>
#include <time.h>
#include <fcntl.h>
#include <signal.h>

#include "communication.h"

#define FAILURE_EXIT(code, format, ...) { fprintf(stderr, format, ##__VA_ARGS__); exit(code);}

void registerClient();
void rqMirror(struct Msg *msg);
void rqCalc(struct Msg *msg);
void rqTime(struct Msg *msg);
void rqEnd(struct Msg *msg);
void rqQ(struct Msg *msg);

int sessionID = -2;
mqd_t publicID = -1;
mqd_t privateID = -1;
char myPath[20];

void rmQueue(void){
    if(privateID > -1){
        if(sessionID >= 0){
            printf("\nBefore quitting, i will try to send QUIT request to public queue!\n");
            Msg msg;
            msg.senderPID = getpid();
            rqQ(&msg);
        }

        if(mq_close(publicID) == -1){
            printf("There was some error closing servers's queue!\n");
        }
        else printf("Servers's queue closed successfully!\n");

        if(mq_close(privateID) == -1){
            printf("There was some error closing client's queue!\n");
        }
        else printf("Client's queue closed successfully!\n");

        if(mq_unlink(myPath) == -1){
            printf("There was some error deleting client's queue!\n");
        }
        else printf("Client's queue deleted successfully!\n");
    } else printf("There was no need of deleting queue!\n");
}
void intHandler(int signo){
    exit(2);
}

int main(void){
    if(atexit(rmQueue) == -1) FAILURE_EXIT(3, "Registering client's atexit failed!");
    if(signal(SIGINT, intHandler) == SIG_ERR) FAILURE_EXIT(3, "Registering INT failed!");

    sprintf(myPath, "/%d", getpid());

    publicID = mq_open(serverPath, O_WRONLY);
    if(publicID == -1) FAILURE_EXIT(3, "Opening public queue failed!");

    struct mq_attr posixAttr;
    posixAttr.mq_maxmsg = MAX_MQSIZE;
    posixAttr.mq_msgsize = MSG_SIZE;

    privateID = mq_open(myPath, O_RDONLY | O_CREAT | O_EXCL, 0666, &posixAttr);
    if(privateID == -1) FAILURE_EXIT(3, "Creation of private queue failed!");

    registerClient();

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

void registerClient(){
    Msg msg;
    msg.mtype = LOGIN;
    msg.senderPID = getpid();

    if(mq_send(publicID, (char*) &msg, MSG_SIZE, 1) == -1) FAILURE_EXIT(3, "Login request failed!");
    if(mq_receive(privateID,(char*) &msg, MSG_SIZE, NULL) == -1) FAILURE_EXIT(3, "Catching LOGIN response failed!");
    if(sscanf(msg.cont, "%d", &sessionID) < 1) FAILURE_EXIT(3, "scanning LOGIN response failed!");
    if(sessionID < 0) FAILURE_EXIT(3, "Server cannot have more clients!");

    printf("Client registered! My session nr is %d!\n", sessionID);
}

void rqMirror(struct Msg *msg){
    msg->mtype = MIRROR;
    printf("Enter string of characters to Mirror: ");
    if(fgets(msg->cont, MAX_CONT_SIZE, stdin) == NULL){
        printf("Too many characters!\n");
        return;
    }
    if(mq_send(publicID, (char*) msg, MSG_SIZE, 1) == -1) FAILURE_EXIT(3, "MIRROR request failed!");
    if(mq_receive(privateID,(char*) msg, MSG_SIZE, NULL) == -1) FAILURE_EXIT(3, "Catching MIRROR response failed!");
    printf("%s", msg->cont);
}
void rqCalc(struct Msg *msg){
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
    if(mq_send(publicID, (char*) msg, MSG_SIZE, 1) == -1) FAILURE_EXIT(3, "CALC request failed!");
    if(mq_receive(privateID,(char*) msg, MSG_SIZE, NULL) == -1) FAILURE_EXIT(3, "Catching CALC response failed!");
    printf("%s", msg->cont);
}
void rqTime(struct Msg *msg){
    msg->mtype = TIME;

    if(mq_send(publicID, (char*) msg, MSG_SIZE, 1) == -1) FAILURE_EXIT(3, "TIME request failed!");
    if(mq_receive(privateID,(char*) msg, MSG_SIZE, NULL) == -1) FAILURE_EXIT(3, "Catching TIME response failed!");
    printf("%s\n", msg->cont);
}
void rqEnd(struct Msg *msg){
    msg->mtype = END;

    if(mq_send(publicID, (char*) msg, MSG_SIZE, 1) == -1) FAILURE_EXIT(3, "END request failed!");
}
void rqQ(struct Msg *msg){
    msg->mtype = QUIT;

    if(mq_send(publicID, (char*) msg, MSG_SIZE, 1) == -1) printf("END request failed - Server may have already been closed!\n");
    fflush(stdout);
}
