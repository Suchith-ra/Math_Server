#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <fcntl.h>

void saveResultToFile(const char *result, pid_t pid) {
    char result_filename[512];
    snprintf(result_filename, sizeof(result_filename), "%sresult_%d.txt", server_folder, getpid());


    FILE *file = fopen(result_filename, "w");
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

        // Create a child process to handle the client
        pid_t child_pid = fork();
        if (child_pid == -1) {
            perror("Fork failed");
            close(client_socket);
            continue; // Continue listening for other connections
        }

        if (child_pid == 0) {
            // Child process code
            close(server_socket); // Close the server socket in the child process

            // Receive the calculator command from the client
            char command[256];
            ssize_t bytes_received = recv(client_socket, command, sizeof(command), 0);
            if (bytes_received == -1) {
                perror("Receive failed");
                close(client_socket);
                exit(EXIT_FAILURE);
            }

            // Null-terminate the received command
            command[bytes_received] = '\0';

            // Prepend "./" to the command
            char full_command[512];
            snprintf(full_command, sizeof(full_command), "./%s", command);

            // Execute the modified command
            FILE *fp = popen(full_command, "r");
            if (fp == NULL) {
                perror("Command execution failed");
                close(client_socket);
                exit(EXIT_FAILURE);
            }

            // Read the result from the calculator program
            char *result_buffer = NULL;
            size_t result_buffer_size = 0;

            while (1) {
                ssize_t bytes_read = getline(&result_buffer, &result_buffer_size, fp);
                if (bytes_read == -1) {
                    // End of file or error
                    break;
                }
                // Send the result to the client
                send(client_socket, result_buffer, bytes_read, 0);
                // Save the result to a file named with the child process PID
                saveResultToFile(result_buffer, getpid());
            }

            // Close the client socket and exit the child process
            close(client_socket);
            exit(EXIT_SUCCESS);
        } else {
            // Parent process code
            close(client_socket); // Close the client socket in the parent process
            // Continue listening for other connections
        }
    }

    // Close the server socket when done
    close(server_socket);

    return 0;
}

