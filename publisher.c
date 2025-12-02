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
    (void)port;
    // This function is deprecated - publishers now connect to gateways directly
    printf("Warning: start_publisher() called but publishers now use send_messages() directly\n");
}

void *send_messages(void *arg) {
    publisher_config_t *config = (publisher_config_t *)arg;
    int publisher_id = config->publisher_id;
    int gateway_port = config->gateway_port;
    free(config);

    int gateway_socket;
    struct sockaddr_in gateway_addr;
    char buffer[PUBLISHER_BUFFER_SIZE];
    srand(time(NULL) + publisher_id); // Semilla para valores aleatorios

    printf("Publisher %d iniciado, conectando al Gateway en puerto %d\n", publisher_id, gateway_port);

    // Conectar al gateway
    while (1) {
        gateway_socket = socket(AF_INET, SOCK_STREAM, 0);
        if (gateway_socket < 0) {
            perror("Error creando socket");
            sleep(2);
            continue;
        }

        gateway_addr.sin_family = AF_INET;
        gateway_addr.sin_port = htons(gateway_port);
        inet_pton(AF_INET, "127.0.0.1", &gateway_addr.sin_addr);

        if (connect(gateway_socket, (struct sockaddr *)&gateway_addr, sizeof(gateway_addr)) < 0) {
            printf("Publisher %d: esperando al Gateway en puerto %d...\n", publisher_id, gateway_port);
            close(gateway_socket);
            sleep(2);
            continue;
        }

        printf("Publisher %d: conectado al Gateway en puerto %d\n", publisher_id, gateway_port);
        break;
    }

    // Enviar mensajes continuamente como ESP32
    int temperature = 22 + (rand() % 10);  // 22-31°C
    int humidity = 45 + (rand() % 20);     // 45-64%

    while (1) {
        // Simular variación de temperatura
        temperature = 22 + (rand() % 10);
        snprintf(buffer, sizeof(buffer), "publisher%d/temperature %d°C\n", publisher_id, temperature);
        
        if (send(gateway_socket, buffer, strlen(buffer), 0) < 0) {
            printf("Publisher %d: Error enviando temperatura, reconectando...\n", publisher_id);
            close(gateway_socket);
            sleep(2);
            // Intentar reconectar
            gateway_socket = socket(AF_INET, SOCK_STREAM, 0);
            gateway_addr.sin_family = AF_INET;
            gateway_addr.sin_port = htons(gateway_port);
            inet_pton(AF_INET, "127.0.0.1", &gateway_addr.sin_addr);
            connect(gateway_socket, (struct sockaddr *)&gateway_addr, sizeof(gateway_addr));
            continue;
        }
        printf("Publisher %d TX: %s", publisher_id, buffer);
        sleep(1);

        // Simular variación de humedad
        humidity = 45 + (rand() % 20);
        snprintf(buffer, sizeof(buffer), "publisher%d/humidity %d%%\n", publisher_id, humidity);
        
        if (send(gateway_socket, buffer, strlen(buffer), 0) < 0) {
            printf("Publisher %d: Error enviando humedad, reconectando...\n", publisher_id);
            close(gateway_socket);
            sleep(2);
            continue;
        }
        printf("Publisher %d TX: %s", publisher_id, buffer);
        sleep(4); // Esperar antes del siguiente ciclo
    }

    close(gateway_socket);
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
