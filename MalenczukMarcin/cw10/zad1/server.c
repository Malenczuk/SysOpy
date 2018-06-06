#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <string.h>
#include <netinet/in.h>
#include <libnet.h>
#include <sys/un.h>
#include <sys/epoll.h>
#include "cluster.h"

#define FAILURE_EXIT(code, format, ...) { fprintf(stderr, format, ##__VA_ARGS__); exit(code);}

void sigHandler(int);

void __init__(char *, char *);

void __del__();

void *start_pinger(void *);

void *start_commander(void *);

void handle_connection(int);

void handle_message(int);

int web_socket;
int local_socket;
int epoll;

char *unix_path;

pthread_t pinger;
pthread_t commander;

int main(int argc, char **argv) {
    if (argc != 3) FAILURE_EXIT(1, "\nUsage: %s <port number> <unix path>\n", argv[0]);
    if (atexit(__del__) == -1) FAILURE_EXIT(1, "\nError : Could not set AtExit\n")

    __init__(argv[1], argv[2]);

    struct epoll_event event;
    while (1) {
        if(epoll_wait(epoll, &event, 1, -1) == -1) FAILURE_EXIT(1, "\nError : epoll_wait failed\n");

        if(event.data.fd < 0)
            handle_connection(-event.data.fd);
        else
            handle_message(event.data.fd);
    }
    return 0;
}

void handle_connection(int socket){
    int client = accept(socket, NULL, NULL);
    if(client == -1) FAILURE_EXIT(1, "\nError : Could not accept new client\n");

    struct epoll_event event;
    event.events = EPOLLIN | EPOLLPRI;
    event.data.fd = client;

    if (epoll_ctl(epoll, EPOLL_CTL_ADD, client, &event) == -1)
        FAILURE_EXIT(1, "\nError : Could not add new client to epoll\n");
}

void handle_message(int socket){

}

void *start_pinger(void *arg) {
    return NULL;
}

void *start_commander(void *arg) {
    return NULL;
}

void sigHandler(int signo){
    FAILURE_EXIT(2, "\nSIGINT\n");
}

void __init__(char *arg1, char *arg2) {
    // Handle Signal
    signal(SIGINT, sigHandler);

    // Get Port Number
    uint16_t port_num = (uint16_t) atoi(arg1);
    if (port_num < 1024 || port_num > 65535)
        FAILURE_EXIT(1, "\nError : Wrong Port Number\n");

    // Get Unix Path
    unix_path = arg2;
    if (strlen(unix_path) < 1 || strlen(unix_path) > UNIX_PATH_MAX)
        FAILURE_EXIT(1, "\nError : Wrong Unix Path\n");

    // Init Web Socket
    struct sockaddr_in web_address;
    web_address.sin_family = AF_INET;
    web_address.sin_addr.s_addr = INADDR_ANY;
    web_address.sin_port = htons(port_num);

    if ((web_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        FAILURE_EXIT(1, "\nError : Could not create web socket\n");

    if (bind(web_socket, (const struct sockaddr *) &web_address, sizeof(web_address)))
        FAILURE_EXIT(1, "\nError : Could not bind web socket\n")

    if (listen(web_socket, 64) == -1)
        FAILURE_EXIT(1, "\nError : Could not listen to web socket\n");

    // Init Local Socket
    struct sockaddr_un local_address;
    local_address.sun_family = AF_UNIX;

    snprintf(local_address.sun_path, UNIX_PATH_MAX, "%s", unix_path);

    if ((local_socket = socket(AF_UNIX, SOCK_STREAM, 0)) < 0)
        FAILURE_EXIT(1, "\nError : Could not create local socket\n");

    if (bind(local_socket, (const struct sockaddr *) &local_address, sizeof(local_address)))
        FAILURE_EXIT(1, "\nError : Could not bind local socket\n");

    if (listen(local_socket, 64) == -1)
        FAILURE_EXIT(1, "\nError : Could not listen to local socket\n");

    // Init EPoll
    struct epoll_event event;
    event.events = EPOLLIN | EPOLLPRI;

    if ((epoll = epoll_create1(0)) == -1)
        FAILURE_EXIT(1, "\nError : Could not create epoll\n");

    event.data.fd = -web_socket;
    if(epoll_ctl(epoll, EPOLL_CTL_ADD, web_socket, &event) == -1)
        FAILURE_EXIT(1, "\nError : Could not add Web Socket to epoll\n");

    event.data.fd = -local_socket;
    if(epoll_ctl(epoll, EPOLL_CTL_ADD, local_socket, &event) == -1)
        FAILURE_EXIT(1, "\nError : Could not add Local Socket to epoll\n");

    // Start Pinger Thread
    if (pthread_create(&pinger, NULL, start_pinger, NULL) != 0)
        FAILURE_EXIT(1, "\nError : Could not create Pinger Thread");

    // Start Commander Thread
    if (pthread_create(&commander, NULL, start_commander, NULL) != 0)
        FAILURE_EXIT(1, "\nError : Could not create Commander Thread");
}

void __del__() {
    pthread_cancel(pinger);
    pthread_cancel(commander);
    if (close(web_socket) == -1)
        fprintf(stderr, "\nError : Could not close Web Socket\n");
    if (close(local_socket) == -1)
        fprintf(stderr, "\nError : Could not close Local Socket\n");
    if (unlink(unix_path) == -1)
        fprintf(stderr, "\nError : Could not unlink Unix Path\n");
    if (close(epoll) == -1)
        fprintf(stderr, "\nError : Could not close epoll\n");
}