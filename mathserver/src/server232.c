#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <fcntl.h>

void saveResultToFile(const char *result, int request_number) {
    char filename[256];
    snprintf(filename, sizeof(filename), "result_%d.txt", request_number);
    FILE *file = fopen(filename, "w");
    if (file == NULL) {
        perror("File creation failed");
        return;
    }
    fprintf(file, "%s", result);
    fclose(file);
}

int main() {
    int server_socket, client_socket;
    struct sockaddr_in server_address, client_address;
    socklen_t client_address_len = sizeof(client_address);

    int request_number = 0; // Counter for tracking client requests

    // Create socket
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(9999); // Use your desired port

    // Bind the socket to the server address
    if (bind(server_socket, (struct sockaddr *)&server_address, sizeof(server_address)) == -1) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    // Listen for incoming connections
    if (listen(server_socket, 5) == -1) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    printf("Calculator Server listening on port 9999...\n");

    while (1) {
        // Accept incoming connections from clients
        client_socket = accept(server_socket, (struct sockaddr *)&client_address, &client_address_len);
        if (client_socket == -1) {
            perror("Accept failed");
            continue; // Continue listening for other connections
        }

        // Increment the request number for each new client request
        request_number++;

        // Receive the calculator command from the client
        char command[256];
        ssize_t bytes_received = recv(client_socket, command, sizeof(command), 0);
        if (bytes_received == -1) {
            perror("Receive failed");
            close(client_socket);
            continue; // Continue listening for other connections
        }

        // Null-terminate the received command
        command[bytes_received] = '\0';

        // Prepend "./" to the command
        char full_command[512];
        snprintf(full_command, sizeof(full_command), "./%s", command);

        // Execute the modified command and generate a result
        FILE *fp = popen(full_command, "r");
        if (fp == NULL) {
            perror("Command execution failed");
            close(client_socket);
            continue; // Continue listening for other connections
        }

        char buffer[512];
        size_t bytes_read;

        while ((bytes_read = fread(buffer, 1, sizeof(buffer), fp)) > 0) {
            send(client_socket, buffer, bytes_read, 0);
        }

        // Close the popen process
        pclose(fp);

        // Close the client socket
        close(client_socket);

        // Save the result to a unique filename
        saveResultToFile(buffer, request_number);
    }

    // Close the server socket when done
    close(server_socket);

    return 0;
    }
