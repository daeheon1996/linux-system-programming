/* #include directives */
#include "http.h"
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>


/* Function Definitions */
// get_status_text() -> Return the HTTP reason phrase for the given status code.
static const char *get_status_text(int status_code){
    char *status_text = NULL;
    if(status_code == 200){
        status_text = "OK";
    } else if(status_code == 400){
        status_text = "Bad Request";
    } else if (status_code == 404){
        status_text = "Not Found";
    } else if (status_code == 405){
        status_text = "Method Not Allowed";
    } else {
        status_text = "Internal Server Error";
    }

    return status_text;
}

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

// send_http_response() -> Generate and send an HTTP response with the given status code.
int send_http_response(int conn_fd, int status_code){
    // 1. Prepare the HTML response body.
    const char *body = "<h1>Hello HTTP Server!</h1>";
    size_t body_len = strlen(body);
    const char *status_text = get_status_text(status_code);

    // 2. Generate the HTTP response header.
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

// parse_http_request() -> extract method, path, version from client's http request
int parse_http_request(const char *request_buf, char *method, char *path, char *version){
    if(sscanf(request_buf, "%15s %255s %15s", method, path, version) != 3){
        return -1;
    }

    return 0;
}

// get_http_status() -> get http status code by considering method and path
int get_http_status(const char *method, const char *path){
    int status_code;
    if(strcmp(method, "GET") == 0 && strcmp(path, "/") == 0){
        status_code = 200;
    } else if (strcmp(method, "GET") != 0 && strcmp(path, "/") == 0){
        status_code = 405;
    } else {
        status_code = 404;
    }

    return status_code;
}

// receive_http_request() -> 
ssize_t receive_http_request(int conn_fd, char *request_buf, size_t buf_size){
    // TODO: 1. how to validate buf size?
    if(request_buf == NULL || buf_size < 2){
        return -1;
    }

    size_t total_received = 0;

    while(total_received < buf_size - 1){
        ssize_t byte_received = recv(conn_fd, request_buf + total_received, buf_size - 1 - total_received, 0);
        if(byte_received > 0){
            total_received += byte_received;

            request_buf[total_received] = '\0';

            if(strstr(request_buf, "\r\n\r\n") != NULL){
            return total_received;
            }

        } else if (byte_received == 0){
            return 0;
        } else {
            // if (byte_received == -1)
            return -1;
        }
    }
    
    // 4. exceeds max buf size?
    return -1;
}
