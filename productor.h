#ifndef PRODUCTOR_H
#define PRODUCTOR_H

#define PRODUCTOR_BUFFER_SIZE 4096
#define MAX_TOPICS 50
#define MAX_VALUE_LENGTH 50
#define MAX_CLIENTS 100
#define BROKER_PORT 8080

typedef struct {
    int publisher_id;
    int gateway_port;
} productor_config_t;

void start_productor(int port);
void *send_messages(void *arg);

#endif // PRODUCTOR_H