#include "broker.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <time.h>


pthread_mutex_t msg_mutex;
subscriber_t *subscribers_head = NULL;
pthread_mutex_t subscribers_mutex;

void initialize_broker()
{
    int server_socket, new_socket;
    struct sockaddr_in server_addr;
    int addr_len = sizeof(server_addr);
    pthread_t thread_id;

    pthread_mutex_init(&log_mutex, NULL);
    pthread_mutex_init(&subscribers_mutex, NULL);

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    int opt = 1;
    setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);
    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("Error en bind");
        exit(EXIT_FAILURE);
    }
    if (listen(server_socket, 3) < 0)
    {
        perror("Error en listen");
        exit(EXIT_FAILURE);
    }
    printf("Broker escuchando en el puerto %d\n", port);

    while (1)
    {
        new_socket = accept(server_socket, (struct sockaddr *)&server_addr, (socklen_t *)&addr_len);
        if (new_socket < 0)
        {
            perror("Error en accept");
            continue;
        }
        printf("Nuevo cliente conectado\n");
        int *pclient = malloc(sizeof(int));
        *pclient = new_socket;
        pthread_create(&thread_id, NULL, handle_client, pclient);
        pthread_detach(thread_id);
    }
}
int match_topic(const char *subscribed_topics, const char *topic)
{
    if (strstr(subscribed_topics, "#") != NULL)
    {
        size_t len = strlen(subscribed_topics);
        if (len > 2 && subscribed_topics[len - 2] == '/' && subscribed_topics[len - 1] == '#')
        {
            char base_topic[MAX_TOPICS];
            strncpy(base_topic, subscribed_topics, len - 2);
            base_topic[len - 2] = '\0';
            if (strncmp(topic, base_topic, strlen(base_topic)) == 0)
            {
                if (topic[strlen(base_topic)] == '/' || topic[strlen(base_topic)] == '\0')
                {
                    return 1;
                }
            }
        }
        return 0;
    }
    return strcmp(subscribed_topics, topic) == 0;
}
void save_message_log(const char *topic, const char *value)
{
    pthread_mutex_lock(&log_mutex);
    FILE *log_file = fopen(MSG_LOG_FILE, "a");
    if (log_file != NULL)
    {
        time_t now = time(NULL);
        char timestamp[30];
        strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", localtime(&now));
        fprintf(log_file, "[%s] Topic: %s, Value: %s\n", timestamp, topic, value);
        fclose(log_file);
    }
    else
    {
        perror("Error al abrir el archivo de log");
    }
    pthread_mutex_unlock(&log_mutex);
}

void notify_subscribers(const char *topic, const char *value)
{
    pthread_mutex_lock(&subscribers_mutex);
    subscriber_t *current = subscribers_head;
    while (current != NULL)
    {
        if (match_topic(current->topics, topic))
        {
            send(current->socket, value, strlen(value), 0);
        }
        current = current->next;
    }
    pthread_mutex_unlock(&subscribers_mutex);
}

void publish_message(const char *topic, const char *value)
{
    printf("Publicando mensaje en el tema: %s, valor: %s\n", topic, value);
    save_message_log(topic, value);
    char notify_buffer[BUFFER_SIZE];
    snprintf(notify_buffer, BUFFER_SIZE, "Nuevo mensaje en %s: %s", topic, value);
    notify_subscribers(topic, value);
}

void *handle_client(void *arg)
{
    int client_socket = *((int *)arg);
    free(arg);
    char buffer[BUFFER_SIZE];
    while (1)
    {
        memset(buffer, 0, BUFFER_SIZE);
        int bytes_received = recv(client_socket, buffer, BUFFER_SIZE - 1, 0);
        if (bytes_received <= 0)
        {
            printf("Cliente desconectado\n");
            close(client_socket);
            return NULL;
        }
        buffer[bytes_received] = '\0';
        printf("Mensaje recibido: %s\n", buffer);
        if (strncmp(buffer, "PUBLICADO:", 10) == 0)
        {
            char *p = buffer + 10;
            char *topic = strtok(p, ":");
            char *value = strtok(NULL, ":");
            if (topic != NULL && value != NULL)
            {
                publish_message(topic, value);
            }
        }
        else if (strncmp(buffer, "SUSCRIBIR:", 10) == 0)
        {
            char topics[MAX_TOPICS];
            sscanf(buffer + 10, "%s", topics);
            subscriber_t *new_subscriber = (subscriber_t *)malloc(sizeof(subscriber_t));
            new_subscriber->socket = client_socket;
            strncpy(new_subscriber->topics, topics, MAX_TOPICS);
            new_subscriber->next = NULL;

            pthread_mutex_lock(&subscribers_mutex);
            new_subscriber->next = subscribers_head;
            subscribers_head = new_subscriber;
            pthread_mutex_unlock(&subscribers_mutex);

            printf("Cliente suscrito a los temas: %s\n", topics);
        }
    }
    close(client_socket);
    return NULL;
}

int main(int argc, char *argv[])
{
    int port = PORT;
    if(argc > 1) {
        port = atoi(argv[1]);
    }
    initialize_broker();
    return 0;
}
