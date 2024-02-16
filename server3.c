#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <process.h>

#define PORT 8080
#define MAXLINE 1024
#define MAX_CLIENTS 5

#pragma comment(lib, "ws2_32.lib")

struct ClientInfo {
    SOCKET sockfd;
    struct sockaddr_in cliaddr;
};

void handleClient(void* clientInfo);

int main() {
    WSADATA wsa;
    SOCKET sockfd;
    struct sockaddr_in servaddr;
    struct ClientInfo clientInfos[MAX_CLIENTS];

    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        perror("WSAStartup failed");
        exit(EXIT_FAILURE);
    }

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd == INVALID_SOCKET) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(PORT);

    if (bind(sockfd, (const struct sockaddr*)&servaddr, sizeof(servaddr)) == SOCKET_ERROR) {
        perror("Binding failed");
        exit(EXIT_FAILURE);
    }

    int len = sizeof(struct sockaddr_in);

    for (int i = 0; i < MAX_CLIENTS; ++i) {
        printf("Waiting for client connection %d...\n", i + 1);

        // Receive a message from the client to establish connection
        char buffer[MAXLINE];
        int n = recvfrom(sockfd, buffer, MAXLINE, 0, (struct sockaddr*)&clientInfos[i].cliaddr, &len);
        if (n == SOCKET_ERROR) {
            perror("Error in receiving connection request");
            exit(EXIT_FAILURE);
        }

        printf("Connection established with client %d\n", i + 1);
        clientInfos[i].sockfd = socket(AF_INET, SOCK_DGRAM, 0);

        // Create a thread to handle each client
        _beginthread(handleClient, 0, (void*)&clientInfos[i]);
    }

    // Wait for all threads to finish
    Sleep(3000); // Adjust this time based on your needs

    printf("Server exiting.\n");
    closesocket(sockfd);
    WSACleanup();
    return 0;
}

void handleClient(void* clientInfo) {
    struct ClientInfo* info = (struct ClientInfo*)clientInfo;
    SOCKET sockfd = info->sockfd;
    struct sockaddr_in cliaddr = info->cliaddr;
    int len=sizeof(struct sockaddr_in);

    FILE* fp = fopen("example.txt", "r");
    if (fp == NULL) {
        printf("File open error\n");
        exit(EXIT_FAILURE);
    }

    fseek(fp, 0, SEEK_END);
    long fsize = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    char* file_buffer = malloc(fsize);
    fread(file_buffer, fsize, 1, fp);
    fclose(fp);

    int window_size = 3;
    int base = 0;

    while (base < fsize) {
        for (int i = base; i < base + window_size && i < fsize; i++) {
            char packet[MAXLINE];
            sprintf(packet, "%d:%c", i, file_buffer[i]);
            sendto(sockfd, (const char*)packet, strlen(packet), 0, (const struct sockaddr*)&cliaddr, sizeof(cliaddr));
            printf("Sent packet %d to client\n", i);
        }

        char ack[MAXLINE];
        recvfrom(sockfd, ack, MAXLINE, 0, (struct sockaddr*)&cliaddr, &len);
        int ack_num = atoi(ack);
        printf("Received ACK %d from client\n", ack_num);

        if (ack_num >= base) {
            base = ack_num + 1;
        }
    }

    printf("File transfer complete for client\n");
    closesocket(sockfd);
    free(file_buffer);
}
