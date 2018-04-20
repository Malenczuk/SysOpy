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

void publicQueueExecute(struct Msg* msg);
void doLogin(struct Msg* msg);
void doMirror(struct Msg* msg);
void doCalc(struct Msg* msg);
void doTime(struct Msg* msg);
void doEnd(struct Msg* msg);
void doQuit(struct Msg* msg);
int findMQD(pid_t senderPID);
int prepareMsg(struct Msg* msg);

int active = 1;
int clientsData[MAX_CLIENTS][2];
int clientCnt = 0;
mqd_t publicID = -1;

char* convertTime(const time_t* mtime){
    char* buff = malloc(sizeof(char) * 30);
    struct tm * timeinfo;
    timeinfo = localtime (mtime);
    strftime(buff, 20, "%b %d %H:%M", timeinfo);
    return buff;
}

void rmQueue(void){
    for(int i=0; i<clientCnt; i++){
        if(mq_close(clientsData[i][1]) == -1){
            printf("Error closing %d client queue\n", i);
        }
        if(kill(clientsData[i][0], SIGINT) == -1){
            printf("Error killing %d client!\n", i);
        }
    }
    if(publicID > -1){
        if(mq_close(publicID) == -1){
            printf("Error closing public queue\n");
        }
        else printf("Server queue closed!\n");

        if(mq_unlink(serverPath) == -1) printf("Error deleting public Queue!\n");
        else printf("Server queue deleted successfully!\n");
    }else printf("There was no need of deleting queue!\n");
}

void intHandler(int signo){
    active = 0;
    exit(2);
}

int main(void){
    if(atexit(rmQueue) == -1) FAILURE_EXIT(3, "Registering server's atexit failed!");
    if(signal(SIGINT, intHandler) == SIG_ERR) FAILURE_EXIT(3, "Registering INT failed!");
    struct mq_attr currentState;

    struct mq_attr posixAttr;
    posixAttr.mq_maxmsg = MAX_MQSIZE;
    posixAttr.mq_msgsize = MSG_SIZE;

    publicID = mq_open(serverPath, O_RDONLY | O_CREAT | O_EXCL, 0666, &posixAttr);
    if(publicID == -1) FAILURE_EXIT(3, "Creation of public queue failed!");

    Msg buff;
    while(1){
        if(active == 0){
            if(mq_getattr(publicID, &currentState) == -1) FAILURE_EXIT(3, "Couldnt read public queue parameters!");
            if(currentState.mq_curmsgs == 0) exit(0);
        }

        if(mq_receive(publicID,(char*) &buff, MSG_SIZE, NULL) == -1) FAILURE_EXIT(3, "Receiving message by server failed!");
        publicQueueExecute(&buff);
    }
    return 0;
}

void publicQueueExecute(struct Msg* msg){
    if(msg == NULL) return;
    switch(msg->mtype){
        case LOGIN:
            doLogin(msg);
            break;
        case MIRROR:
            doMirror(msg);
            break;
        case CALC:
            doCalc(msg);
            break;
        case TIME:
            doTime(msg);
            break;
        case END:
            doEnd(msg);
            break;
        case QUIT:
            doQuit(msg);
            break;
        default:
            break;
    }
}

void doLogin(struct Msg* msg){
    int clientPID = msg->senderPID;
    char clientPath[15];
    sprintf(clientPath, "/%d", clientPID);

    int clientMQD = mq_open(clientPath, O_WRONLY);
    if(clientMQD == -1 ) FAILURE_EXIT(3, "Reading clientMQD failed!");

    msg->mtype = INIT;
    msg->senderPID = getpid();

    if(clientCnt > MAX_CLIENTS - 1){
        printf("Maximum amount of clients reached!\n");
        sprintf(msg->cont, "%d", -1);
        if(mq_send(clientMQD, (char*) msg, MSG_SIZE, 1) == -1) FAILURE_EXIT(3, "Login response failed!");
        if(mq_close(clientMQD) == -1) FAILURE_EXIT(3, "Closing client's queue failed!");
    }else{
        clientsData[clientCnt][0] = clientPID;
        clientsData[clientCnt++][1] = clientMQD;
        sprintf(msg->cont, "%d", clientCnt-1);
        if(mq_send(clientMQD, (char*) msg, MSG_SIZE, 1) == -1) FAILURE_EXIT(3, "Login response failed!");
    }
}

void doMirror(struct Msg* msg){
    int clientMQD = prepareMsg(msg);
    if(clientMQD == -1) return;

    int msgLen = (int) strlen(msg->cont);
    if(msg->cont[msgLen-1] == '\n') msgLen--;

    for(int i=0; i < msgLen / 2; i++) {
        char buff = msg->cont[i];
        msg->cont[i] = msg->cont[msgLen - i - 1];
        msg->cont[msgLen - i - 1] = buff;
    }

    if(mq_send(clientMQD, (char*) msg, MSG_SIZE, 1) == -1) FAILURE_EXIT(3, "MIRROR response failed!");
}

void doCalc(struct Msg* msg){
    int clientMQD = prepareMsg(msg);
    if(clientMQD == -1) return;

    char cmd[4108];
    sprintf(cmd, "echo '%s' | bc", msg->cont);
    FILE* calc = popen(cmd, "r");
    fgets(msg->cont, MAX_CONT_SIZE, calc);
    pclose(calc);

    if(mq_send(clientMQD, (char*) msg, MSG_SIZE, 1) == -1) FAILURE_EXIT(3, "CALC response failed!");
}

void doTime(struct Msg* msg){
    int clientMQD = prepareMsg(msg);
    if(clientMQD == -1) return;

    time_t timer;
    time(&timer);
    char* timeStr = convertTime(&timer);

    sprintf(msg->cont, "%s", timeStr);
    free(timeStr);

    if(mq_send(clientMQD, (char*) msg, MSG_SIZE, 1) == -1) FAILURE_EXIT(3, "TIME response failed!");
}

void doEnd(struct Msg* msg){
    active = 0;
}
void doQuit(struct Msg* msg){
    int i;
    for(i=0; i<clientCnt; i++){
        if(clientsData[i][0] == msg->senderPID) break;
    }
    if(i == clientCnt){
        printf("Client Not Found!\n");
        return;
    }
    if(mq_close(clientsData[i][1]) == -1) FAILURE_EXIT(3, "Closing clients queue in QUIT response failed!");
    for(; i+1 < clientCnt; i++){
        clientsData[i][0] = clientsData[i+1][0];
        clientsData[i][1] = clientsData[i+1][1];
    }
    clientCnt--;
    printf("Successfully shifted clientsData!\n");
}

int prepareMsg(struct Msg* msg){
    int clientMQD = findMQD(msg->senderPID);
    if(clientMQD == -1){
        printf("Client Not Found!\n");
        return -1;
    }

    msg->mtype = msg->senderPID;
    msg->senderPID = getpid();

    return clientMQD;
}

int findMQD(pid_t senderPID){
    for(int i=0; i<clientCnt; i++){
        if(clientsData[i][0] == senderPID) return clientsData[i][1];
    }
    return -1;
}
