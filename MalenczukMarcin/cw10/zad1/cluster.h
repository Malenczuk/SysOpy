#ifndef ZAD1_CLUSTER_H
#define ZAD1_CLUSTER_H

#define UNIX_PATH_MAX    108
#define CLIENT_MAX       64

typedef struct message {
    char op;
    double arg1;
    double arg2;
} message;

typedef struct Client {
    int fd;
    char *name;
} Client;

typedef enum message_type {
    LOGIN = 0
} message_type;

typedef enum connect_type {
    LOCAL = 0, WEB = 1
} connect_type;
#endif //ZAD1_CLUSTER_H
