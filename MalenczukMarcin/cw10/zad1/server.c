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

void handle_connection(int);

void handle_message(int);

void register_client(char *, int);

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

        if (event.data.fd < 0)
            handle_connection(-event.data.fd);
        else
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
                if (write(clients[i].fd, &message_type, 1) != 1)
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
        msg.op_num = ++op_num;
        pthread_mutex_lock(&clients_mutex);
        if(cn == 0){
            printf("No Clients connected to server\n");
            continue;
        }
        error = 0;
        int i = rand() % cn;
        if (write(clients[i].fd, &message_type, 1) != 1) error = 1;
        if (write(clients[i].fd, &msg, sizeof(operation_t)) != sizeof(operation_t)) error = 1;
        if(error == 0)
            printf("Command %d: %lf %c %lf Has been sent to client \"%s\"\n",
                   msg.op_num, msg.arg1, msg.op, msg.arg2, clients[i].name);
        else
            printf("\nError : Could not send request to client \"%s\"\n", clients[i].name);
        pthread_mutex_unlock(&clients_mutex);

    }
}

void handle_connection(int socket) {
    int client = accept(socket, NULL, NULL);
    if (client == -1) FAILURE_EXIT(1, "\nError : Could not accept new client\n");

    struct epoll_event event;
    event.events = EPOLLIN | EPOLLPRI;
    event.data.fd = client;

    if (epoll_ctl(epoll, EPOLL_CTL_ADD, client, &event) == -1)
        FAILURE_EXIT(1, "\nError : Could not add new client to epoll\n");
}

void handle_message(int socket) {
    uint8_t message_type;
    uint16_t message_size;

    if(read(socket, &message_type, 1) != 1) FAILURE_EXIT(1,"\nError : Could not read message type\n");
    if(read(socket, &message_size, 2) != 2) FAILURE_EXIT(1,"\nError : Could not read message size\n");
    char* client_name = malloc(message_size);

    switch(message_type){
        case REGISTER:{
            if(read(socket, client_name, message_size)!= message_size)
                FAILURE_EXIT(1,"\nError : Could not read register message name\n");
            register_client(client_name, socket);
            break;
        }
        case UNREGISTER:{
            if(read(socket, client_name, message_size) != message_size)
            FAILURE_EXIT(1,"\nError : Could not read unregister message name\n");
            unregister_client(client_name);
            break;
        }
        case RESULT:{
            result_t result;
            if(read(socket, client_name, message_size) != message_size)
                FAILURE_EXIT(1,"\nError : Could not read result message name\n");
            if(read(socket, &result, sizeof(result_t)) != sizeof(result_t))
                FAILURE_EXIT(1,"\nError : Could not read result message\n");
            printf("Client \"%s\" calculated operation [%d]. Result: %lf\n", client_name, result.op_num, result.value);
            break;
        }
        case PONG:{
            if(read(socket, client_name, message_size) != message_size)
                FAILURE_EXIT(1,"\nError : Could not read PONG message\n");
            pthread_mutex_lock(&clients_mutex);
            int i = in(client_name, clients, (size_t) cn, sizeof(Client), (__compar_fn_t) cmp_name);
            if (i >= 0) clients[i].un_active--;
            pthread_mutex_unlock(&clients_mutex);
            break;
        }
        default:
            printf("Unknown message type\n");
            break;
    }
    free(client_name);
}

void register_client(char *client_name, int socket){
    uint8_t message_type;
    pthread_mutex_lock(&clients_mutex);
    if(cn == CLIENT_MAX){
        message_type = FAILSIZE;
        if (write(socket, &message_type, 1) != 1)
            FAILURE_EXIT(1, "\nError : Could not write FAILSIZE message to client \"%s\"\n", client_name);
        remove_socket(socket);
    } else{
        int exists = in(client_name, clients, (size_t) cn, sizeof(Client), (__compar_fn_t) cmp_name);
        if(exists != -1){
            message_type = FAILNAME;
            if (write(socket, &message_type, 1) != 1)
                FAILURE_EXIT(1, "\nError : Could not write FAILNAME message to client \"%s\"\n", client_name);
            remove_socket(socket);
        } else{
            clients[cn].fd = socket;
            clients[cn].name = malloc(strlen(client_name) + 1);
            clients[cn].un_active = 0;
            strcpy(clients[cn++].name, client_name);
            message_type = SUCCESS;
            if (write(socket, &message_type, 1) != 1)
                FAILURE_EXIT(1, "\nError : Could not write SUCCESS message to client \"%s\"\n", client_name);
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
    remove_socket(clients[i].fd);

    free(clients[i].name);

    cn--;
    for (int j = i; j < cn; ++j)
        clients[j] = clients[j + 1];

}

void remove_socket(int socket){
    if (epoll_ctl(epoll, EPOLL_CTL_DEL, socket, NULL) == -1)
    FAILURE_EXIT(1,"\nError : Could not remove client's socket from epoll\n");

    if (shutdown(socket, SHUT_RDWR) == -1) FAILURE_EXIT(1, "\nError : Could not shutdown client's socket\n");

    if (close(socket) == -1) FAILURE_EXIT(1, "\nError : Could not close client's socket\n");
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
    memset(&web_address, 0, sizeof(struct sockaddr_in));
    web_address.sin_family = AF_INET;
    web_address.sin_addr.s_addr = htonl(INADDR_ANY);
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