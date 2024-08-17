#include <stdio.h>

#define NETSOCK_IMPLEMENTATION
#include "netsock.h"

#define SERVE_IMPLEMENTATION
#include "serve.h"

int main(void) {
    Net sock = {0};
    if (!netsocket_bind(&sock, ipv4_from_cstr("0.0.0.0"), 8080)) return 1;
    if (!netsocket_listen(&sock, 1)) return 1;

    bool should_exit = false;
    while (1) {
        Net conn = {0};

        printf("Waiting for request...\n");
        if (!netsocket_accept(&sock, &conn)) continue;

        char buf[1024] = {0};
        int n = 0;
        if (!netsocket_recv(&conn, buf, 1024, &n, 0)) goto defer;

        buf[n] = 0;
        ServeRequest* req = NULL;
        if (!serve_parse_request(&req, buf)) {
            char* res = "HTTP/1.1 400 Bad Request\r\n\r\n";
            if (!netsocket_send(&conn, res, strlen(res), 0)) goto defer;
        }

        printf("Request: %s %s %s\n", req->method, req->path, req->protocol);
        ServeHeader* cur = req->headers;
        while (cur != NULL) {
            printf("    %s: %s\n", cur->name, cur->value);
            cur = cur->next;
        }

        if (strcmp(req->path, "/ping") == 0) {
            char* res = "HTTP/1.1 200 Ok\r\n\r\nPong";
            if (!netsocket_send(&conn, res, strlen(res), 0)) goto defer;
        } else if (strcmp(req->path, "/kill") == 0) {
            char* res = "HTTP/1.1 200 Ok\r\n\r\n";
            if (!netsocket_send(&conn, res, strlen(res), 0)) goto defer;
            should_exit = true;
        } else {
            char* res = "HTTP/1.1 404 Not Found\r\n\r\n";
            if (!netsocket_send(&conn, res, strlen(res), 0)) goto defer;
        }

defer:
        serve_request_free(req);
        if (!netsocket_close(&conn)) return 1;

        if (should_exit) break;
    }

    if (!netsocket_close(&sock)) return 1;
    return 0;
}
