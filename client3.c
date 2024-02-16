#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>

#define PORT 8080
#define MAXLINE 1024

#pragma comment(lib, "ws2_32.lib")

int main() {
    WSADATA wsa;
    SOCKET sockfd;
    char buffer[MAXLINE];
    struct sockaddr_in servaddr;

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
    servaddr.sin_port = htons(PORT);
    inet_pton(AF_INET, "127.0.0.1", &servaddr.sin_addr);

    int len = sizeof(servaddr);

    // Send a message to the server to establish the connection
    const char* connect_msg = "Connect"; // Message to establish connection
    sendto(sockfd, connect_msg, strlen(connect_msg), 0, (struct sockaddr*)&servaddr, len);

    FILE* fp = fopen("sample2.txt", "w");
    if (fp == NULL) {
        printf("File open error\n");
        exit(EXIT_FAILURE);
    }

    int expected_seq_num = 0;

    while (1) {
        char packet[MAXLINE];

        // Receive a packet from the server
        int bytes_received = recvfrom(sockfd, packet, MAXLINE, 0, (struct sockaddr*)&servaddr, &len);
        if (bytes_received == SOCKET_ERROR) {
            perror("Error in receiving packet");
            break;
        }

        int seq_num = atoi(strtok(packet, ":"));

        if (seq_num == expected_seq_num) {
            char* data = strtok(NULL, ":");

            // Write received data to file
            fwrite(data, strlen(data), 1, fp);
            printf("Received packet %d\n", seq_num);
            expected_seq_num++;
        }

        char ack[MAXLINE];
        sprintf(ack, "%d", expected_seq_num - 1);

        // Send acknowledgment back to the server
        int bytes_sent = sendto(sockfd, ack, strlen(ack), 0, (struct sockaddr*)&servaddr, len);
        if (bytes_sent == SOCKET_ERROR) {
            perror("Error in sending acknowledgment");
            break;
        }

        if (expected_seq_num > 4) {
            break;
        }
    }

    printf("File transfer complete\n");
    fclose(fp);
    closesocket(sockfd);
    WSACleanup();
    return 0;
}
