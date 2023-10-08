#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <getopt.h>

int main(int argc, char *argv[]) {
    int opt;
    char *server_ip = NULL;
    int server_port = 0;
 
    // Loop through command-line arguments
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-ip") == 0) {
            // Check if the next argument exists and is not another option
            if (i + 1 < argc && argv[i + 1][0] != '-') {
                server_ip = argv[i + 1];
                i++; // Move to the next argument
            } else {
                fprintf(stderr, "Missing argument for -ip option\n");
                exit(EXIT_FAILURE);
            }
        } else if (strcmp(argv[i], "-p") == 0) {
            // Check if the next argument exists and is not another option
            if (i + 1 < argc && argv[i + 1][0] != '-') {
                server_port = atoi(argv[i + 1]);
                i++; // Move to the next argument
            } else {
                fprintf(stderr, "Missing argument for -p option\n");
                exit(EXIT_FAILURE);
            }
        } else {
            fprintf(stderr, "Unknown option: %s\n", argv[i]);
            exit(EXIT_FAILURE);
        }
    }

    if (server_ip == NULL || server_port == 0) {
        fprintf(stderr, "Usage: %s -ip <Server_IP> -p <Server_Port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

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
    

    char result_filename[256];
    ssize_t filename_received = recv(client_socket, result_filename, sizeof(result_filename), 0);
    if (filename_received == -1) {
        perror("Receive failed");
        exit(EXIT_FAILURE);
    }
    result_filename[filename_received] = '\0';

    // Open a file in the client folder with the received filename
    char client_folder[] = "/home/ubuntu/Desktop/assign_21/client/"; // Use the client's current folder
    
    char full_result_path[512]; // Increase buffer size to accommodate the path
    snprintf(full_result_path, sizeof(full_result_path), "%sresult.txt", client_folder);

    // Open and read the file content
    FILE *result_file = fopen(full_result_path, "r");
    if (result_file == NULL) {
        perror("Error opening result file");
        exit(EXIT_FAILURE);
    }

    // Dynamically allocate a buffer for the file content
    char buffer[512];

    while (1) {
        ssize_t bytes_received = recv(client_socket, buffer, sizeof(buffer),0);
        if (bytes_received <= 0) {
            // End of file or error
            break;
        }
        fwrite(buffer, 1, bytes_received, result_file);
     }

    // Close the file, free the dynamically allocated buffer, and close the client socket
    fclose(result_file);

    // Close the client socket
    close(client_socket);
    
    // Print the received file name to the user 
    printf("Received result file: %s\n", result_filename);

    return 0;
}
