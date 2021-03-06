#ifndef COMMUNICATION_H
#define COMMUNICATION_H

#define MAX_CLIENTS  10
#define PROJECT_ID 2137
#define MAX_CONT_SIZE 4096
#define MSG_SIZE sizeof(Msg)
#define MAX_MQSIZE 9

typedef enum mtype{
    LOGIN = 1, MIRROR = 2, CALC = 3, TIME = 4, END = 5, INIT = 6, QUIT = 7
} mtype;

typedef struct Msg{
    long mtype;
    pid_t senderPID;
    char cont[MAX_CONT_SIZE];
} Msg;

const char serverPath[] = "/aRandomStringJustToMakeSureThereIsNoQueueWithThatExactName";

#endif
