#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <time.h>
#include <math.h>

struct sensor_message {
    char type[12];
    int coords[2];
    float measurement;
};

float calculate_distance(int x1, int y1, int x2, int y2) {
    return sqrt(pow(x2 - x1, 2) + pow(y2 - y1, 2));
}

float update_measurement(float current_measurement, float remote_measurement, float distance) {
    return current_measurement + 0.1 * (1 / (distance + 1)) * (remote_measurement - current_measurement);
}

void print_usage() {
    printf("Usage: ./client <server_ip> <port> -type <temperature|humidity|air_quality> -coords <x> <y>\n");
}

float rand_meassurement_by_type(const char *type) {
    if (strcmp(type, "temperature") == 0) return 20.0 + (rand() % 210) / 10.0;
    if (strcmp(type, "humidity") == 0) return 30.0 + (rand() % 700) / 10.0;
    if (strcmp(type, "air_quality") == 0) return 50.0 + (rand() % 500) / 10.0;
    return 0.0;
}

int main(int argc, char *argv[]) {
    if (argc != 8) {
        fprintf(stderr, "Quantidade de argumentos invalida");
        print_usage();
        exit(EXIT_FAILURE);
    }

    if (strcmp(argv[3], "-type") != 0) {
        fprintf(stderr, "Error: Expected '-type' argument\n");
        print_usage();
        exit(EXIT_FAILURE);
    }
    
    char *sensor_type = argv[4];
    if (strcmp(sensor_type, "temperature") != 0 && strcmp(sensor_type, "humidity") != 0 && strcmp(sensor_type, "air_quality") != 0) {
        fprintf(stderr, "Error: Invalid sensor type\n");
        print_usage();
        exit(EXIT_FAILURE);
    }
    
     if (strcmp(argv[5], "-coords") != 0) {
        fprintf(stderr, "Error: Expected '-coords' argument\n");
        print_usage();
        exit(EXIT_FAILURE);
     }

    int x = atoi(argv[6]);
    int y = atoi(argv[7]);
    if (x < 0 || x > 9 || y < 0 || y > 9) {
        fprintf(stderr, "Error: Coordinates must be in the range 0-9\n");
        print_usage();
        exit(EXIT_FAILURE);
    }

    int sockfd;
    struct addrinfo hints, *res;
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    getaddrinfo(argv[1], argv[2], &hints, &res);
    sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    connect(sockfd, res->ai_addr, res->ai_addrlen);
    freeaddrinfo(res);

    srand(time(NULL));

    struct sensor_message msg;
    strncpy(msg.type, sensor_type, sizeof(msg.type) - 1);
    msg.coords[0] = x;
    msg.coords[1] = y;
    msg.measurement = rand_meassurement_by_type(msg.type);

    send(sockfd, &msg, sizeof(msg), 0);
    printf("Conectado ao servidor. Aguardando mensagens...\n");

    while (1) {
        int received = recv(sockfd, &msg, sizeof(msg), 0);
        if (received > 0) {
            printf("log:\n%s sensor in (%d, %d)\nmeasurement: %.2f\naction: received update\n\n",
                   msg.type, msg.coords[0], msg.coords[1], msg.measurement);
        }
    }

    close(sockfd);
    return 0;
}
