#include <errno.h>
#include <stdlib.h>
#include <stdarg.h>

#include "http_server/http_server.h"
#include "http_server.h"

static HTTPHandle *HTTPHandle_last(const HTTPHandle *handle);

typedef struct LinkedList
{
    struct LinkedList *next;
} LinkedList;

typedef void (Deleter)(void*);

static void LinkedList_delete(LinkedList *ll, Deleter *deleter);

int HTTPServer_create(
    HTTPServer *http,
    const char *host,
    uint16_t port,
    int use_ipv6,
    uint32_t max_clients_amount
)
{
    if (http == NULL || host == NULL || port == 0)
        return socket_error_invalid_args;

    int status = Server_create(&http->tcp_server, host, port, use_ipv6, max_clients_amount);
    if (status != socket_error_success)
        return status;

    return socket_error_success;
}

int HTTPServer_add_handle(HTTPServer *http, const char *path, size_t handlers_amount, ...)
{
    if (http == NULL || path == NULL || handlers_amount == 0)
    {
        errno = EINVAL;
        return -1;
    }

    HTTPHandle *last = NULL;
    if (http->handles == NULL) {
        http->handles = malloc(sizeof(HTTPHandle));
        last = http->handles;
    }
    else {
        last = HTTPHandle_last(http->handles);
        last->next = malloc(sizeof(HTTPHandle));
        last = last->next;
    }
    last->next = NULL;
    last->path = path;

    va_list args;
    va_start(args, handlers_amount);
    HandlerList **handlers = &last->handlers;
    for (; handlers_amount > 0; --handlers_amount) {
        *handlers = malloc(sizeof(HandlerList));
        (*handlers)->method = va_arg(args, HTTPMethod);
        (*handlers)->handler_function = va_arg(args, Handler);
        handlers = &(*handlers)->next;
    }
    *handlers = NULL;
    va_end(args);

    return 0;
}

int HTTPServer_process(HTTPServer *http)
{
    char buffer[HTTP_REQ_BUF_SIZE];
    HTTPRequest request;
    Server_listen(&http->tcp_server);
    http->listen = 1;
    while (http->listen) {
        if (http->tcp_server._clients_amount <= 0)
            continue;

        size_t received = recv(http->tcp_server._clients[0]->_socket_descriptor, buffer, HTTP_REQ_BUF_SIZE, 0);
        if (!received)
            continue;

        HTTPRequest_parse(&request, buffer);
        HTTPHandle *handle = http->handles;
        while (handle != NULL) {
            if (strncmp(request.url, handle->path, strlen(handle->path)) != 0)
                continue;
            
            HandlerList *handler = handle->handlers;
            while (handler != NULL) {
                if (request.method != handler->method)
                    continue;

                (handler->handler_function)(&request);
                break;
            }
            break;
        }

        HTTPRequest_clean(&request);
    }
    Server_stop(&http->tcp_server);
}

void HTTPServer_delete(HTTPServer *http)
{
    if (http == NULL)
    {
        errno = EINVAL;
        return ;
    }

    http->listen = 0;
    Server_close(&http->tcp_server);
    LinkedList_delete(http->handles, &HTTPHandle_delete);
}

int HTTPRequest_parse(HTTPRequest *request, const char *buffer)
{
    if (request == NULL || buffer == NULL) {
        errno = EINVAL;
        return -1;
    }

    // Method parsing
    size_t substr_len = strcspn(buffer, " ");
    char temp[4096];
    strncpy(temp, buffer, substr_len);
    request->method = http_method_unknown;
    buffer += substr_len + 1;

    for (size_t i = 0; i < sizeof (conversion) / sizeof (*conversion);  ++i) {
        if (strcmp(temp, conversion[i].str) == 0) {
            request->method = conversion[i].val;
            break;
        }
    }

    // URL parsing
    memset(temp, 0, substr_len);
    substr_len = strcspn(buffer, " ");
    request->url = malloc(substr_len);
    strncpy(request->url, buffer, substr_len);
    buffer += substr_len + 1;

    // HTTP versioin parsing
    substr_len = strcspn(buffer, "\r\n");
    request->http_version = malloc(substr_len);
    strncpy(request->http_version, buffer, substr_len);
    buffer += substr_len + 2;
    
    // Headers parsing
    HTTPHeader **new_header = &request->headers;
    while (buffer[0] != '\r' || buffer[1] != '\n') {
        *new_header = malloc(sizeof(HTTPHeader));
        
        substr_len = strcspn(buffer, ":");
        (*new_header)->key = malloc(substr_len);
        
        memset(temp, 0, substr_len);
        strncpy((*new_header)->key, buffer, substr_len);
        buffer += substr_len + 2;

        substr_len = strcspn(buffer, "\r\n");
        (*new_header)->value = malloc(substr_len);
        
        memset(temp, 0, substr_len);
        strncpy((*new_header)->value, buffer, substr_len);
        buffer += substr_len + 2;
        
        new_header = &(*new_header)->next_header;
    }
    *new_header = NULL;
    buffer += 2;

    memset(temp, 0, substr_len);
    substr_len = strlen(buffer);
    request->body = malloc(substr_len);
    strncpy(request->body, buffer, substr_len);
    
    return 0;
}

int HTTPRequest_clean(HTTPRequest *request)
{
    if (request == NULL) {
        errno = EINVAL;
        return -1;
    }

    free(request->url);
    free(request->http_version);
    LinkedList_delete(request->headers, &HTTPHeader_delete);
    free(request->body);

    request->method = http_method_unknown;
    request->url = NULL;
    request->http_version = NULL;
    request->headers = NULL;
    request->body = NULL;
    
    return 0;
}

int HTTPResponse_send(ClientInterface *client, const HTTPResponse *response)
{
    if (client == NULL || response == NULL)
    {
        errno = EINVAL;
        return -1;
    }

    

    return 0;
}

void HTTPHeader_delete(HTTPHeader *header_list)
{
    free(header_list->key);
    free(header_list->value);
    free(header_list);
}

void HTTPHandle_delete(HTTPHandle *handle)
{
    LinkedList_delete(handle->handlers, &HTTPHandlerList_delete);
    free(handle->path);
    free(handle);
}

void HTTPHandlerList_delete(HandlerList *handle_list)
{
    free(handle_list);
}

HTTPHandle *HTTPHandle_last(const HTTPHandle *handle)
{
    while (handle->next != NULL)
        handle = handle->next;
    
    return handle;
}

void LinkedList_delete(LinkedList *ll, Deleter *deleter)
{
    if (ll->next != NULL)
        LinkedList_delete(ll->next, deleter);

    (*deleter)(ll);
}
