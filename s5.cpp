#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>
#pragma comment(lib, "ws2_32.lib") // Winsock Library

#define PACKET_SIZE 1024
#define WINDOW_SIZE 4
#define ACK_MSG "ACK"

int main() {
    WSADATA wsa;
    SOCKET s, new_socket;
    struct sockaddr_in server, client;
    int c;
    FILE *file;
    char file_name[] = "sender1.txt"; // Name of the file to send
    char packet[PACKET_SIZE];
    int read_size, sent_size, packet_number = 0, ack_number = 0;
    int base = 0, next_seqnum = 0, last_seqnum = 0;

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

    while (1) {
        // Send packets within the window
        while (next_seqnum < base + WINDOW_SIZE && (read_size = fread(packet, 1, PACKET_SIZE, file)) > 0) {
            packet_number++;
            sent_size = send(new_socket, packet, read_size, 0);
            if (sent_size == SOCKET_ERROR) {
                printf("Send failed with error code: %d\n", WSAGetLastError());
                break;
            }
            printf("Packet %d sent.\n", packet_number);
            next_seqnum++;
        }

        // Wait for ACKs
        fd_set readfds;
        struct timeval timeout;
        FD_ZERO(&readfds);
        FD_SET(new_socket, &readfds);
        timeout.tv_sec = 5; // Timeout in seconds
        timeout.tv_usec = 0;

        if (select(0, &readfds, NULL, NULL, &timeout) == SOCKET_ERROR) {
            printf("Select failed with error code: %d\n", WSAGetLastError());
            break;
        }

        if (FD_ISSET(new_socket, &readfds)) {
            if (recv(new_socket, packet, PACKET_SIZE, 0) == SOCKET_ERROR) {
                printf("Recv failed with error code: %d\n", WSAGetLastError());
                break;
            }
            ack_number++;
            printf("ACK %d received.\n", ack_number);
            base++;
            if (base > last_seqnum) {
                break; // All packets acknowledged
            }
        } else {
            // Timeout occurred, retransmit the entire window
            printf("Timeout occurred. Retransmitting entire window.\n");
            fseek(file, (base - packet_number) * PACKET_SIZE, SEEK_CUR);
            next_seqnum = base;
        }
    }

    fclose(file);
    closesocket(new_socket);
    closesocket(s);
    WSACleanup();
    return 0;
}
