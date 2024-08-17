#ifndef SERVE_H_
#define SERVE_H_
#include <stddef.h>

typedef struct serve_header_s {
    struct serve_header_s* next;

    char* name;
    char* value;

    char data[];
}ServeHeader;

typedef struct {
    char* method;
    char* path;
    char* protocol;
    ServeHeader* headers;
    char* body;

    char data[];
}ServeRequest;

#define SERVE_CODE_TYPES \
    X(CONTINUE, 100, "Continue") \
    X(SWITCHING_PROTOCOLS, 101, "Switching Protocols") \
    X(PROCESSING, 102, "Processing") \
    X(EARLY_HINTS, 103, "Early Hints") \
    X(OK, 200, "Ok") \
    X(CREATED, 201, "Created") \
    X(ACCEPTED, 202, "Accepted") \
    X(NON_AUTHORITATIVE_INFORMATION, 203, "Non-Authoritative information") \
    X(NO_CONTENT, 204, "No Content") \
    X(RESET_CONTENT, 205, "Reset Content") \
    X(PARTIAL_CONTENT, 206, "Partial Content") \
    X(MULTI_STATUS, 207, "Multi-Status") \
    X(ALREADY_REPORTED, 208, "Already Reported") \
    X(IM_USED, 209, "IM Used") \
    X(MULTIPLE_CHOICES, 300, "Multiple Choices") \
    X(MOVED_PERMANETLY, 301, "Moved Perminantly") \
    X(FOUND, 302, "Found") \
    X(SEE_OTHER, 303, "See Other") \
    X(NOT_MODIFIED, 304, "Not Modified") \
    X(TEMPORARY_REDIRECT, 307, "Temporary Redirect") \
    X(PERMANENT_REDIRECT, 308, "Permanent Redirect") \
    X(BAD_REQUEST, 400, "Bad Request") \
    X(UNAUTHORIZED, 401, "Unauthorized") \
    X(PAYMENT_REQUIRED, 402, "Payment Required") \
    X(FORBIDDEN, 403, "Forbidden") \
    X(NOT_FOUND, 404, "Not Found") \
    X(METHOD_NOT_ALLOWED, 405, "Method Not Allowed") \
    X(NOT_ACCEPTABLE, 406, "Not Acceptable") \
    X(PROXY_AUTHENTICATION_REQUIRED, 407, "Proxy Authentication Required") \
    X(REQUEST_TIMEOUT, 408, "Request Timeout") \
    X(CONFLICT, 409, "Conflict") \
    X(GONE, 410, "Gone") \
    X(LENGTH_REQUIRED, 411, "Length Required") \
    X(PRECONDITION_FAILED, 412, "Precondition Failed") \
    X(PAYLOAD_TOO_LARGE, 413, "Payload Too Large") \
    X(URI_TOO_LONG, 414, "URI Too Long") \
    X(UNSUPPORTED_MEDIA_TYPE, 415, "Unsupported Media Type") \
    X(RANGE_NOT_SATISFIABLE, 416, "Range Not Satisfiable") \
    X(EXPECTATION_FAILED, 417, "Expectation Failed") \
    X(IM_A_TEAPOT, 418, "I'm a teapot") \
    X(MISDIRECTED_REQUEST, 421, "Misdirected Request") \
    X(UNPROCESSABLE_CONTENT, 422, "Unprocessable Content") \
    X(LOCKED, 423, "Locked") \
    X(FAILED_DEPENDENCY, 424, "Failed Dependency") \
    X(TOO_EARLY, 425, "Too Early") \
    X(UPGRADE_REQUIRED, 426, "Upgrade Required") \
    X(PRECONDTITION_REQUIRED, 428, "Precondition Required") \
    X(TOO_MANY_REQUESTS, 429, "Too Many Requests") \
    X(REQUEST_HEADER_FIELDS_TOO_LARGE, 431, "Request Header Fields Too Large") \
    X(UNAVAILABLE_FOR_LEGAL_REASONS, 451, "Unavailable For Legal Reasons") \
    X(INTERNAL_SERVER_ERROR, 500, "Internal Server Error") \
    X(NOT_IMPLEMENTED, 501, "Not Implemented") \
    X(BAD_GATEWAY, 502, "Bad Gateway") \
    X(SERVICE_UNAVAILABLE, 503, "Service Unavailable") \
    X(GATEWAY_TIMEOUT, 504, "Gateway Timeout") \
    X(HTTP_VERSION_NOT_SUPPORTED, 505, "HTTP Version Not Supported") \
    X(VARIANT_ALSO_NEGOTIATES, 506, "Variant Also Negotiates") \
    X(LOOP_DETECTED, 508, "Loop Detected") \
    X(NOT_EXTENDED, 510, "Not Extended") \
    X(NETWORK_AUTHENTICATION_REQUIRED, 511, "Network Authentication Required")

#define X(name, code, ...) SERVE_##name = code,
typedef enum {
    SERVE_CODE_TYPES
}ServeResCode;
#undef X

typedef struct {
    char* protocol;
    ServeResCode code;
    ServeHeader* headers;
    char* body;
}ServeResponse;

char* serve_code_to_description(ServeResCode code);
char* serve_response_to_str(ServeResponse* res);

ServeHeader* serve_header_new(char* name, size_t name_len, char* value, size_t value_len);
void serve_header_append(ServeHeader* headers, char* name, size_t name_len, char* value, size_t value_len);
void serve_header_free(ServeHeader* headers);

bool serve_parse_request(ServeRequest** out, char* request);
void serve_request_free(ServeRequest* req);

