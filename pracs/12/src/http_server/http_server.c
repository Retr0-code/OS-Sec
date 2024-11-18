#include <errno.h>
#include <stdlib.h>
#include <stdarg.h>
#include <signal.h>

#include "http_server/http_server.h"

#define MSG_NOT_FOUND   "Not Found"

typedef struct LinkedList
{
    struct LinkedList *next;
} LinkedList;

typedef void (Deleter)(void*);

static void LinkedList_delete(LinkedList *ll, Deleter *deleter);

static LinkedList *LinkedList_last(const LinkedList *ll);

static void HTTPHeader_create(HTTPHeader *header, const char *key, const char *value);

static HTTPServer *http_server_signal = NULL;

void HTTPServer_signal_stop(int signal);

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

    http_server_signal = http;
    struct sigaction *action_segv = malloc(sizeof(struct sigaction));
    memset(action_segv, 0, sizeof(struct sigaction));
    action_segv->sa_flags = SA_SIGINFO;
    action_segv->sa_sigaction = &HTTPServer_signal_stop;
    sigaction(SIGSEGV, action_segv, NULL);
    signal(SIGINT, &HTTPServer_signal_stop);

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
        last = LinkedList_last(http->handles);
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
    HTTPResponse response;
    ClientInterface client;
    http->listen = 1;
    int status = 404;
    while (http->listen) {
        if (Server_listen_connection(&http->tcp_server, &client) != socket_error_success)
            continue;

        size_t received = recv(client._socket_descriptor, buffer, HTTP_REQ_BUF_SIZE, 0);
        if (!received)
            continue;

        memset(&request, 0, sizeof(HTTPRequest));
        memset(&response, 0, sizeof(HTTPResponse));

        HTTPRequest_parse(&request, buffer);
        HTTPHandle *handle = http->handles;
        while (handle != NULL) {
            if (strncmp(request.url, handle->path, strlen(handle->path)) != 0) {
                handle = handle->next;
                continue;
            }
            
            HandlerList *handler = handle->handlers;
            while (handler != NULL) {
                if (request.method != handler->method) {
                    handler = handler->next;
                    continue;
                }

                status = (handler->handler_function)(&request, &response);
                break;
            }
            break;
        }

        if (status == 404)
            HTTPResponse_error(&response, status, MSG_NOT_FOUND);

        if (HTTPResponse_send(&client, &response))
            fprintf(stderr, "%s Broken response:\n%s\n", ERROR, strerror(errno));
        
        HTTPResponse_clean(&response);
        HTTPRequest_clean(&request);
        ClientInterface_close(&client);
        status = 404;
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

int HTTPRequest_parse_get_args(const HTTPRequest *request, int *argc, char ***argv)
{
    if (request == NULL || argc == NULL || argv == NULL) {
        errno = EINVAL;
        return -1;
    }

    *argc = 1;
    size_t substr_len = strcspn(request->url, "?");
    size_t req_len = strlen(request->url);
    if (req_len - substr_len <= 1) {
        *argv = malloc(sizeof(char *));
        **argv = malloc(substr_len);
        strncpy(**argv, request->url, substr_len);
        return 0;
    }
    *argc = 2;
    
    const char *param_str = request->url + substr_len;
    for (size_t i = 0; req_len > i && param_str[i] != 0; ++i)
        *argc += param_str[i] == '&';

    *argv = malloc(sizeof(char *) * (*argc));
    **argv = malloc(substr_len);
    strncpy(**argv, request->url, substr_len);
    for (int i = 1; *argc > i; ++i) {
        ++param_str;
        
        substr_len = strcspn(param_str, "&");

        (*argv)[i] = malloc(substr_len + 2);
        (*argv)[i][0] = '-';
        (*argv)[i][1] = '-';
        strncpy(&(*argv)[i][2], param_str, substr_len);
        (*argv)[i][substr_len + 2] = 0;

        param_str += substr_len;
    }

    return 0;
}

void HTTPRequest_delete_args(int argc, char **argv)
{
    for (int i = 0; argc > i; ++i)
        free(argv[i]);

    free(argv);
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

void HTTPHeader_create(HTTPHeader *header, const char *key, const char *value)
{
    size_t key_len = strlen(key) + 1;
    header->key = malloc(key_len);
    strncpy(header->key, key, key_len);

    size_t value_len = strlen(value) + 1;
    header->value = malloc(value_len);
    strncpy(header->value, value, value_len);
    header->next_header = NULL;
}

void HTTPServer_signal_stop(int signal)
{
    fprintf(stderr, "%s Catched signal %i\n", ERROR, signal);
    HTTPServer_delete(http_server_signal);
    http_server_signal = NULL;
    exit(0);
}

void HTTPHeader_add(HTTPHeader **header, size_t headers_amount, ...)
{
    va_list args;
    va_start(args, headers_amount);
    HTTPHeader *last = NULL;
    
    if (*header == NULL) {
        *header = malloc(sizeof(HTTPHeader));
        last = *header;
        const char *key = va_arg(args, char *);
        const char *value = va_arg(args, char *);
        HTTPHeader_create(last, key, value);
        --headers_amount;
    }
    else
        last = LinkedList_last(*header);

    for (; headers_amount > 0; --headers_amount) {
        last->next_header = malloc(sizeof(HTTPHeader));
        last = last->next_header;
        const char *key = va_arg(args, char *);
        const char *value = va_arg(args, char *);
        HTTPHeader_create(last, key, value);
    }
    va_end(args);
}

int HTTPResponse_send(ClientInterface *client, const HTTPResponse *response)
{
    if (client == NULL || response == NULL) {
        errno = EINVAL;
        return -1;
    }

    char *raw_response = malloc(response->content_length + HTTP_RES_BUF_LEN);
    char raw_header[HTTP_RES_BUF_LEN / 2];
    memset(raw_response, 0, response->content_length + HTTP_RES_BUF_LEN);
    sprintf(raw_response, "%s %d\r\n", response->version, response->status);
    if (response->content_length != 0) {
        memset(raw_header, 0, HTTP_RES_BUF_LEN / 2);
        sprintf(raw_header, "Content-Length: %i\r\n", response->content_length);
        strcat(raw_response, raw_header);
    }

    for (HTTPHeader *header = response->headers; header != NULL; header = header->next_header) {
        memset(raw_header, 0, HTTP_RES_BUF_LEN / 2);
        sprintf(raw_header, "%s: %s\r\n", header->key, header->value);
        strcat(raw_response, raw_header);
    }
    strcat(raw_response, "\r\n");
    size_t response_len = strlen(raw_response);
    
    if (response->body != NULL)
        memcpy(&raw_response[response_len], response->body, response->content_length);

    if (response->content_length != 0)
        response_len += response->content_length;

    if (send(client->_socket_descriptor, raw_response, response_len, 0) != response_len)
        return -1;

    free(raw_response);

    return 0;
}

void HTTPResponse_error(HTTPResponse *response, uint16_t status, const char *message)
{
    response->status = status;
    response->version = "HTTP/1.1";
    response->headers = NULL;

    size_t length = strlen(message);
    response->body = malloc(length);
    strncpy(response->body, message, length);
    HTTPHeader_add(&response->headers, 2, "Connection", "Closed", "Content-Type", "text/html");
}

void HTTPResponse_clean(HTTPResponse *response)
{
    if (response == NULL)
        return ;

    if (response->body != NULL)
        free(response->body);

    if (response->headers != NULL)
        LinkedList_delete(response->headers, &HTTPHeader_delete);
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

LinkedList *LinkedList_last(const LinkedList *ll)
{
    for (; ll->next != NULL; ll = ll->next);
    return ll;
}

void LinkedList_delete(LinkedList *ll, Deleter *deleter)
{
    if (ll->next != NULL)
        LinkedList_delete(ll->next, deleter);

    (*deleter)(ll);
}
