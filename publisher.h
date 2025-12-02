#ifndef PUBLISHER_H
#define PUBLISHER_H

#define PUBLISHER_BUFFER_SIZE 4096
#define MAX_TOPICS 50
#define MAX_VALUE_LENGTH 50
#define MAX_CLIENTS 100

typedef struct {
    int publisher_id;
    int gateway_port;
} publisher_config_t;

void start_publisher(int port);
void *send_messages(void *arg);

#endif // PUBLISHER_H