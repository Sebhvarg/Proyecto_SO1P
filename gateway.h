#ifndef GATEWAY_H
#define GATEWAY_H

#define BUFFER_SIZE 1024

typedef struct {
    int id;
    int client_socket;
} client_info_t;

void initialize_gateway(int port);
void *handle_client_connection(void *arg);

#endif // GATEWAY_H