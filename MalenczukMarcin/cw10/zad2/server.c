#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <libnet.h>
#include <sys/un.h>
#include <sys/epoll.h>
#include "cluster.h"

#define FAILURE_EXIT(code, format, ...) { fprintf(stderr, format, ##__VA_ARGS__); exit(code);}

int in(void *const a, void *const pbase, size_t total_elems, size_t size, __compar_fn_t cmp) {
    char *base_ptr = (char *) pbase;
    if (total_elems > 0) {
        for (int i = 0; i < total_elems; ++i) {
            if ((*cmp)(a, (void *) (base_ptr + i * size)) == 0) return i;
        }
    }
    return -1;
}

int cmp_name(char *name, Client *client){
    return strcmp(name, client->name);
}

void sigHandler(int);

void __init__(char *, char *);

void __del__();

void *ping_routine(void *);

void *command_routine(void *);

void handle_message(int);

void register_client(int, message_t, struct sockaddr *, socklen_t);

void unregister_client(char *);

void remove_client(int);

void remove_socket(int);

int web_socket;
int local_socket;
int epoll;
char *unix_path;

pthread_t ping;
pthread_t command;

pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;
Client clients[CLIENT_MAX];
int cn = 0;
int op_num = 0;

int main(int argc, char **argv) {
    if (argc != 3) FAILURE_EXIT(1, "\nUsage: %s <port number> <unix path>\n", argv[0]);
    if (atexit(__del__) == -1) FAILURE_EXIT(1, "\nError : Could not set AtExit\n")

    __init__(argv[1], argv[2]);

    struct epoll_event event;
    while (1) {
        if (epoll_wait(epoll, &event, 1, -1) == -1) FAILURE_EXIT(1, "\nError : epoll_wait failed\n");
        handle_message(event.data.fd);
    }
}

void *ping_routine(void *arg) {
    uint8_t message_type = PING;
    while (1) {
        pthread_mutex_lock(&clients_mutex);
        for (int i = 0; i < cn; ++i) {
            if (clients[i].un_active != 0) {
                printf("Client \"%s\" do not respond. Removing from registered clients\n", clients[i].name);
                remove_client(i--);
            } else {
                int socket = clients[i].connect_type == WEB ? web_socket : local_socket;
                if (sendto(socket, &message_type, 1, 0, clients[i].sockaddr, clients[i].socklen) != 1)
                    FAILURE_EXIT(1, "\nError : Could not PING client \"%s\"\n", clients[i].name);
                clients[i].un_active++;
            }
        }
        pthread_mutex_unlock(&clients_mutex);
        sleep(5);
    }
}

void *command_routine(void *arg) {
    srand((unsigned int) time(NULL));
    operation_t msg;
    uint8_t message_type = REQUEST;
    int error = 0;
    char buffer[256];
    while (1) {
        printf("Enter command: \n");
        fgets(buffer, 256, stdin);
        if (sscanf(buffer, "%lf %c %lf", &msg.arg1, &msg.op, &msg.arg2) != 3) {
            printf("Wrong command format\n");
            continue;
        }
        if (msg.op != '+' && msg.op != '-' && msg.op != '*' && msg.op != '/') {
            printf("Wrong operator (%c)\n", msg.op);
            continue;
        }
        pthread_mutex_lock(&clients_mutex);
        if (cn == 0) {
            printf("No Clients connected to server\n");
            continue;
        }
        msg.op_num = ++op_num;
        error = 0;
        int i = rand() % cn;
        int socket = clients[i].connect_type == WEB ? web_socket : local_socket;
        if (sendto(socket, &message_type, 1, 0, clients[i].sockaddr, clients[i].socklen) != 1) error = 1;
        if (sendto(socket, &msg, sizeof(operation_t), 0, clients[i].sockaddr, clients[i].socklen) !=
            sizeof(operation_t)) error = 1;
        if (error == 0)
            printf("Command %d: %lf %c %lf Has been sent to client \"%s\"\n",
                   msg.op_num, msg.arg1, msg.op, msg.arg2, clients[i].name);
        else
            printf("\nError : Could not send request to client \"%s\"\n", clients[i].name);
        pthread_mutex_unlock(&clients_mutex);

    }
}

void handle_message(int socket) {
    struct sockaddr* sockaddr = malloc(sizeof(struct sockaddr));
    socklen_t socklen = sizeof(struct sockaddr);
    message_t msg;

    if(recvfrom(socket, &msg, sizeof(message_t), 0, sockaddr, &socklen) != sizeof(message_t))
        FAILURE_EXIT(1,"\nError : Could not receive new message\n");

    switch(msg.message_type){
        case REGISTER:{
            register_client(socket, msg, sockaddr, socklen);
            break;
        }
        case UNREGISTER:{
            unregister_client(msg.name);
            break;
        }
        case RESULT:{
            printf("Client \"%s\" calculated operation [%d]. Result: %lf\n", msg.name, msg.op_num, msg.value);
            break;
        }
        case PONG:{
            pthread_mutex_lock(&clients_mutex);
            int i = in(msg.name, clients, (size_t) cn, sizeof(Client), (__compar_fn_t) cmp_name);
            if (i >= 0) clients[i].un_active--;
            pthread_mutex_unlock(&clients_mutex);
            break;
        }
        default:
            printf("Unknown message type\n");
            break;
    }
}

