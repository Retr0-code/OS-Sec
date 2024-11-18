#include <errno.h>
#include <stdlib.h>

#include "status.h"
#include "http_server/http_server.h"

#define EMBLEM_PATH "./static/images/emblem.jpeg"

int handler_get_emblem(const HTTPRequest *request, HTTPResponse *response)
{
    response->status = 200;
    response->version = "HTTP/1.1";
    response->headers = NULL;

    size_t emblem_size;
    FILE *emblem_file = fopen(EMBLEM_PATH, "r");
    fseek(emblem_file, 0, SEEK_END);
    emblem_size = ftell(emblem_file);
    fseek(emblem_file, 0, SEEK_SET);

    response->body = malloc(emblem_size);
    if (fread(response->body, emblem_size, 1, emblem_file) != 1) {
        response->status = 500;
        fclose(emblem_file);
        return response->status;
    }
    fclose(emblem_file);
    response->content_length = emblem_size;

    HTTPHeader_add(&response->headers, 1, "Content-Type", "image/jpeg");

    return response->status;
}

int main(int argc, char **argv)
{
    if (argc != 3)
    {
        fprintf(stderr, "%s Only two positional parameters are required <host port>, but %i, were given\n", ERROR, argc);
        return -1;
    }

    const char *host = argv[1];
    uint16_t port = atoi(argv[2]);
    if (port == 0 || errno != 0)
    {
        fprintf(stderr, "%s Invalid port format %s:\t%s\n", ERROR, argv[2], strerror(errno));
        return -1;
    }

    HTTPServer *http = malloc(sizeof(HTTPServer));
    if (HTTPServer_create(http, host, port, 0, 100)) {
        free(http);
        fprintf(stderr, "%s Unable to start HTTP server:\n%s\n", ERROR, strerror(errno));
        return -1;
    }

    HTTPServer_add_handle(http, "/emblem", 1, http_method_get, &handler_get_emblem);
    HTTPServer_process(http);
    
    return 0;
}
