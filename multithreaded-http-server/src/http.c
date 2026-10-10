
/* #include directives */
#include "http.h"
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>


/* Function Definitions */

// send_all() -> Send all requested bytes, handling partial sends.
static int send_all(int fd, const char *buf, size_t len){
    size_t byte_handled = 0;

    while(byte_handled < len){
        ssize_t byte_sent = send(fd, buf + byte_handled, len - byte_handled, MSG_NOSIGNAL);
        if(byte_sent <= 0){
            return -1;
        }

        byte_handled += byte_sent;
    }

    return 0;
}

// send_http_response() -> Generate and send an HTTP 200 OK response.
int send_http_response(int conn_fd){
    // 1. Prepare the HTML response body.
    const char *body = "<h1>Hello HTTP Server!</h1>";
    size_t body_len = strlen(body);

    // 2. Generate the HTTP response header.
    char header[512];
    int header_len = snprintf(header, sizeof(header),
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/html; charset=utf-8\r\n"
        "Content-Length: %zu\r\n"
        "Connection: close\r\n"
        "\r\n",
        body_len
    );

    // Check for formatting errors or header buffer overflow.
    if(header_len < 0 || (size_t)header_len >= sizeof(header)){
        fprintf(stderr, "Header Creation Error\n");
        return -1;
    }

    // 3. Send the HTTP response header.
    if(send_all(conn_fd, header, (size_t)header_len) == -1){
        return -1;
    }

    // 4. Send the HTML response body.
    if(send_all(conn_fd, body, body_len) == -1){
        return -1;
    }

    return 0;
}
