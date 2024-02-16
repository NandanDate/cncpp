#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>
#pragma comment(lib, "ws2_32.lib")

#define PORT 9999
#define SERVER_IP "127.0.0.1"
#define FILENAME "received_file4.txt"
#define BUFFER_SIZE 2000

int main() {
    WSADATA wsa;
    SOCKET clientSocket;
    struct sockaddr_in server;
    FILE *file;
    char buffer[BUFFER_SIZE];
    int bytes_received;
    int frame_number = 0; // Initialize frame number
    int simulate_error;   // Variable to hold user input for error simulation

    printf("Initializing Winsock...\n");
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        printf("Failed. Error Code: %d\n", WSAGetLastError());
        return 1;
    }
    printf("Initialized.\n");

    // Create socket
    if ((clientSocket = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
        printf("Could not create socket: %d\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }
    printf("Socket created.\n");

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = inet_addr(SERVER_IP);
    server.sin_port = htons(PORT);

    // Connect to server
    if (connect(clientSocket, (struct sockaddr *)&server, sizeof(server)) < 0) {
        printf("Connect failed.\n");
        closesocket(clientSocket);
        WSACleanup();
        return 1;
    }
    printf("Connected to server.\n");

    // Prompt user to simulate error
    printf("Do you want to simulate an error? (1 for Yes / 0 for No): ");
    scanf("%d", &simulate_error);

    // Open file for writing
    file = fopen(FILENAME, "wb");
    if (file == NULL) {
        printf("Error opening file for writing.\n");
        closesocket(clientSocket);
        WSACleanup();
        return 1;
    }

    while (1) {
        printf("Do you want to simulate an error? (1 for Yes / 0 for No): ");
    scanf("%d", &simulate_error);
        bytes_received = recv(clientSocket, buffer, BUFFER_SIZE, 0);
        if (bytes_received <= 0) {
            break;
        }

        if (simulate_error == 1) {
            printf("Error simulated. Did not send acknowledgment for frame.\n");
            char ack = 211
            ;
            (send(clientSocket, (char *)&ack, sizeof(ack), 0) == SOCKET_ERROR);
            // Write received data to file
            fwrite(buffer, 1, bytes_received, file);
            simulate_error=0;
            // Send acknowledgment
          
        } else {
            // Write received data to file
            fwrite(buffer, 1, bytes_received, file);

            // Send acknowledgment
            int ack = frame_number; // Assuming acknowledgment value is the frame number
            if (send(clientSocket, (char *)&ack, sizeof(ack), 0) == SOCKET_ERROR) {
                printf("Send failed.\n");
                fclose(file);
                closesocket(clientSocket);
                WSACleanup();
                return 1;
            }
            printf("Received packet: %d\n", frame_number);
            printf("Sent ACK: %d\n", ack);
        }

        frame_number++;
    }

    // Close file and socket
    fclose(file);
    closesocket(clientSocket);
    WSACleanup();
    printf("File received successfully.\n");
    return 0;
}
