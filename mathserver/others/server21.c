#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <fcntl.h>

void saveResultToFile(const char *result) {
    FILE *file = fopen("result.txt", "w");
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

        // Execute the modified command
        FILE *fp = popen(full_command, "r");
        if (fp == NULL) {
            perror("Command execution failed");
            close(client_socket);
            continue; // Continue listening for other connections
        }
	
	// Create a file to store the result
	char server_folder[] = "/home/ubuntu/Desktop/assign_21/mathserver/";
        char result_filename[512];
        snprintf(result_filename, sizeof(result_filename), "%sresult.txt", server_folder);
        FILE *result_file = fopen(result_filename, "w");
        if (result_file == NULL) {
            perror("Error opening result file");
            exit(EXIT_FAILURE);
        }
        // Read the result from the calculator program and write it to the file
        char *result_buffer = NULL;
        size_t result_buffer_size = 0;

        while (1) {
            ssize_t bytes_read = getline(&result_buffer, &result_buffer_size, fp);
            if (bytes_read == -1) {
            // End of file or error
               break;
            }
            fputs(result_buffer, result_file);
        }

        // Close the result file
        fclose(result_file);
        
        result_file = fopen(result_filename, "r");
        if (result_file == NULL) {
            perror("Error opening result file for reading");
            exit(EXIT_FAILURE);
        }

        char buffer[512]; // Use a buffer for reading and sending

        while (1) {
          size_t bytes_read = fread(buffer, 1, sizeof(buffer), result_file);
          if (bytes_read == 0) {
              break; // End of file
          }
          send(client_socket, buffer, bytes_read, 0);
        }
        
        // Close the client socket
        close(client_socket);
    }

    // Close the server socket when done
    close(server_socket);

    return 0;
}
