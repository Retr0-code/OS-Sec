#include <errno.h>
#include <stdlib.h>
#include <getopt.h>
#include <unistd.h>
#include <sys/stat.h>

#include "status.h"
#include "http_server/http_server.h"

#define EMBLEM_PATH         "./static/images/emblem.jpeg"
#define ROOT_TEMPLATE_PATH  "./static/templates/get_root_template.form.html"

int handler_get_emblem(const HTTPRequest *request, HTTPResponse *response)
{
    response->status = 200;
    response->version = "HTTP/1.1";
    response->headers = NULL;

    FILE *emblem_file = fopen(EMBLEM_PATH, "r");
    if (emblem_file == NULL) {
        HTTPResponse_error(response, 500, "Internal server error");
        return response->status;
    }
    fseek(emblem_file, 0, SEEK_END);
    size_t emblem_size = ftell(emblem_file);
    fseek(emblem_file, 0, SEEK_SET);

    response->body = malloc(emblem_size);
    if (fread(response->body, emblem_size, 1, emblem_file) != 1) {
        HTTPResponse_error(response, 500, "Internal server error");
        fclose(emblem_file);
        return response->status;
    }
    fclose(emblem_file);
    response->content_length = emblem_size;

    HTTPHeader_add(&response->headers, 1, "Content-Type", "image/jpeg");

    return response->status;
}

int handler_get_root(const HTTPRequest *request, HTTPResponse *response)
{
    response->status = 200;
    response->version = "HTTP/1.1";
    response->headers = NULL;

    int argc;
    char **argv;
    HTTPRequest_parse_get_args(request, &argc, &argv);

    if (argc != 4) {
        HTTPRequest_delete_args(argc, argv);
        HTTPResponse_error(response, 400, "Bad Request");
        return response->status;
    }

    char *full_name = NULL;
    char *group = NULL;
    char *course = NULL;

    struct option long_options[] = {
        {"full_name",   required_argument, 0, 0},
        {"group",       required_argument, 0, 1},
        {"course",      required_argument, 0, 2},
        {0, 0, 0, 0}
    };

    int option, option_index;
    optind = 1;
    while ((option = getopt_long_only(argc, argv, "", long_options, &option_index)) != -1) {
        switch (option_index) {
            case 0:
                full_name = optarg;
                break;
            case 1:
                group = optarg;
                break;
            case 2:
                course = optarg;
                break;
            default:
                HTTPRequest_delete_args(argc, argv);
                HTTPResponse_error(response, 400, "Bad Request");
                return response->status;
        }
    }

    if (full_name == NULL || group == NULL || course == NULL) {
        HTTPRequest_delete_args(argc, argv);
        HTTPResponse_error(response, 400, "Bad Request");
        return response->status;
    }

    FILE *template_file = fopen(ROOT_TEMPLATE_PATH, "r");
    if (template_file == NULL) {
        HTTPResponse_error(response, 500, "Internal server error");
        return response->status;
    }
    fseek(template_file, 0, SEEK_END);
    size_t template_size = ftell(template_file);
    fseek(template_file, 0, SEEK_SET);

    size_t total_size = template_size + strlen(request->url);
    response->body = malloc(total_size);
    char *template = malloc(template_size + 1);
    memset(template, 0, template_size    + 1);
    if (fread(template, template_size, 1, template_file) != 1) {
        HTTPRequest_delete_args(argc, argv);
        fclose(template_file);
        HTTPResponse_error(response, 500, "Internal server error");
        return response->status;
    }
    fclose(template_file);

    snprintf(response->body, total_size, template, full_name, group, course);
    response->content_length = strlen(response->body);

    HTTPHeader_add(&response->headers, 1, "Content-Type", "text/html");
    HTTPRequest_delete_args(argc, argv);
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
    HTTPServer_add_handle(http, "/", 1, http_method_get, &handler_get_root);
    HTTPServer_process(http);
    
    return 0;
}
