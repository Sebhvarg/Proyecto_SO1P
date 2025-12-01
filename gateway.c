#include "gateway.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include "broker.h"

void initialize_gateway(int port) {
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
    printf("Gateway TCP escuchando en el puerto %d\n", port);

    while (1)
    {
        if ((new_socket = accept(server_socket, (struct sockaddr *)&server_addr, (socklen_t*)&addr_len))<0) {
            perror("Fallo en el accept");
            continue;
        }
        client_info_t *client_info = malloc(sizeof(client_info_t));
        client_info->client_socket = new_socket;
        client_info->id = new_socket; // Usar el socket como ID temporal

        if (pthread_create(&thread_id, NULL, handle_client_connection, (void *)client_info) != 0) {
            perror("Fallo en la creacion del hilo");
            free(client_info);
        }
        pthread_detach(thread_id); // Desvincular el hilo para que libere recursos al terminar
    }
}

void *handle_client_connection(void *arg) {
    client_info_t *client_info = (client_info_t *)arg;
    int client_socket = client_info->client_socket;
    char buffer[GATEWAY_BUFFER_SIZE];
    char topic[MAX_TOPICS];
    char value[MAX_VALUE_LENGTH];
    ssize_t read_size;
    
    printf("Cliente TCP conectado: ID %d\n", client_info->id);

    // Leer datos TCP raw (el ESP32 puede enviar múltiples líneas)
    while ((read_size = recv(client_socket, buffer, GATEWAY_BUFFER_SIZE - 1, 0)) > 0) {
        buffer[read_size] = '\0';
        
        // Procesar cada línea recibida
        char *line = strtok(buffer, "\n");
        while (line != NULL) {
            // Eliminar carriage return si existe
            char *cr = strchr(line, '\r');
            if (cr) *cr = '\0';
            
            // Parsear el formato: "topic value"
            if (sscanf(line, "%s %[^\n]", topic, value) == 2) {
                printf("Mensaje TCP recibido del cliente %d: Topic: %s, Value: %s\n", 
                       client_info->id, topic, value);
                
                // Publicar el mensaje en el broker
                publish_message(topic, value);

                // Enviar respuesta TCP simple
                const char *response = "OK\n";
                send(client_socket, response, strlen(response), 0);
                
                // Notificar al broker
                int broker_socket = socket(AF_INET, SOCK_STREAM, 0);
                struct sockaddr_in broker_addr;
                broker_addr.sin_family = AF_INET;
                broker_addr.sin_port = htons(PORT);
                inet_pton(AF_INET, "127.0.0.1", &broker_addr.sin_addr);
                
                if (connect(broker_socket, (struct sockaddr *)&broker_addr, sizeof(broker_addr)) < 0) {
                    perror("Fallo en la conexion al broker");
                    close(broker_socket);
                } else {
                    char broker_msg[GATEWAY_BUFFER_SIZE];
                    snprintf(broker_msg, sizeof(broker_msg), "PUBLISH %s %s", topic, value);
                    send(broker_socket, broker_msg, strlen(broker_msg), 0);
                    close(broker_socket);
                }
            } else if (strlen(line) > 0) {
                printf("Formato TCP invalido del cliente %d: %s\n", client_info->id, line);
                const char *response = "ERROR\n";
                send(client_socket, response, strlen(response), 0);
            }
            
            line = strtok(NULL, "\n");
        }
    }

    // Cliente desconectado
    if (read_size == 0) {
        printf("Cliente TCP desconectado: ID %d\n", client_info->id);
    } else {
        printf("Error de lectura del cliente TCP: ID %d\n", client_info->id);
    }
    
    close(client_socket);
    free(client_info);
    pthread_exit(NULL);
    return NULL;
}