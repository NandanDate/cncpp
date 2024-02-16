#include <stdio.h>
#include <winsock2.h>
#pragma comment(lib, "ws2_32.lib") // Winsock Library

#define PACKET_SIZE 1024
#define ACK_MSG "ACK"

int main() {
    WSADATA wsa;
    SOCKET s;
    struct sockaddr_in server;
    FILE *file;
    char packet[PACKET_SIZE];
    int recv_size, write_size, packet_number = 0, ack_number = 0;
    char user_input;

    printf("\nInitialising Winsock...");
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        printf("Failed. Error Code: %d\n", WSAGetLastError());
        return 1;
    }

    printf("Initialised.\n");

    if ((s = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
        printf("Could not create socket: %d\n", WSAGetLastError());
        return 1;
    }

    printf("Socket created.\n");
    server.sin_addr.s_addr = inet_addr("127.0.0.1");
    server.sin_family = AF_INET;
    server.sin_port = htons(9999);

    if (connect(s, (struct sockaddr *)&server, sizeof(server)) < 0) {
        printf("Connect error\n");
        return 1;
    }

    printf("Connected.\n");

    file = fopen("receiver1.txt", "wb");
    if (file == NULL) {
        printf("Error opening file.\n");
        closesocket(s);
        WSACleanup();
        return 1;
    }

    while ((recv_size = recv(s, packet, PACKET_SIZE, 0)) > 0) {
        packet_number++;
        
        printf("Packet %d received. Simulate error? (y/n): ", packet_number);
        scanf(" %c", &user_input);

        if (user_input == 'y' || user_input == 'Y') {
            printf("Simulating error. Not sending ACK.\n");
            continue; // Skip sending ACK for this packet
        }

        write_size = fwrite(packet, 1, recv_size, file);
        if (write_size < recv_size) {
            printf("Write failed.\n");
            break;
        }

        printf("Packet %d written to file.\n", packet_number);

        // Send ACK
        if (send(s, ACK_MSG, strlen(ACK_MSG), 0) == SOCKET_ERROR) {
            printf("Send failed with error code: %d\n", WSAGetLastError());
            break;
        }

        ack_number++;
        printf("ACK %d sent.\n", ack_number);
    }

    fclose(file);
    closesocket(s);
    WSACleanup();
    return 0;
}
