#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>

struct sensor_message {
    char type[12];       // Tipo do sensor (temperature, humidity, air_quality)
    int coords[2];       // Coordenadas do sensor (linha, coluna)
    float measurement;   // Medição do sensor
};

void *handle_client(void *arg) {
    int client_socket = *((int *)arg);
    free(arg);
    struct sensor_message msg;

    while (1) {
        memset(&msg, 0, sizeof(msg));  

        int received = recv(client_socket, &msg, sizeof(msg), 0);
        if (received == 0) {
            printf("Cliente desconectado\n");
            break;
        } else if (received == -1) {
            perror("Erro ao receber dados do cliente");
            break;
        }

        printf("Sensor tipo: %s\n", msg.type);
        printf("Coordenadas: (%d, %d)\n", msg.coords[0], msg.coords[1]);
        printf("Medição: %.2f\n", msg.measurement);

        strcpy(msg.type, "air_quality");
        msg.coords[0] = 5;
        msg.coords[1] = 5;
        msg.measurement = 90;

        if (send(client_socket, &msg, sizeof(msg), 0) == -1) {
            perror("Erro ao enviar dados para o cliente");
            break;
        }
    }

    close(client_socket);
    return NULL;
}

int main(int argc, char *argv[]) {
    if (argc != 2) { 
        fprintf(stderr, "Uso: %s <porta>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int server_fd;
    struct sockaddr_in addr;
    int porta = atoi(argv[1]); // Use argv[1] como a porta

    if (porta <= 0) {
        fprintf(stderr, "Porta inválida: %d\n", porta);
        exit(EXIT_FAILURE);
    }

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        perror("Erro ao criar socket");
        exit(EXIT_FAILURE);
    }

    addr.sin_family = AF_INET;
    addr.sin_port = htons(porta);
    addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_fd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
        perror("Erro ao fazer bind");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 10) == -1) {
        perror("Erro ao ouvir");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("Servidor aguardando conexões na porta %d...\n", porta);

    while (1) {
        struct sockaddr_storage client_addr;
        socklen_t addr_size = sizeof(client_addr);
        int *new_socket = malloc(sizeof(int));
        *new_socket = accept(server_fd, (struct sockaddr *)&client_addr, &addr_size);

        if (*new_socket == -1) {
            perror("Erro ao aceitar conexão");
            free(new_socket);
            continue;
        }

        pthread_t client_thread;
        pthread_create(&client_thread, NULL, handle_client, new_socket);
        pthread_detach(client_thread);
    }

    close(server_fd);
    return 0;
}