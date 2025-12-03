#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include "broker.h"
#include "gateway.h"
#include "publisher.h"

void initialize_broker();
void initialize_gateway(int port);
void start_publisher(int port);

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Uso: %s <num_publishers>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    int num_p = atoi(argv[1]);

    pthread_t broker_thread;
    if (pthread_create(&broker_thread, NULL, (void *)initialize_broker, NULL) != 0) {
        perror("Fallo en la creacion del hilo del broker");
        exit(EXIT_FAILURE);
    }
    sleep(1); // Esperar a que el broker inicie

    // Crear 1 gateway por cada 3 publishers (incluyendo el reservado ID 1)
    int total_publishers = num_p + 1; // +1 por el ESP32 (ID 1)
    int num_gateways = (total_publishers + 2) / 3;  // Ceiling division
    for (int i = 1; i <= num_gateways; i++) {
        pthread_t gateway_thread;
        int gateway_port = 9000 + i; // Puertos 9001, 9002, ...
        if (pthread_create(&gateway_thread, NULL, (void *)initialize_gateway, (void *)(intptr_t)gateway_port) != 0) {
            perror("Fallo en la creacion del hilo del gateway");
            exit(EXIT_FAILURE);
        }
        printf("Gateway %d creado en puerto %d\n", i, gateway_port);
        sleep(1); // Esperar a que el gateway inicie
    }
    
    // Crear publishers simulados (IDs 2, 3, ...) y asignarlos a gateways
    for (int i = 1; i <= num_p; i++) {
        pthread_t publisher_thread;
        
        int publisher_id = i + 1; // IDs comienzan en 2 (1 reservado para ESP32)
        
        // Calcular a qué gateway debe conectarse este publisher
        int gateway_index = (publisher_id - 1) / 3 + 1; 
        int gateway_port = 9000 + gateway_index;
        
        // Crear configuración para el publisher
        publisher_config_t *config = malloc(sizeof(publisher_config_t));
        config->publisher_id = publisher_id;
        config->gateway_port = gateway_port;
        
        if (pthread_create(&publisher_thread, NULL, send_messages, (void *)config) != 0) {
            perror("Fallo en la creacion del hilo del publisher");
            free(config);
            exit(EXIT_FAILURE);
        }
        printf("Publisher %d creado, asignado a Gateway en puerto %d\n", publisher_id, gateway_port);
        pthread_detach(publisher_thread);
        sleep(1); // Esperar a que el publisher inicie
    }
    pthread_join(broker_thread, NULL);
    return 0;
}
