#include <stdio.h>
#include <string.h>
#include <winsock2.h>

#define PORT 8080

// Function to perform binary division
void binaryDivision(char *dataWord, char *generator, char *codeword) {
    int dataLen = strlen(dataWord);
    int genLen = strlen(generator);

    // Append zeros to the data word
    strcat(dataWord, "0000");

    // Perform binary division (XOR operation)
    for (int i = 0; i < dataLen; i++) {
        if (dataWord[i] == '1') {
            for (int j = 0; j < genLen; j++) {
                dataWord[i + j] = (dataWord[i + j] == generator[j]) ? '0' : '1';
            }
        }
    }

    // Copy the result to the codeword
    strcpy(codeword, dataWord);
}

int main() {
    WSADATA wsa;
    SOCKET serverSocket, newSocket;
    struct sockaddr_in serverAddr, clientAddr;
    int addr_size;

    // Initialize Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        printf("Failed to initialize Winsock\n");
        return 1;
    }

    // Create socket
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);

    // Configure settings of the server address struct
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(PORT);

    // Bind the socket to the address and port
    bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr));

    // Listen on the socket, allowing up to 5 connections to queue
    listen(serverSocket, 5);

    printf("Server: Waiting for client connection...\n");

    // Accept a connection from the client
    newSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &addr_size);

    printf("Server: Client connected\n");

    // Generate codeword
    char dataWord[] = "1010011101";
    char generator[] = "10111";
    char codeword[20];
    binaryDivision(dataWord, generator, codeword);

    // Send codeword to client
    send(newSocket, codeword, sizeof(codeword), 0);

    printf("Server: Codeword sent to client: %s\n", codeword);

    // Close the sockets
    closesocket(newSocket);
    closesocket(serverSocket);

    // Cleanup Winsock
    WSACleanup();

    return 0;
}
