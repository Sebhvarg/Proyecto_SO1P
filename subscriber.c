#include "subscriber.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

void suscribe_to_topic(const char *topic) {
    int sock = 0;
    struct sockaddr_in serv_addr;
    char buffer[BUFFER_SIZE] = {0};

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\n Error al crear socket \n");
        return;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // Convertir direcciones IPv4 e IPv6 de texto a binario
    if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        printf("\nDireccion invalida\n");
        return;
    }
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("\nConexion fallida \n");
        return;
    }
    
    char subscribe_msg[BUFFER_SIZE];
    snprintf(subscribe_msg, sizeof(subscribe_msg), "SUBSCRIBE %s", topic);
    send(sock , subscribe_msg , strlen(subscribe_msg) , 0 );
    printf("Suscrito al topic: %s\n", topic);
    
    while (1) {
        int valread = read( sock , buffer, BUFFER_SIZE);
        if (valread > 0) {
            buffer[valread] = '\0';
            printf("Mensaje recibido en el tema %s: %s\n", topic, buffer);
        } else if (valread == 0) {
            printf("Servidor cerro la conexion\n");
            break;
        } else {
            perror("Error de lectura");
            break;
        }
    }
    close(sock);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Uso: %s <topic>\n Ejemplo: %s temperatura \n", argv[0], argv[0]);
        return 1;
    }
    const char *topic = argv[1];
    suscribe_to_topic(topic);
    return 0;
}