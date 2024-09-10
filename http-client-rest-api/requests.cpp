#include <stdlib.h>     /* exit, atoi, malloc, free */
#include <stdio.h>
#include <unistd.h>     /* read, write, close */
#include <string.h>     /* memcpy, memset */
#include <sys/socket.h> /* socket, connect */
#include <netinet/in.h> /* struct sockaddr_in, struct sockaddr */
#include <netdb.h>      /* struct hostent, gethostbyname */
#include <arpa/inet.h>
#include "helpers.h"
#include "requests.h"

char *compute_get_request(char *host, char *url, char *query_params,
    char **cookies, int cookies_count, char *token) {

    char *message = (char *)calloc(BUFLEN, sizeof(char));
    char *line = (char *)calloc(LINELEN, sizeof(char));

    // Step 1: Write the method name, URL, request params (if any), and protocol type
    if (query_params != NULL) {
        sprintf(line, "GET %s?%s HTTP/1.1", url, query_params);
    } else {
        sprintf(line, "GET %s HTTP/1.1", url);
    }
    compute_message(message, line);

    // Step 2: Add the host
    sprintf(line, "Host: %s", host);
    compute_message(message, line);

    if (cookies != NULL && cookies_count > 0) {
        sprintf(line, "Cookie: ");
        for (int i = 0; i < cookies_count; i++) {
            strcat(line, cookies[i]);
            if (i < cookies_count - 1) {
                strcat(line, "; ");
            }
        }
        compute_message(message, line);
    }

    if (token != NULL) {
        sprintf(line, "Authorization: Bearer %s", token);
        compute_message(message, line);
    }

    sprintf(line, "Connection: keep-alive");
    compute_message(message, line);

    compute_message(message, "");

    // Free the line buffer
    free(line);
    // Return the complete HTTP GET request message
    return message;
}

char *compute_post_request(char *host, char *url, char *content_type, char **body_data,
    int body_data_fields_count, char **cookies, int cookies_count, char *token)
{
    char *message = (char *)calloc(BUFLEN, sizeof(char));
    char *line = (char *)calloc(LINELEN, sizeof(char));
    char *body_data_buffer = (char *)calloc(BUFLEN, sizeof(char));

    // Step 1: write the method name, URL and protocol type
    sprintf(line, "POST %s HTTP/1.1", url);
    compute_message(message, line);

    memset(line, 0, LINELEN);
    // Step 2: add the host
    strcat(line, "Host: ");
    strcat(line, host);
    compute_message(message, line);
    /* Step 3: add necessary headers (Content-Type and Content-Length are mandatory)
            in order to write Content-Length you must first compute the message size
    */
    memset(line, 0, LINELEN);
    strcat(line, "Content-Type: ");
    strcat(line, content_type);
    compute_message(message, line);

    for (int i = 0; i < body_data_fields_count; i++) {
        strcat(body_data_buffer, body_data[i]);
    }
    memset(line, 0, LINELEN);
    sprintf(line, "Content-Length: %ld", strlen(body_data_buffer));
    // strcat(line, "Content-Length: ");
    // strcat(line, strlen(body_data_buffer));
    compute_message(message, line);

    // Step 4 (optional): add cookies
    memset(line, 0, LINELEN);
    if (cookies != NULL) {
        if (cookies_count > 0) {
            strcat(line, "Cookies: ");
        }

        for (int i = 0; i < cookies_count; i++) {
            strcat(line, cookies[i]);
            if (i != cookies_count - 1) {
                strcat(line, "; ");
            }
        }
        compute_message(message, line);
    }

    if (token != NULL) {
        sprintf(line, "Authorization: Bearer %s", token);
        compute_message(message, line);
    }
    sprintf(line, "connection: keep-alive");
    compute_message(message, line);

    compute_message(message, "");

    // Step 6: Add the actual payload data
    strcat(message, body_data_buffer);

    // Free the allocated buffers
    free(line);
    free(body_data_buffer);

    return message;
}

char *delete_request(char *host, char *url, char *query_params,
    char **cookies, int cookies_count, char *token) {

    char *message = (char *)calloc(BUFLEN, sizeof(char));
    char *line = (char *)calloc(LINELEN, sizeof(char));

    // Step 1: Write the method name, URL, request params (if any), and protocol type
    if (query_params != NULL) {
        sprintf(line, "DELETE %s?%s HTTP/1.1", url, query_params);
    } else {
        sprintf(line, "DELETE %s HTTP/1.1", url);
    }
    compute_message(message, line);

    // Step 2: Add the host
    sprintf(line, "Host: %s", host);
    compute_message(message, line);

    if (cookies != NULL && cookies_count > 0) {
        sprintf(line, "Cookie: ");
        for (int i = 0; i < cookies_count; i++) {
            strcat(line, cookies[i]);
            if (i < cookies_count - 1) {
                strcat(line, "; ");
            }
        }
        compute_message(message, line);
    }

    if (token != NULL) {
        sprintf(line, "Authorization: Bearer %s", token);
        compute_message(message, line);
    }

    sprintf(line, "connection: keep-alive");
    compute_message(message, line);

    compute_message(message, "");

    // Free the line buffer
    free(line);
    // Return the complete HTTP GET request message
    return message;
}
