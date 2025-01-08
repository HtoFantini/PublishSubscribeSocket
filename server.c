#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <time.h>
#include <netdb.h>

#define MAX_CLIENTS 100
#define TEMPERATURE_UPDATE_TIME 5
#define HUMIDITY_UPDATE_TIME 7
#define AIR_QUALITY_UPDATE_TIME 10

struct sensor_message {
    char type[12];
    int coords[2];
    float measurement;
};

struct client_info {
    int socket;
    char type[12];
};

struct client_info clients[MAX_CLIENTS];
pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;

// Envia atualizações para todos os clientes a cada 5 segundos
void *periodic_updates(void *arg) {
    while (1) {
        sleep(5);
        pthread_mutex_lock(&clients_mutex);
        for (int i = 0; i < MAX_CLIENTS; ++i) {
            if (clients[i].socket != 0) {
                struct sensor_message msg;
                strcpy(msg.type, clients[i].type);
                msg.measurement = rand() % 100;
                send(clients[i].socket, &msg, sizeof(msg), 0);
                printf("Atualização enviada para %s\n", clients[i].type);
            }
        }
        pthread_mutex_unlock(&clients_mutex);
    }
}

void *handle_client(void *arg) {
    int client_socket = *((int *)arg);
    free(arg);
    struct sensor_message msg;

    if (recv(client_socket, &msg, sizeof(msg), 0) <= 0) {
        close(client_socket);
        return NULL;
    }

    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < MAX_CLIENTS; ++i) {
        if (clients[i].socket == 0) {
            clients[i].socket = client_socket;
            strcpy(clients[i].type, msg.type);
            break;
        }
    }
    pthread_mutex_unlock(&clients_mutex);

    while (1) {
        memset(&msg, 0, sizeof(msg));
        int received = recv(client_socket, &msg, sizeof(msg), 0);
        if (received <= 0) {
            close(client_socket);
            break;
        }

        printf("log:\n%s sensor in (%d, %d)\nmeasurement: %.2f\n\n",
               msg.type, msg.coords[0], msg.coords[1], msg.measurement);
    }

    close(client_socket);
    return NULL;
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <v4|v6> <porta>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    srand(time(NULL));

    int server_fd;
    struct sockaddr_in6 addr6;
    struct sockaddr_in addr4;
    int porta = atoi(argv[2]);
    int family = 0;

    if (strcmp(argv[1], "v4") == 0) {
        family = AF_INET;
        server_fd = socket(AF_INET, SOCK_STREAM, 0);
        addr4.sin_family = AF_INET;
        addr4.sin_port = htons(porta);
        addr4.sin_addr.s_addr = INADDR_ANY;
        bind(server_fd, (struct sockaddr*)&addr4, sizeof(addr4));
    } else if (strcmp(argv[1], "v6") == 0) {
        family = AF_INET6;
        server_fd = socket(AF_INET6, SOCK_STREAM, 0);
        addr6.sin6_family = AF_INET6;
        addr6.sin6_port = htons(porta);
        addr6.sin6_addr = in6addr_any;
        bind(server_fd, (struct sockaddr*)&addr6, sizeof(addr6));
    } else {
        fprintf(stderr, "Opção inválida. Use: %s <v4|v6> <porta>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    listen(server_fd, 10);

    pthread_t update_thread;
    pthread_create(&update_thread, NULL, periodic_updates, NULL);

    printf("Servidor aguardando conexões na porta %d usando %s...\n", porta, (family == AF_INET) ? "IPv4" : "IPv6");

    while (1) {
        struct sockaddr_storage client_addr;
        socklen_t addr_size = sizeof(client_addr);
        int *new_socket = malloc(sizeof(int));
        *new_socket = accept(server_fd, (struct sockaddr *)&client_addr, &addr_size);

        pthread_t client_thread;
        pthread_create(&client_thread, NULL, handle_client, new_socket);
        pthread_detach(client_thread);
    }

    close(server_fd);
    return 0;
}
