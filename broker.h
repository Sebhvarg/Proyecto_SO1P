#ifndef BROKER_H
#define BROKER_H
#include <pthread.h>

#define PORT 8080
#define MAX_CLIENTS 100
#define BUFFER_SIZE 2048
#define MAX_VALUE_LENGTH 50
#define MAX_TOPICS 50

typedef struct {
    char topic[MAX_TOPICS];
    char value[MAX_VALUE_LENGTH];
} message_t;

typedef struct subscriber {
    int socket;
    char topics[MAX_TOPICS];
    
    struct subscriber *next;
} subscriber_t;

void initialize_broker();
void *handle_client(void *arg);
void publish_message(const char *topic, const char *value);

#endif // BROKER_H