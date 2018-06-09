#ifndef ZAD1_CLUSTER_H
#define ZAD1_CLUSTER_H

#define UNIX_PATH_MAX    108
#define CLIENT_MAX       64

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
    LOCAL,
    WEB
} connect_type;

typedef struct message_t {
    enum message_type message_type;
    char name[64];
    enum connect_type connect_type;
    int op_num;
    double value;
} message_t;

typedef struct operation_t {
    int op_num;
    char op;
    double arg1;
    double arg2;
} operation_t;

typedef struct Client {
    struct sockaddr* sockaddr;
    socklen_t socklen;
    enum connect_type connect_type;
    char *name;
    uint8_t un_active;
} Client;

#endif //ZAD1_CLUSTER_H
