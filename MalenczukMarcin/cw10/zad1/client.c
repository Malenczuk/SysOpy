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

void __init__(char *, char *, char *);

void __del__();

int SOCKET;
char *name;

int main(int argc, char **argv) {
    if (argc != 4) FAILURE_EXIT(1, "\nUsage: %s <Client Name> <WEB / LOCAL> <Port Number / Unix Path>\n", argv[0]);
    if (atexit(__del__) == -1) FAILURE_EXIT(1, "\nError : Could not set AtExit\n")

    __init__(argv[1], argv[2], argv[3]);


    return 0;
}

void sigHandler(int signo){
    FAILURE_EXIT(2, "\nSIGINT\n");
}

void __init__(char *arg1, char *arg2, char *arg3) {
    // Handle Signal
    signal(SIGINT, sigHandler);

    // Get Client Name
    name = arg1;

    // Init Socket
    switch((strcmp(arg2, "WEB") == 0 ? WEB : strcmp(arg2, "LOCAL") == 0 ? LOCAL : -1)){
        case WEB: {
            // Get Port Number
            uint16_t port_num = (uint16_t) atoi(arg3);
            if (port_num < 1024 || port_num > 65535)
            FAILURE_EXIT(1, "\nError : Wrong Port Number\n");

            // Init Web Socket
            struct sockaddr_in web_address;
            web_address.sin_family = AF_INET;
            web_address.sin_addr.s_addr = INADDR_ANY;
            web_address.sin_port = htons(port_num);

            if ((SOCKET = socket(AF_INET, SOCK_STREAM, 0)) < 0)
                FAILURE_EXIT(1, "\nError : Could not create web socket\n");

            if (connect(SOCKET, (const struct sockaddr *) &web_address, sizeof(web_address)) == -1)
                FAILURE_EXIT(1, "\nError : Could not connect to web socket\n");

            break;
        }
        case LOCAL: {
            // Get Unix Path
            char* unix_path = arg3;
            if (strlen(unix_path) < 1 || strlen(unix_path) > UNIX_PATH_MAX)
            FAILURE_EXIT(1, "\nError : Wrong Unix Path\n");

            // Init Local Socket
            struct sockaddr_un local_address;
            local_address.sun_family = AF_UNIX;
            snprintf(local_address.sun_path, UNIX_PATH_MAX, "%s", unix_path);

            if ((SOCKET = socket(AF_UNIX, SOCK_STREAM, 0)) < 0)
                FAILURE_EXIT(1, "\nError : Could not create local socket\n");

            if (connect(SOCKET, (const struct sockaddr *) &local_address, sizeof(local_address)) == -1)
                FAILURE_EXIT(1, "\nError : Could not connect to local socket\n");

            break;
        }
        default:
            FAILURE_EXIT(1,"\nError : Use WEB or LOCAL\n");
    }

}

void __del__() {
    if (shutdown(SOCKET, SHUT_RDWR) == -1)
        fprintf(stderr, "\nError : Could not shutdown Socket\n");
    if (close(SOCKET) == -1)
        fprintf(stderr, "\nError : Could not close Socket\n");
}