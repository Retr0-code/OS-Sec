#pragma once

#include "server/server.h"
#include "server/client.h"
#include "network_exceptions.h"

#define HTTP_REQ_BUF_SIZE       4096
#define HTTP_METHOD_MAX_LEN     12
#define HTTP_URI_MAX_LEN        8096
#define HTTP_VERSION_MAX_LEN    12

typedef enum
{
    http_method_unknown,
    http_method_get,
    http_method_post,
    http_method_put,
    http_method_head
} HTTPMethod;

const static struct {
    HTTPMethod val;
    const char *str;
} conversion [] = {
    { http_method_unknown, "" },
    { http_method_get, "GET" },
    { http_method_post, "POST" },
    { http_method_put, "PUT" },
    { http_method_head, "HEAD" },
};

typedef struct HTTPHeader
{
    struct HTTPHeader   *next_header;
    char                *key; 
    char                *value;
} HTTPHeader;

typedef struct HTTPRequest
{
    HTTPMethod          method;
    char                *url;
    char                *http_version;
    struct HTTPHeader   *headers;
    char                *body;
} HTTPRequest;

typedef struct HTTPResponse
{
    int test;
} HTTPResponse;

typedef HTTPResponse *(*Handler)(HTTPRequest*);

typedef struct HandlerList
{
    struct HandlerList  *next;
    HTTPMethod          method;
    Handler             handler_function;
} HandlerList;
 
typedef struct HTTPHandle
{
    struct HTTPHandle   *next;
    const char          *path;
    HandlerList         *handlers;
} HTTPHandle;

typedef struct HTTPServer
{
    atomic_int  listen;
    Server      tcp_server;
    HTTPHandle  *handles;
} HTTPServer;

int HTTPServer_create(
    HTTPServer *http,
    const char *host,
    uint16_t port,
    int use_ipv6,
    uint32_t max_clients_amount
);

int HTTPServer_add_handle(HTTPServer *http, const char *path, size_t handlers_amount, ...);

int HTTPServer_process(HTTPServer *http);

void HTTPServer_delete(HTTPServer *http);

int HTTPRequest_parse(HTTPRequest *request, const char *buffer);

int HTTPRequest_clean(HTTPRequest *request);

int HTTPResponse_send(ClientInterface *client, const HTTPResponse *response);

void HTTPHeader_delete(HTTPHeader *header_list);

void HTTPHandle_delete(HTTPHandle *handle);

void HTTPHandlerList_delete(HandlerList *handle_list);

// int HTTPServer_parse_request(HTTPServer *http, const char *data);
