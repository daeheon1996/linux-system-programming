/* Include Directives */
#include "http.h"
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <errno.h>

/* Function Definitions */

// get_status_text() -> Return the HTTP reason phrase for a status code.
static const char *get_status_text(int status_code){
    const char *status_text = NULL;

    if(status_code == 200){
        status_text = "OK";
    } else if(status_code == 400){
        status_text = "Bad Request";
    } else if(status_code == 404){
        status_text = "Not Found";
    } else if(status_code == 405){
        status_text = "Method Not Allowed";
    } else {
        status_text = "Internal Server Error";
    }

    return status_text;
}

// send_all() -> Send all bytes while handling partial sends and interruptions.
static int send_all(int fd, const char *buf, size_t len){
    size_t byte_handled = 0;

    while(byte_handled < len){
        ssize_t byte_sent = send(fd, buf + byte_handled, len - byte_handled, MSG_NOSIGNAL);

        if(byte_sent == -1){
            if(errno == EINTR){
                continue;
            }

            return -1;
        } else if(byte_sent == 0){
            return -1;
        }

        byte_handled += byte_sent;
    }

    return 0;
}

// send_http_response() -> Generate and send an HTTP response.
int send_http_response(int conn_fd, int status_code){
    // Prepare the HTML response body.
    const char *body = "<h1>Hello HTTP Server!</h1>";
    size_t body_len = strlen(body);
    const char *status_text = get_status_text(status_code);

    // Generate the HTTP response headers.
    char header[512];
    int header_len;

    if(status_code != 405){
        header_len = snprintf(header, sizeof(header),
            "HTTP/1.1 %d %s\r\n"
            "Content-Type: text/html; charset=utf-8\r\n"
            "Content-Length: %zu\r\n"
            "Connection: close\r\n"
            "\r\n",
            status_code,
            status_text,
            body_len
        );
    } else {
        header_len = snprintf(header, sizeof(header),
            "HTTP/1.1 %d %s\r\n"
            "Allow: GET\r\n"
            "Content-Type: text/html; charset=utf-8\r\n"
            "Content-Length: %zu\r\n"
            "Connection: close\r\n"
            "\r\n",
            status_code,
            status_text,
            body_len
        );
    }

    // Check for formatting errors and header buffer overflow.
    if(header_len < 0 || (size_t)header_len >= sizeof(header)){
        fprintf(stderr, "Header creation error\n");
        return -1;
    }

    // Send the HTTP response headers.
    if(send_all(conn_fd, header, (size_t)header_len) == -1){
        return -1;
    }

    // Send the HTTP response body.
    if(send_all(conn_fd, body, body_len) == -1){
        return -1;
    }

    return 0;
}

// parse_http_request() -> Extract the method, path, and version from an HTTP request.
int parse_http_request(const char *request_buf, char *method, char *path, char *version){
    if(sscanf(request_buf, "%15s %255s %15s", method, path, version) != 3){
        return -1;
    }

    return 0;
}

// get_http_status() -> Determine the HTTP status code from the method and path.
int get_http_status(const char *method, const char *path){
    int status_code;

    if(strcmp(method, "GET") == 0 && strcmp(path, "/") == 0){
        status_code = 200;
    } else if(strcmp(method, "GET") != 0 && strcmp(path, "/") == 0){
        status_code = 405;
    } else {
        status_code = 404;
    }

    return status_code;
}

// receive_http_request() -> Receive HTTP request headers until CRLF CRLF is found.
ssize_t receive_http_request(int conn_fd, char *request_buf, size_t buf_size){
    // Validate the receive buffer.
    if(request_buf == NULL || buf_size < 2){
        return -1;
    }

    size_t total_received = 0;

    // Receive data until the complete HTTP headers arrive.
    while(total_received < buf_size - 1){
        ssize_t byte_received = recv(conn_fd, request_buf + total_received, buf_size - 1 - total_received, 0);

        if(byte_received > 0){
            total_received += byte_received;
            request_buf[total_received] = '\0';

            // Check for the end of the HTTP headers.
            if(strstr(request_buf, "\r\n\r\n") != NULL){
                return total_received;
            }

        } else if(byte_received == 0){
            return 0;
        } else {
            if(errno == EINTR){
                continue;
            }

            return -1;
        }
    }

    // Return -2 if the request headers exceed the buffer limit.
    return -2;
}