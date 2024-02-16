#include <stdio.h>
#include <string.h>
#include <winsock2.h>

#define PORT 8080

// Function to perform binary division
void binaryDivision(char *dataWord, char *generator, char *result) {
    int dataLen = strlen(dataWord);
    int genLen = strlen(generator);

    // Perform binary division (XOR operation)
    for (int i = 0; i < dataLen - genLen + 1; i++) {
        if (dataWord[i] == '1') {
            for (int j = 0; j < genLen; j++) {
                dataWord[i + j] = (dataWord[i + j] == generator[j]) ? '0' : '1';
            }
        }
    }

    // Copy the result to the output
    strcpy(result, dataWord);
}

// Function to simulate an error in the 5th bit position
void simulateError(char *dataWord) {
    // Flip the 5th bit (index 4) to simulate an error
    if (dataWord[4] == '0') {
        dataWord[4] = '1';
    } else {
        dataWord[4] = '0';
    }
}

int main() {
    WSADATA wsa;
    SOCKET clientSocket;
    struct sockaddr_in serverAddr;
    char receivedCodeword[20];
    char dataWord[] = "1010011101";
    char last4Bits[5];
    char extendedDataWord[20];
    char result[20];
    char userInput;

    // Initialize Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        printf("Failed to initialize Winsock\n");
        return 1;
    }

    // Create socket
    clientSocket = socket(AF_INET, SOCK_STREAM, 0);

    // Configure settings of the server address struct
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1"); // Use the server's IP address
    serverAddr.sin_port = htons(PORT);

    // Connect to the server
    connect(clientSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr));

    // Ask the user if they want to simulate an error
    printf("Do you want to simulate an error? (y/n): ");
    scanf(" %c", &userInput);

    // Receive codeword from server
    recv(clientSocket, receivedCodeword, sizeof(receivedCodeword), 0);

    if (userInput == 'y' || userInput == 'Y') {
        // Simulate an error in the 5th bit position
        simulateError(dataWord);

        // Print the modified data word with an error
        printf("Data Word with Error: %s\n", dataWord);
    } else {
        // Print the original data word
        printf("Original Data Word: %s\n", dataWord);
    }

    // Append the last 4 bits to the modified data word
    strcpy(extendedDataWord, dataWord);
    strcat(extendedDataWord, last4Bits);

    // Perform binary division with the new generator (10111)
    binaryDivision(extendedDataWord, "10111", result);

    // Print the resulting codeword after introducing an error
    printf("Codeword after Introducing Error: %s\n", result);

    // Check for errors
    if (strcmp(result, "00000000000000") == 0) {
        printf("Client: No error detected in received codeword.\n");
    } else {
        printf("Client: Error detected in received codeword.\n");
    }

    // Close the socket
    closesocket(clientSocket);

    // Cleanup Winsock
    WSACleanup();

    return 0;
}
