
/* #include directives */
#include "server.h"
#include "http.h"
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>


/* Function Definitions */

// run_server() -> Run a sequential HTTP server on port 8080.
int run_server(void){
    // 1. Create a TCP listening socket.
    int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(listen_fd == -1){
        perror("socket");
        return 1;
    }

    // 2. Configure the server address and port.
    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(8080);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    // 3. Allow local address reuse when restarting the server.
    int opt = 1;
    if(setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1){
        perror("setsockopt");
        close(listen_fd);
        return 1;
    }

    // 4. Bind the socket to the server address.
    if(bind(listen_fd, (struct sockaddr *)&addr, sizeof(addr)) == -1){
        perror("bind");
        close(listen_fd);
        return 1;
    }

    // 5. Mark the socket as listening for connections.
    if(listen(listen_fd, 5) == -1){
        perror("listen");
        close(listen_fd);
        return 1;
    }

    printf("Server listening on port 8080...\n");

    // 6. Handle client connections sequentially.
    while(true){
        // Accept a new client connection.
        int conn_fd = accept(listen_fd, NULL, NULL);
        if(conn_fd == -1){
            perror("accept");
            close(listen_fd);
            return 1;
        }

        printf("Client connected!\n");

        // Receive the client's HTTP request (single recv for now).
        char buf[4096];
        ssize_t byte_received = recv(conn_fd, buf, sizeof(buf) - 1, 0);

        if(byte_received > 0){
            // Null-terminate the received data for printing.
            buf[byte_received] = '\0';
            printf("Received = %s\n", buf);

            // Send an HTTP response to the client.
            if(send_http_response(conn_fd) == -1){
                fprintf(stderr, "HTTP response failed\n");
                close(conn_fd);
                continue;
            }

        } else if(byte_received == 0){
            // The client has closed its sending side (TCP EOF).
            printf("Connection Closed\n");
            close(conn_fd);
            continue;

        } else {
            // Handle a receive error.
            perror("recv");
            close(conn_fd);
            continue;
        }

        // Close the current connection before accepting another.
        close(conn_fd);
    }

    // 7. Release the listening socket on server shutdown.
    close(listen_fd);
    return 0;
}
