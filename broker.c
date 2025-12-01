#include "broker.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include "gateway.h"

message_t msg[MAX_CLIENTS];
int msg_count = 0;
pthread_mutex_t msg_mutex;
subscriber_t *subscribers_head = NULL;
pthread_mutex_t subscribers_mutex;

void initialize_broker() {
    int server_socket, new_socket;
    struct sockaddr_in server_addr;
    int addr_len = sizeof(server_addr);
    pthread_t thread_id;
    pthread_mutex_init(&msg_mutex, NULL);
    pthread_mutex_init(&subscribers_mutex, NULL);

    // Crear el socket del servidor
    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Creacion del socket fallida");
        exit(EXIT_FAILURE);
    }
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);
    // Enlazar el socket
    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Fallo en el bind");
        close(server_socket);
        exit(EXIT_FAILURE);
    }
    // Escuchar conexiones entrantes
    if (listen(server_socket, MAX_CLIENTS) < 0) {
        perror("Fallo en el listen");
        close(server_socket);
        exit(EXIT_FAILURE);
    }
    printf("Broker escuchando en el puerto %d\n", PORT);
    while (1)
    {
        if ((new_socket = accept(server_socket, (struct sockaddr *)&server_addr, (socklen_t*)&addr_len))<0) {
            perror("Fallo en el accept");
            continue;
        }
        int *client_socket = malloc(sizeof(int));
        *client_socket = new_socket;

        if (pthread_create(&thread_id, NULL, handle_client, (void *)client_socket) != 0) {
            perror("Fallo en la creacion del hilo");
            free(client_socket);
        }
        pthread_detach(thread_id); // Desvincular el hilo para que libere recursos al terminar
    }
    
}

void notify_subscribers(const char *topic, const char *value);

void publish_message(const char *topic, const char *value) {
    pthread_mutex_lock(&msg_mutex);
    if (msg_count < MAX_CLIENTS) {
        strncpy(msg[msg_count].topic, topic, MAX_TOPICS);
        strncpy(msg[msg_count].value, value, MAX_VALUE_LENGTH);
        msg_count++;
        printf("Mensaje publicado en el broker: Topic: %s, Value: %s\n", topic, value);
    } else {
        printf("Broker lleno, no se puede publicar mas mensajes\n");
    }
    pthread_mutex_unlock(&msg_mutex);
    notify_subscribers(topic, value);
}

void notify_subscribers(const char *topic, const char *value) {
    pthread_mutex_lock(&subscribers_mutex);
    subscriber_t *current = subscribers_head;
    while (current != NULL) {
        if (strstr(current->topics, topic) != NULL) {
            send(current->socket, value, strlen(value), 0);
        }
        current = current->next;
    }
    pthread_mutex_unlock(&subscribers_mutex);
}

void* handle_client(void *arg) {
    int client_socket = *((int *)arg);
    free(arg);
    char buffer[BUFFER_SIZE];
    char topic[MAX_TOPICS];
    char value[MAX_VALUE_LENGTH];

    while (1) {
        memset(buffer, 0, BUFFER_SIZE);
        int bytes_read = recv(client_socket, buffer, BUFFER_SIZE - 1, 0);
        if (bytes_read <= 0) {
            printf("Cliente desconectado\n");
            close(client_socket);
            return NULL;
        }
        buffer[bytes_read] = '\0';

        if (strncmp(buffer, "PUBLISH", 7) == 0) {
            sscanf(buffer + 8, "%s %s", topic, value);
            publish_message(topic, value);
        } else if (strncmp(buffer, "SUBSCRIBE", 9) == 0) {
            sscanf(buffer + 10, "%s", topic);
            subscriber_t *new_subscriber = malloc(sizeof(subscriber_t));
            new_subscriber->socket = client_socket;
            strncpy(new_subscriber->topics, topic, MAX_TOPICS);
            new_subscriber->next = NULL;

            pthread_mutex_lock(&subscribers_mutex);
            new_subscriber->next = subscribers_head;
            subscribers_head = new_subscriber;
            pthread_mutex_unlock(&subscribers_mutex);

            printf("Cliente suscrito al tema: %s\n", topic);
        }
    }
    close(client_socket);
    return NULL;
}
