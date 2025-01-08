#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <time.h>

void print_usage(){
    printf("Usage: ./client <server_ip> <port> -type <temperature|humidity|air_quality> -coords <x> <y>\n");
}

struct sensor_message {
    char type[12];       // Tipo do sensor (temperature, humidity, air_quality)
    int coords[2];       // Coordenadas do sensor (linha, coluna)
    float measurement;   // Medição do sensor
};

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

    int status = getaddrinfo(argv[1], argv[2], &hints, &res);
    if (status != 0) {
        fprintf(stderr, "Erro no getaddrinfo: %s\n", gai_strerror(status));
        exit(EXIT_FAILURE);
    }

    sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (sockfd == -1) {
        perror("Erro ao criar socket");
        freeaddrinfo(res);
        exit(EXIT_FAILURE);
    }

    if (connect(sockfd, res->ai_addr, res->ai_addrlen) < 0) {
        perror("Erro ao conectar ao servidor");
        freeaddrinfo(res);
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    freeaddrinfo(res);
    printf("Conectado ao servidor\n");
    
    srand(time(NULL));

    struct sensor_message msg;
    strncpy(msg.type, sensor_type, sizeof(msg.type) - 1); 
    msg.coords[0] = x;
    msg.coords[1] = y;
    msg.measurement = 20.0 + ((rand() % 210) / 10.0);


    // Enviar a mensagem
    if (send(sockfd, &msg, sizeof(msg), 0) == -1) {
        perror("Erro ao enviar dados ao servidor");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    // Receber a resposta do servidor
    int received = recv(sockfd, &msg, sizeof(msg), 0);
    if (received <= 0) {
        perror("Servidor desconectado");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    printf("Resposta do servidor:\n");
    printf("Sensor tipo: %s\n", msg.type);
    printf("Coordenadas: (%d, %d)\n", msg.coords[0], msg.coords[1]);
    printf("Medição: %.2f\n", msg.measurement);




    // TESTE MANDANDO MSG DE OI
    // while (1) {
    //     printf("Digite 'Oi' para enviar ao servidor: ");
    //     fgets(buffer, sizeof(buffer), stdin);
    //     buffer[strcspn(buffer, "\n")] = '\0';  // Remover a nova linha da entrada

    //     if (send(sockfd, buffer, strlen(buffer), 0) == -1) {
    //         perror("Erro ao enviar dados ao servidor");
    //         break;
    //     }

    //     int received = recv(sockfd, buffer, sizeof(buffer), 0);
    //     if (received <= 0) {
    //         perror("Erro ao receber dados do servidor");
    //         break;
    //     }

    //     printf("Servidor respondeu: %s\n", buffer);
    // }

    close(sockfd);
    return 0;
}
