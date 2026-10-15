#ifndef HTTP_H
#define HTTP_H

/* #include directives */
#include <stddef.h>
#include <sys/types.h>

/* function prototpyes */
int send_http_response(int conn_fd, int status_code);
int parse_http_request(const char *request_buf, char *method, char *path, char *version);
int get_http_status(const char *method, const char *path);
ssize_t receive_http_request(int conn_fd, char *request_buf, size_t buf_size);


#endif