#ifndef SERVE_MALLOC
#include <stdlib.h>
#define SERVE_MALLOC malloc
#endif // SERVE_MALLOC

#ifndef SERVE_FREE
#include <stdlib.h>
#define SERVE_FREE free
#endif // SERVE_FREE

#endif // SERVE_H_

#ifdef SERVE_IMPLEMENTATION
char* serve_code_to_description(ServeResCode code) {
#define X(name, _1, desc) case SERVE_##name: return desc;
    switch (code) {
        SERVE_CODE_TYPES
    }
#undef X

    return NULL;
}

char* serve_response_to_str(ServeResponse* res) {
    size_t str_len = strlen(res->protocol) + 1;
    str_len += 3 + 1;
    str_len += strlen(serve_code_to_description(res->code)) + 2;
    
    ServeHeader* cur = res->headers;
    while (cur != NULL) {
        str_len += strlen(cur->name);
        str_len += strlen(": ");
        str_len += strlen(cur->value);
        str_len += 2;
    }
    str_len += 2 + strlen(res->body);

    char* str = SERVE_MALLOC(str_len + 1);
    sprintf(str, "%s %d %s\r\n", res->protocol, res->code, serve_code_to_description(res->code));
    cur = res->headers;
    while (cur != NULL) {
        sprintf(str + strlen(str), "%s: %s\r\n", cur->name, cur->value);
        cur = cur->next;
    }
    sprintf(str + strlen(str), "\r\n%s", res->body);

    str[str_len] = 0;
    return str;
}


ServeHeader* serve_header_new(char* name, size_t name_len, char* value, size_t value_len) {
    ServeHeader* header = SERVE_MALLOC(sizeof(header->next) 
        + sizeof(header->name) 
        + sizeof(header->value) 
        + (name_len + 1 + value_len + 1) * sizeof(char));

    header->next = NULL;
    header->name = header->data;
    header->value = header->data + name_len + 1;

    memcpy(header->data, name, name_len);
    header->data[name_len] = 0;
    memcpy(header->data + name_len + 1, value, value_len);
    header->data[name_len + 1 + value_len] = 0;

    return header;
}

void serve_header_append(ServeHeader* headers, char* name, size_t name_len, char* value, size_t value_len) {
    if (headers->next == NULL) {
        headers->next = serve_header_new(name, name_len, value, value_len);
    } else {
        serve_header_append(headers->next, name, name_len, value, value_len);
    }
}

void serve_header_free(ServeHeader* headers) {
    if (headers->next != NULL) serve_header_free(headers->next);
    SERVE_FREE(headers);
}

bool serve_parse_request(ServeRequest** out, char* request) {
    char* reqcopy = request;

    char* first_space = strchr(reqcopy, ' ');
    if (first_space == NULL) return false;

    char* method = reqcopy;
    size_t method_len = first_space - method;
    reqcopy = first_space + 1;

    char* second_space = strchr(reqcopy, ' ');
    if (second_space == NULL) return false;

    char* path = reqcopy;
    size_t path_len = second_space - path;
    reqcopy = second_space + 1;

    char* crlf = strchr(reqcopy, '\r');
    if (crlf == NULL) return false;

    char* protocol = reqcopy;
    size_t protocol_len = crlf - protocol;
    reqcopy = crlf;

    reqcopy += 2;

    struct serve_header_s* headers = NULL;

    while (1) {
        if (strncmp(reqcopy, "\r\n", 2) == 0) break;

        size_t name_len = 0;
        char* name = NULL;
        size_t value_len = 0;
        char* value = NULL;

        char* colon = strchr(reqcopy, ':');
        if (colon == NULL) return false;

        name_len = colon - reqcopy;
        name = reqcopy;
        reqcopy = colon + 2;

        char* crlf = strchr(reqcopy, '\r');
        if (crlf == NULL) return false;

        value_len = crlf - reqcopy;
        value = reqcopy;
        reqcopy = crlf;

        if (headers == NULL) {
            serve_header_new(name, name_len, value, value_len);
        } else {
            serve_header_append(headers, name, name_len, value, value_len);
        }

        reqcopy += 2;
    }

    reqcopy += 2;

    char* body = reqcopy;
    size_t body_len = strlen(reqcopy);

    *out = SERVE_MALLOC(sizeof(uintptr_t) * 5 + (method_len + 1 + path_len + 1 + protocol_len + 1 + body_len + 1) * sizeof(char));
    (*out)->headers = headers;
    memcpy((*out)->data, method, method_len);
    (*out)->data[method_len] = 0;
    (*out)->method = (*out)->data;
    memcpy((*out)->data + method_len + 1, path, path_len);
    (*out)->data[method_len + 1 + path_len] = 0;
    (*out)->path = (*out)->data + method_len + 1;
    memcpy((*out)->data + method_len + 1 + path_len + 1, protocol, protocol_len);
    (*out)->data[method_len + 1 + path_len + 1 + protocol_len] = 0;
    (*out)->protocol = (*out)->data + method_len + 1 + path_len + 1;
    memcpy((*out)->data + method_len + 1 + path_len + 1 + protocol_len + 1, body, body_len);
    (*out)->data[method_len + 1 + path_len + 1 + protocol_len + 1 + body_len] = 0;
    (*out)->body = (*out)->data + method_len + 1 + path_len + 1 + protocol_len + 1;

    return true;
}

void serve_request_free(ServeRequest* req) {
    serve_header_free(req->headers);
    SERVE_FREE(req);
}

#endif // SERVE_IMPLEMENTATION