void register_client(int socket, message_t msg, struct sockaddr *sockaddr, socklen_t socklen){
    uint8_t message_type;
    pthread_mutex_lock(&clients_mutex);
    if(cn == CLIENT_MAX){
        message_type = FAILSIZE;
        if (sendto(socket, &message_type, 1, 0, sockaddr, socklen) != 1)
            FAILURE_EXIT(1, "\nError : Could not send FAILSIZE message to client \"%s\"\n", msg.name);
        free(sockaddr);
    } else{
        int exists = in(msg.name, clients, (size_t) cn, sizeof(Client), (__compar_fn_t) cmp_name);
        if(exists != -1){
            message_type = FAILNAME;
            if (sendto(socket, &message_type, 1, 0, sockaddr, socklen) != 1)
                FAILURE_EXIT(1, "\nError : Could not send FAILNAME message to client \"%s\"\n", msg.name);
            free(sockaddr);
        } else{
            clients[cn].sockaddr = sockaddr;
            clients[cn].connect_type = msg.connect_type;
            clients[cn].socklen = socklen;
            clients[cn].name = malloc(strlen(msg.name) + 1);
            clients[cn].un_active = 0;
            strcpy(clients[cn++].name, msg.name);
            message_type = SUCCESS;
            if (sendto(socket, &message_type, 1, 0, sockaddr, socklen) != 1)
                FAILURE_EXIT(1, "\nError : Could not send SUCCESS message to client \"%s\"\n", msg.name);
        }
    }
    pthread_mutex_unlock(&clients_mutex);
}

void unregister_client(char *client_name){
    pthread_mutex_lock(&clients_mutex);
    int i = in(client_name, clients, (size_t) cn, sizeof(Client), (__compar_fn_t) cmp_name);
    if(i >= 0){
        remove_client(i);
        printf("Client \"%s\" unregistered\n", client_name);
    }
    pthread_mutex_unlock(&clients_mutex);
}

void remove_client(int i) {
    free(clients[i].sockaddr);
    free(clients[i].name);

    cn--;
    for (int j = i; j < cn; ++j)
        clients[j] = clients[j + 1];
}

void sigHandler(int signo) {
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
    web_address.sin_addr.s_addr = htonl(INADDR_ANY);
    web_address.sin_port = htons(port_num);

    if ((web_socket = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
        FAILURE_EXIT(1, "\nError : Could not create web socket\n");

    if (bind(web_socket, (const struct sockaddr *) &web_address, sizeof(web_address)))
        FAILURE_EXIT(1, "\nError : Could not bind web socket\n")

    // Init Local Socket
    struct sockaddr_un local_address;
    local_address.sun_family = AF_UNIX;

    snprintf(local_address.sun_path, UNIX_PATH_MAX, "%s", unix_path);

    if ((local_socket = socket(AF_UNIX, SOCK_DGRAM, 0)) < 0)
        FAILURE_EXIT(1, "\nError : Could not create local socket\n");

    if (bind(local_socket, (const struct sockaddr *) &local_address, sizeof(local_address)))
        FAILURE_EXIT(1, "\nError : Could not bind local socket\n");

    // Init EPoll
    struct epoll_event event;
    event.events = EPOLLIN | EPOLLPRI;

    if ((epoll = epoll_create1(0)) == -1)
        FAILURE_EXIT(1, "\nError : Could not create epoll\n");

    event.data.fd = web_socket;
    if(epoll_ctl(epoll, EPOLL_CTL_ADD, web_socket, &event) == -1)
        FAILURE_EXIT(1, "\nError : Could not add Web Socket to epoll\n");

    event.data.fd = local_socket;
    if(epoll_ctl(epoll, EPOLL_CTL_ADD, local_socket, &event) == -1)
        FAILURE_EXIT(1, "\nError : Could not add Local Socket to epoll\n");

    // Start Pinger Thread
    if (pthread_create(&ping, NULL, ping_routine, NULL) != 0)
        FAILURE_EXIT(1, "\nError : Could not create Pinger Thread");

    // Start Commander Thread
    if (pthread_create(&command, NULL, command_routine, NULL) != 0)
        FAILURE_EXIT(1, "\nError : Could not create Commander Thread");
}

void __del__() {
    pthread_cancel(ping);
    pthread_cancel(command);
    if (close(web_socket) == -1)
        fprintf(stderr, "\nError : Could not close Web Socket\n");
    if (close(local_socket) == -1)
        fprintf(stderr, "\nError : Could not close Local Socket\n");
    if (unlink(unix_path) == -1)
        fprintf(stderr, "\nError : Could not unlink Unix Path\n");
    if (close(epoll) == -1)
        fprintf(stderr, "\nError : Could not close epoll\n");
}