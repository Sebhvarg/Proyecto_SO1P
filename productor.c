#include "publisher.h"
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <sys/wait.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#ifdef __linux__
#include <sys/sysinfo.h>
#include <sys/statvfs.h>
#endif

void start_productor(int port) {
    productor_config_t *config = malloc(sizeof(productor_config_t));
    send_messages(config);
    
}

void *send_messages(void *arg) {
    free(arg);
    int sock = -1;
    struct sockaddr_in broker_addr;
    char buffer[PRODUCTOR_BUFFER_SIZE];
    
    char *broker_ip = getenv("BROKER_IP");
    if (!broker_ip){
        broker_ip = "127.0.0.1";
        fprintf(stderr, "BROKER_IP no establecido, usando %s\n", broker_ip);
    }
    if (getenv("BROKER_PORT")) {
        broker_addr.sin_port = htons(atoi(getenv("BROKER_PORT")));
    } else {
        broker_addr.sin_port = htons(BROKER_PORT);
        fprintf(stderr, "BROKER_PORT no establecido, usando %d\n", BROKER_PORT);
    }
    while (1)
    {
        if (sock < 0) {
           if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
                perror("Error al crear socket");
                sleep(5);
                continue;
            }
            serv_addr.sin_family = AF_INET;
            serv_addr.sin_addr.s_addr = inet_addr(broker_ip);
            struct hostent *server = gethostbyname(broker_ip);
            if(!server){
                fprintf(stderr, "Error: No se pudo resolver la direccion del broker %s\n", broker_ip);
                close(sock);
                sock = -1;
                sleep(5);
                continue;
            }
            bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);

            if(connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
                perror("Error al conectar con el broker");
                close(sock);
                sock = -1;
                sleep(5);
                continue;
        }
        printf("Conectado al broker en %s:%d\n", broker_ip, ntohs(serv_addr.sin_port));
    }
    long long memoria_fisica_usada = 0;
    unsigned long long espacio_disco_usado = 0;

#ifdef __linux__
    struct sysinfo memInfo;
    sysinfo(&memInfo);
    memoria_fisica_usada = memInfo.totalram - memInfo.freeram;

    struct statvfs stat;
    if (statvfs("/", &stat) == 0) {
        espacio_disco_usado = (stat.f_blocks - stat.f_bfree) * stat.f_frsize;
    }
    
}
#endif
    snprintf(buffer, PRODUCTOR_BUFFER_SIZE, "MEMORIA:%lld;DISCO:%llu", memoria_fisica_usada, espacio_disco_usado);
    ssize_t bytes_sent = send(sock, buffer, strlen(buffer), 0);
    if (bytes_sent < 0) {
        perror("Error al enviar datos al broker");
        close(sock);
        sock = -1;
        sleep(5);
        continue;
    }
    printf("Datos enviados al broker: %s\n", buffer);
    sleep(10);
    }       
#ifdef STANDALONE
int main(int argc, char *argv[]) {
    start_productor(0);
    return 0;
}
#endif
