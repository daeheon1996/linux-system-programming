/* #include directives */
#include "server.h"
#include "http.h"
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>


/* #define directives */
#define MAX_REQUEST_SIZE 4096


/* Function Definitions */

// handle_client() -> Process a single client's HTTP request in a worker thread.
static void *handle_client(void *arg){
    // Copy the client socket FD from the thread argument.
    int conn_fd = *(int *)arg;

    // Release the dynamically allocated thread argument.
    free(arg);

    // Receive the client's HTTP request headers.
    char request_buf[MAX_REQUEST_SIZE];
    ssize_t byte_received = receive_http_request(conn_fd, request_buf, MAX_REQUEST_SIZE);

    if(byte_received > 0){
        // Print the received HTTP request.
        printf("Received = %s\n", request_buf);

        // Parse the HTTP request line.
        char method[16];
        char path[256];
        char version[16];

        if(parse_http_request(request_buf, method, path, version) == -1){
            // Send HTTP 400 if request parsing fails.
            if(send_http_response(conn_fd, 400) == -1){
                fprintf(stderr, "HTTP 400 response failed\n");
            }

            close(conn_fd);
            return NULL;
        }

        // Print the parsed HTTP request information.
        printf("Method: %s\n", method);
        printf("Path: %s\n", path);
        printf("Version: %s\n", version);

        // Determine the HTTP status code and send a response.
        int status_code = get_http_status(method, path);

        if(send_http_response(conn_fd, status_code) == -1){
            fprintf(stderr, "HTTP response failed\n");
        }

    } else if(byte_received == 0){
        // The client closed its sending side (TCP EOF).
        printf("Connection Closed\n");

    } else {
        // Handle an HTTP request receive error.
        perror("recv");
    }

    // Close the client connection.
    close(conn_fd);

    // Terminate the worker thread.
    return NULL;
}


// run_server() -> Run a multithreaded HTTP server on port 8080.
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

    // 6. Accept clients and dispatch worker threads.
    while(true){
        // Accept a new client connection.
        int conn_fd = accept(listen_fd, NULL, NULL);
        if(conn_fd == -1){
            perror("accept");
            close(listen_fd);
            return 1;
        }

        printf("Client connected!\n");

        // Allocate independent memory for the client's socket FD.
        int *client_fd = malloc(sizeof(int));
        if(client_fd == NULL){
            perror("malloc");
            close(conn_fd);
            close(listen_fd);
            return 1;
        }

        // Copy the client FD into the allocated memory.
        *client_fd = conn_fd;

        // Create a worker thread to handle the client.
        pthread_t tid;
        int pthread_create_ret = pthread_create(&tid, NULL, handle_client, client_fd);

        if(pthread_create_ret != 0){
            fprintf(stderr, "pthread_create: %s\n", strerror(pthread_create_ret));

            // Release resources if thread creation fails.
            free(client_fd);
            close(conn_fd);
            close(listen_fd);
            return 1;
        }

        // Detach the worker to enable automatic thread resource cleanup.
        int detach_ret = pthread_detach(tid);
        if(detach_ret != 0){
            fprintf(stderr, "pthread_detach: %s\n", strerror(detach_ret));
        }

        // Continue accepting clients without waiting for the worker.
    }

    // 7. Release the listening socket on server shutdown.
    close(listen_fd);
    return 0;
}