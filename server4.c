#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#pragma comment(lib, "ws2_32.lib")

#define PORT 9999
#define WINDOW_SIZE 10 // Adjust window size as needed
#define FRAME_SIZE 1 // Adjust frame size as needed
#define FILENAME "file.txt"
#define MAX_RETRY 3   // Maximum retransmission attempts for lost frames

int main() {
    WSADATA wsa;
    SOCKET serverSocket, clientSocket;
    struct sockaddr_in server, client;
    int c;
    FILE *file;
    char file_buffer[2000];
    int read_size;
    int retry_count;  // Variable to track the number of retries

    printf("Initializing Winsock...\n");
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        printf("Failed. Error Code: %d\n", WSAGetLastError());
        return 1;
    }
    printf("Initialized.\n");

    // Create socket
    if ((serverSocket = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
        printf("Could not create socket: %d\n", WSAGetLastError());
        return 1;
    }
    printf("Socket created.\n");

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(PORT);

    // Bind
    if (bind(serverSocket, (struct sockaddr *)&server, sizeof(server)) == SOCKET_ERROR) {
        printf("Bind failed with error code: %d\n", WSAGetLastError());
        return 1;
    }
    printf("Bind done.\n");

    // Listen
    listen(serverSocket, SOMAXCONN);
    printf("Listening for incoming connections...\n");

    // Accept incoming connections and send file contents to clients
    while (1) {
        c = sizeof(struct sockaddr_in);
        clientSocket = accept(serverSocket, (struct sockaddr *)&client, &c);
        if (clientSocket == INVALID_SOCKET) {
            printf("Accept failed with error code: %d\n", WSAGetLastError());
            return 1;
        }
        printf("Connection accepted.\n");

        file = fopen(FILENAME, "rb");
        if (file == NULL) {
            printf("Error opening file.\n");
            closesocket(clientSocket);
            continue;
        }

        int frame_number = 0; // Initialize frame number

        while ((read_size = fread(file_buffer, 1, sizeof(file_buffer), file)) > 0) {
    send(clientSocket, file_buffer, read_size, 0); // Send file content
    printf("Sent packet: %d\n", frame_number);

    // Receive acknowledgment
    int ack;
    retry_count = 0;
    int original_frame_number = frame_number; // Store the original frame number

    while (1) {
    if (ack == 211) {
        printf("Frame lost. Retransmitting frames: %d to %d\n", frame_number, frame_number + WINDOW_SIZE - 1);
        fseek(file, frame_number * sizeof(file_buffer), SEEK_SET);
        retry_count = 0;

        // Retransmit frames up to the window size
        int frames_to_send = frame_number + WINDOW_SIZE;
        for (int i = frame_number; i < frames_to_send; i++) {
            if (i < 50) {
                read_size = fread(file_buffer, 1, sizeof(file_buffer), file);
                send(clientSocket, file_buffer, read_size, 0);
                printf("Retransmitted packet: %d\n", i);
            }
        break;
    }
    }
    if (recv(clientSocket, (char *)&ack, sizeof(ack), 0) > 0) {
        printf("Received ACK: %d\n", ack);
        break;
    }

        // Resend frame if acknowledgment not received after maximum retries
        
        retry_count++;
    }

    frame_number++;
}


        // Sending End of File (EOF) frame
        int eof_frame = EOF;
        send(clientSocket, (char *)&eof_frame, sizeof(eof_frame), 0);
        printf("Sent EOF frame.\n");

        fclose(file);
        closesocket(clientSocket);
    }

    closesocket(serverSocket);
    WSACleanup();
    return 0;
}
