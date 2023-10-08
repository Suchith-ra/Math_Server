
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

void receiveAndStoreResults(int client_socket, int request_number) {
    char result_filename[256];
    snprintf(result_filename, sizeof(result_filename), "result_%d.txt", request_number);
    FILE *result_file = fopen(result_filename, "w");
    if (result_file == NULL) {
        perror("Error opening result file");
        exit(EXIT_FAILURE);
    }

    char result_buffer[512];
    ssize_t bytes_received;

    while ((bytes_received = recv(client_socket, result_buffer, sizeof(result_buffer), 0)) > 0) {
        fwrite(result_buffer, 1, bytes_received, result_file); // Write the result to the file
    }

    // Close the result file and the client socket
    fclose(result_file);

    printf("Results stored in %s\n", result_filename);
}

int main(int argc, char *argv[]) {
    // ... (previous code for argument parsing)

    int client_socket;
    struct sockaddr_in server_address;

    // Create socket
    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == -1) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = inet_addr(server_ip);
    server_address.sin_port = htons(server_port);

    // Connect to the server
    if (connect(client_socket, (struct sockaddr *)&server_address, sizeof(server_address)) == -1) {
        perror("Connection failed");
        exit(EXIT_FAILURE);
    }

    // Send calculator command to the server
    char calculator_command[256];
    printf("Enter calculator command: ");
    fgets(calculator_command, sizeof(calculator_command), stdin);

    // Send the command to the server
    send(client_socket, calculator_command, strlen(calculator_command), 0);

    // Receive and store the results in a new file
    static int request_number = 0;
    request_number++; // Increment the request number for each new request

    receiveAndStoreResults(client_socket, request_number);

    // Close the client socket
    close(client_socket);

    return 0;
    }
