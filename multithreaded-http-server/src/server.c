/* Include Directives */
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
#include <signal.h>
#include <errno.h>

/* Define Directives */
#define MAX_REQUEST_SIZE 4096

/* Global Variables */
static int active_clients = 0;
static pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;
static volatile sig_atomic_t stop_requested = 0;
static pthread_cond_t clients_cond = PTHREAD_COND_INITIALIZER;


/* Function Definitions */

// handle_signal() -> Request server shutdown when SIGINT or SIGTERM is received.
static void handle_signal(int signo){
    (void)signo;
    stop_requested = 1;
}

// handle_client() -> Process a single client's HTTP request in a worker thread.
static void *handle_client(void *arg){
    // Take ownership of the client FD and release the thread argument.
    int conn_fd = *(int *)arg;
    free(arg);

    // Receive the client's HTTP request headers.
    char request_buf[MAX_REQUEST_SIZE];
    ssize_t byte_received = receive_http_request(conn_fd, request_buf, MAX_REQUEST_SIZE);

    if(byte_received > 0){
        printf("Received = %s\n", request_buf);

        // Parse the HTTP request line.
        char method[16];
        char path[256];
        char version[16];

        if(parse_http_request(request_buf, method, path, version) == -1){
            if(send_http_response(conn_fd, 400) == -1){
                fprintf(stderr, "HTTP 400 response failed\n");
            }

            goto cleanup;
        }

        printf("Method: %s\n", method);
        printf("Path: %s\n", path);
        printf("Version: %s\n", version);

        // Determine the response status based on the request.
        int status_code = get_http_status(method, path);

        if(send_http_response(conn_fd, status_code) == -1){
            fprintf(stderr, "HTTP response failed\n");
        }

    } else if(byte_received == 0){
        printf("Connection Closed\n");

    } else {
        perror("recv");
    }

cleanup:
    // Release the client connection and update the active worker count.
    close(conn_fd);

    pthread_mutex_lock(&clients_mutex);
    active_clients--;
    printf("[INFO] Active Clients = %d\n", active_clients);

    // Notify the main thread that a worker has finished its job.
    pthread_cond_signal(&clients_cond);
    pthread_mutex_unlock(&clients_mutex);

    return NULL;
}

// run_server() -> Run a multithreaded HTTP server on port 8080.
int run_server(void){
    int exit_status = 0;

    // Register shutdown signal handlers without SA_RESTART.
    struct sigaction sa = {0};
    sa.sa_flags = 0;
    sa.sa_handler = handle_signal;

    if(sigemptyset(&sa.sa_mask) == -1){
        perror("sigemptyset");
        return 1;
    }

    if(sigaction(SIGINT, &sa, NULL) == -1){
        perror("sigaction");
        return 1;
    }

    if(sigaction(SIGTERM, &sa, NULL) == -1){
        perror("sigaction");
        return 1;
    }

    // Create and configure the listening socket.
    int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(listen_fd == -1){
        perror("socket");
        return 1;
    }

    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(8080);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    // Allow immediate reuse of the server address after restart.
    int opt = 1;
    if(setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1){
        perror("setsockopt");
        close(listen_fd);
        return 1;
    }

    if(bind(listen_fd, (struct sockaddr *)&addr, sizeof(addr)) == -1){
        perror("bind");
        close(listen_fd);
        return 1;
    }

    if(listen(listen_fd, 5) == -1){
        perror("listen");
        close(listen_fd);
        return 1;
    }

    printf("Server listening on port 8080...\n");

    // Accept connections and create one worker thread per client.
    while(!stop_requested){
        int conn_fd = accept(listen_fd, NULL, NULL);

        if(conn_fd == -1){
            // Retry if accept() was interrupted by a signal.
            if(errno == EINTR){
                continue;
            }

            perror("accept");
            exit_status = 1;
            break;
        }

        // Reject connections accepted after a shutdown request.
        if(stop_requested){
            close(conn_fd);
            break;
        }

        printf("[INFO] Client connected: (fd = %d)\n", conn_fd);

        // Prepare signals to block during worker thread creation.
        sigset_t sigset;
        sigemptyset(&sigset);
        sigaddset(&sigset, SIGINT);
        sigaddset(&sigset, SIGTERM);

        // Allocate a separate FD argument for the new worker.
        int *client_fd = malloc(sizeof(int));
        if(client_fd == NULL){
            perror("malloc");
            close(conn_fd);
            exit_status = 1;
            break;
        }

        *client_fd = conn_fd;

        // Count the worker before creation to prevent shutdown races.
        pthread_mutex_lock(&clients_mutex);
        active_clients++;
        printf("[INFO] Active clients = %d\n", active_clients);
        pthread_mutex_unlock(&clients_mutex);

        // Block shutdown signals so the new worker inherits the blocked mask.
        sigset_t oldset;
        int pthread_sigblock_ret = pthread_sigmask(SIG_BLOCK, &sigset, &oldset);

        if(pthread_sigblock_ret != 0){
            fprintf(stderr, "pthread_sigmask: %s\n", strerror(pthread_sigblock_ret));

            pthread_mutex_lock(&clients_mutex);
            active_clients--;
            pthread_mutex_unlock(&clients_mutex);

            free(client_fd);
            close(conn_fd);
            exit_status = 1;
            break;
        }

        // Create the worker and restore the main thread's signal mask.
        pthread_t tid;
        int pthread_create_ret = pthread_create(&tid, NULL, handle_client, client_fd);

        int pthread_sigrestore_ret = pthread_sigmask(SIG_SETMASK, &oldset, NULL);
        if(pthread_sigrestore_ret != 0){
            fprintf(stderr, "pthread_sigmask restore: %s\n", strerror(pthread_sigrestore_ret));
            exit_status = 1;
        }

        // Roll back the worker count and release resources if creation fails.
        if(pthread_create_ret != 0){
            fprintf(stderr, "pthread_create: %s\n", strerror(pthread_create_ret));

            pthread_mutex_lock(&clients_mutex);
            active_clients--;
            pthread_mutex_unlock(&clients_mutex);

            free(client_fd);
            close(conn_fd);
            exit_status = 1;
            break;
        }

        // Detach the worker for automatic thread resource reclamation.
        int detach_ret = pthread_detach(tid);

        if(detach_ret != 0){
            fprintf(stderr, "pthread_detach: %s\n", strerror(detach_ret));

            // Attempt to join the worker if detaching fails.
            int join_ret = pthread_join(tid, NULL);
            if(join_ret != 0){
                fprintf(stderr, "pthread_join: %s\n", strerror(join_ret));
            }

            exit_status = 1;
            break;
        }

        // Stop accepting clients if signal mask restoration failed.
        if(exit_status != 0){
            break;
        }
    }

    printf("[INFO] server shutdown started\n");

    // Stop accepting new connections.
    close(listen_fd);

    // Wait until all active workers have completed their cleanup.
    pthread_mutex_lock(&clients_mutex);

    while(active_clients > 0){
        pthread_cond_wait(&clients_cond, &clients_mutex);
    }

    pthread_mutex_unlock(&clients_mutex);

    printf("[INFO] server shutdown completed\n");

    return exit_status;
}
