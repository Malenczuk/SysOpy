#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <string.h>
#include <netinet/in.h>
#include <libnet.h>
#include <sys/un.h>
#include "cluster.h"

#define FAILURE_EXIT(code, format, ...) { fprintf(stderr, format, ##__VA_ARGS__); exit(code);}

void sigHandler(int);

void __init__(char *, char *, char *);

void __del__();

void handle_messages();

void handle_request();

void register_on_server();

void send_message(uint8_t);

int SOCKET;
char *name;

int main(int argc, char **argv) {
    if (argc != 4) FAILURE_EXIT(1, "\nUsage: %s <Client Name> <WEB / LOCAL> <ADDRESS:PORT / Unix Path>\n", argv[0]);
    if (atexit(__del__) == -1) FAILURE_EXIT(1, "\nError : Could not set AtExit\n")

    __init__(argv[1], argv[2], argv[3]);

    register_on_server();

    handle_messages();

    return 0;
}

void handle_messages(){
    uint8_t message_type;
    while(1){
        if(read(SOCKET, &message_type, 1) != 1) FAILURE_EXIT(1,"\nError : Could not read message type\n");
        switch(message_type){
            case REQUEST:
                handle_request();
                break;
            case PING:
                send_message(PONG);
                break;
            default:
                printf("Unknown message type\n");
                break;
        }
    }
}

void handle_request(){
    operation_t operation;
    result_t result;
    char buffer[256];

    if(read(SOCKET, &operation, sizeof(operation_t)) != sizeof(operation_t))
        FAILURE_EXIT(1,"\nError : Could not read request message\n");

    result.op_num = operation.op_num;
    result.value = 0;

    sprintf(buffer, "echo 'scale=6; %lf %c %lf' | bc", operation.arg1, operation.op, operation.arg2);
    FILE* calc = popen(buffer, "r");
    size_t n = fread(buffer, 1, 256, calc);
    pclose(calc);
    buffer[n-1] = '\0';
    sscanf(buffer, "%lf", &result.value);

    send_message(RESULT);
    if(write(SOCKET, &result, sizeof(result_t)) != sizeof(result_t))
        FAILURE_EXIT(1, "\nError : Could not write result message\n");
}

void register_on_server(){
    send_message(REGISTER);

    uint8_t message_type;
    if(read(SOCKET, &message_type, 1) != 1) FAILURE_EXIT(1, "\nError : Could not read response message type\n");

    switch(message_type){
        case FAILNAME:
            FAILURE_EXIT(2, "Name already in use\n");
        case FAILSIZE:
            FAILURE_EXIT(2, "Too many clients logged to the server\n");
        case SUCCESS:
            printf("Logged with SUCCESS\n");
            break;
        default:
            FAILURE_EXIT(1, "\nError : Unpredicted REGISTER behavior\n")
    }
}

void send_message(uint8_t message_type){
    uint16_t message_size = (uint16_t) (strlen(name) + 1);
    if(write(SOCKET, &message_type, 1) != 1) FAILURE_EXIT(1, "\nError : Could not write message type\n");
    if(write(SOCKET, &message_size, 2) != 2) FAILURE_EXIT(1, "\nError : Could not write message size\n");
    if(write(SOCKET, name, message_size) != message_size)
        FAILURE_EXIT(1, "\nError : Could not write name message\n");
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
            strtok(arg3, ":");
            char *arg4 = strtok(NULL, ":");
            if(arg4 == NULL) FAILURE_EXIT(1, "\nError : Wrong IP address with port format\n")

            // Get IP Address
            uint32_t ip = inet_addr(arg3);
            if (ip == -1) FAILURE_EXIT(1, "\nError : Wrong IP address\n");

            // Get Port Number
            uint16_t port_num = (uint16_t) atoi(arg4);
            if (port_num < 1024 || port_num > 65535)
                FAILURE_EXIT(1, "\nError : Wrong Port Number\n");

            // Init Web Socket
            struct sockaddr_in web_address;
            memset(&web_address, 0, sizeof(struct sockaddr_in));

            web_address.sin_family = AF_INET;
            web_address.sin_addr.s_addr = htonl(ip);
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
    send_message(UNREGISTER);
    if (shutdown(SOCKET, SHUT_RDWR) == -1)
        fprintf(stderr, "\nError : Could not shutdown Socket\n");
    if (close(SOCKET) == -1)
        fprintf(stderr, "\nError : Could not close Socket\n");
}