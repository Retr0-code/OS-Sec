#pragma once
#include <stdio.h>
#include <string.h>

#include <unistd.h>
#include <sys/socket.h>

#include "server/server.h"

// typedef struct Server;

typedef struct
{
    uint32_t        _id;
    int32_t         _socket_descriptor;
    Server *const   _server;
} ClientInterface;

int ClientInterface_create(ClientInterface *client, int sock_fd, Server *const server);

void ClientInterface_close_connection(ClientInterface *client);
