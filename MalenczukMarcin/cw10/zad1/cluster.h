#ifndef ZAD1_CLUSTER_H
#define ZAD1_CLUSTER_H

#define UNIX_PATH_MAX    108
#define CLIENT_MAX       64

typedef struct result_t {
    int op_num;
    int value;
} result_t;

typedef struct operation_t {
    int op_num;
    char op;
    int arg1;
    int arg2;
} operation_t;

typedef struct Client {
    int fd;
    char *name;
    uint8_t unactive;
} Client;

typedef enum message_type {
    REGISTER,
    UNREGISTER,
    SUCCESS,
    FAILSIZE,
    FAILNAME,
    REQUEST,
    RESULT,
    PING,
    PONG,
} message_type;

typedef enum connect_type {
    LOCAL, WEB
} connect_type;
#endif //ZAD1_CLUSTER_H
