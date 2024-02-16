#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>
#pragma comment(lib, "ws2_32.lib") // Winsock Library

#define PACKET_SIZE 1024
#define ACK_MSG "ACK"
#define TIMEOUT_SEC 4 // Timeout in seconds

int main() {
    WSADATA wsa;
    SOCKET s, new_socket;
    struct sockaddr_in server, client;
    int c;
    FILE *file;
    char file_name[] = "sender1.txt"; // Name of the file to send
    char packet[PACKET_SIZE];
    int read_size, sent_size, packet_number = 0, ack_number = 0;
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
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(9999);

    if (bind(s, (struct sockaddr *)&server, sizeof(server)) == SOCKET_ERROR) {
        printf("Bind failed with error code: %d\n", WSAGetLastError());
        return 1;
    }

    printf("Bind done.\n");
    listen(s, 3);
    printf("Waiting for incoming connections...\n");

    c = sizeof(struct sockaddr_in);
    new_socket = accept(s, (struct sockaddr *)&client, &c);
    if (new_socket == INVALID_SOCKET) {
        printf("Accept failed with error code: %d\n", WSAGetLastError());
        return 1;
    }

    printf("Connection accepted.\n");

    file = fopen(file_name, "rb");
    if (file == NULL) {
        printf("Error opening file.\n");
        send(new_socket, "ERROR", strlen("ERROR"), 0);
        closesocket(new_socket);
        WSACleanup();
        return 1;
    }

    while ((read_size = fread(packet, 1, PACKET_SIZE, file)) > 0) {
        packet_number++;

     

        sent_size = send(new_socket, packet, read_size, 0);
        if (sent_size == SOCKET_ERROR) {
            printf("Send failed with error code: %d\n", WSAGetLastError());
            break;
        }

        printf("Packet %d sent.\n", packet_number);

        // Wait for ACK with timeout
        fd_set readfds;
        struct timeval timeout;
        FD_ZERO(&readfds);
        FD_SET(new_socket, &readfds);
        timeout.tv_sec = TIMEOUT_SEC;
        timeout.tv_usec = 0;

        int select_result = select(0, &readfds, NULL, NULL, &timeout);
        if (select_result == SOCKET_ERROR) {
            printf("Select failed with error code: %d\n", WSAGetLastError());
            break;
        } else if (select_result == 0) {
            // Timeout occurred, retransmit the packet
            printf("Timeout occurred. Retransmitting packet %d...\n", packet_number);
            fseek(file, -read_size, SEEK_CUR); // Rewind the file pointer
            continue; // Skip sending this frame
        }

        // ACK received
        if (recv(new_socket, packet, PACKET_SIZE, 0) == SOCKET_ERROR) {
            printf("Recv failed with error code: %d\n", WSAGetLastError());
            break;
        }

        ack_number++;
        printf("ACK %d received.\n", ack_number);
    }

    fclose(file);
    closesocket(new_socket);
    closesocket(s);
    WSACleanup();
    return 0;
}
