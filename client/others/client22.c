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

    // Open a file to store the results
    char result_filename[256] = "result.txt"; // You can customize the filename as needed
    FILE *result_file = fopen(result_filename, "w");
    if (result_file == NULL) {
        perror("Error opening result file");
        exit(EXIT_FAILURE);
    }

    // Receive and write the results to the file
    char buffer[512];
    ssize_t bytes_received;

    while ((bytes_received = recv(client_socket, buffer, sizeof(buffer), 0)) > 0) {
        fwrite(buffer, 1, bytes_received, result_file);
    }

    // Close the result file and the client socket
    fclose(result_file);
    close(client_socket);

    printf("Results stored in %s\n", result_filename);

    return 0;
}
