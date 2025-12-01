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
        fprintf(stderr, "Uso: %s <broker_port> <gateway_port> <publisher_port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    int num_p = atoi(argv[1]);

    pthread_t broker_thread;
    if (pthread_create(&broker_thread, NULL, (void *)initialize_broker, NULL) != 0) {
        perror("Fallo en la creacion del hilo del broker");
        exit(EXIT_FAILURE);
    }
    sleep(1); // Esperar a que el broker inicie

    for (int i = 1; i<= (num_p + 4)/5; i++) {
        pthread_t gateway_thread;
        int gateway_port = 9000 + i; // Puertos 9001, 9002, ...
        if (pthread_create(&gateway_thread, NULL, (void *)initialize_gateway, (void *)(intptr_t)gateway_port) != 0) {
            perror("Fallo en la creacion del hilo del gateway");
            exit(EXIT_FAILURE);
        }
        sleep(1); // Esperar a que el gateway inicie
    }
    for (int i = 1; i <= num_p; i++) {
        pthread_t publisher_thread;
        int publisher_port = 10000 + i; // Puertos 10001, 10002, ...
        if (pthread_create(&publisher_thread, NULL, (void *)start_publisher, (void *)(intptr_t)publisher_port) != 0) {
            perror("Fallo en la creacion del hilo del publisher");
            exit(EXIT_FAILURE);
        }
        sleep(1); // Esperar a que el publisher inicie
    }
    pthread_join(broker_thread, NULL);
    return 0;
}
