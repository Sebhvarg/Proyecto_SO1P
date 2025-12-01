#include "publisher.h"
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <sys/wait.h>
#include <stdio.h>
#include "gateway.h"
#include <string.h>
#include <time.h>

void start_publisher(int port) {
    int server_socket, new_socket;
    struct sockaddr_in server_addr;
    int addr_len = sizeof(server_addr);
    pthread_t thread_id;

    // Crear el socket del servidor
    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Creacion del socket fallida");
        exit(EXIT_FAILURE);
    }
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

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
    printf("Publisher escuchando en el puerto %d\n", port);

    while (1)
    {
        if ((new_socket = accept(server_socket, (struct sockaddr *)&server_addr, (socklen_t*)&addr_len))<0) {
            perror("Fallo en el accept");
            continue;
        }
        int *client_socket = malloc(sizeof(int));
        *client_socket = new_socket;

        if (pthread_create(&thread_id, NULL, send_messages, (void *)client_socket) != 0) {
            perror("Fallo en la creacion del hilo");
            free(client_socket);
        }
        pthread_detach(thread_id); // Desvincular el hilo para que libere recursos al terminar
    }
}

void *send_messages(void *arg) {
    int client_socket = *((int *)arg);
    free(arg);
    char buffer[PUBLISHER_BUFFER_SIZE];
    srand(time(NULL) + client_socket); // Semilla para valores aleatorios

    while (1) {
        // Simular la creacion de un mensaje
        char topic[MAX_TOPICS];
        char value[MAX_VALUE_LENGTH];
        snprintf(topic, sizeof(topic), "topic_%d", rand() % 10);
        snprintf(value, sizeof(value), "value_%d", rand() % 100);

        // Formatear el mensaje
        snprintf(buffer, sizeof(buffer), "%s:%s", topic, value);

        // Enviar el mensaje al broker
        send(client_socket, buffer, strlen(buffer), 0);
        printf("Mensaje enviado: %s\n", buffer);

        sleep(2); // Esperar antes de enviar el siguiente mensaje
    }

    close(client_socket);
    return NULL;
}

#ifdef STANDALONE
int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Uso: %s <puerto>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    int port = atoi(argv[1]);
    start_publisher(port);
    return 0;
}
#endif
