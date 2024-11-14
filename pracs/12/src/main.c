#include <errno.h>
#include <stdlib.h>

#include "status.h"
#include "http_server/http_server.h"

HTTPResponse *get_image_handler(HTTPRequest *request)
{
    return NULL; 
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
    HTTPServer_create(http, host, port, 0, 100);
    HTTPServer_add_handle(http, "/image", 1, http_method_get, &get_image_handler);
    HTTPServer_process(http);

    // char buffer[] = "POST /en-US/docs/Web/HTTP/Messages HTTP/2\r\n"
    //                 "Host: developer.mozilla.org\r\n"
    //                 "User-Agent: Mozilla/5.0 (X11; Ubuntu; Linux x86_64; rv:132.0) Gecko/20100101 Firefox/132.0\r\n"
    //                 "\r\n"
    //                 "test=lol";
    // HTTPRequest request;
    // HTTPRequest_parse(&request, buffer);
    
    // // Delets all contents of request
    // HTTPRequest_clean(&request);
    
    return 0;
}