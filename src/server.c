#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "server.h"

static int handle_client(int client_socket)
{
    int     result = 0;
    uint8_t command;

    while (recv(client_socket, &command, sizeof(uint8_t), 0) > 0)
    {
        printf("kvcloud: recv %d %c\n", command, command);
    }

    result = close(client_socket);
    if (result < 0)
    {
        fprintf(stderr, "Unable to close client socket.\n");
        return 2;
    }

    return 0;
}

int start_server()
{
    int                 result = 0;
    int                 listening_socket;
    unsigned short      listening_port = DEFAULT_PORT;
    struct sockaddr_in  socket_address;

    listening_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (listening_socket < 0)
    {
       fprintf(stderr, "Unable to create listening socket.\n");
       return 2;
    }

    memset(&socket_address, 0, sizeof(socket_address));
    socket_address.sin_family = AF_INET;
    socket_address.sin_addr.s_addr = htonl(INADDR_ANY);
    socket_address.sin_port = htons(listening_port);

    result = bind(listening_socket,
                  (struct sockaddr*) &socket_address,
                  sizeof(socket_address));
    if (result < 0)
    {
        fprintf(stderr, "Unable to bind listening socket on port %d\n", listening_port);
        return 2;
    }

    result = listen(listening_socket, DEFAULT_LISTEN_QUEUE);
    if (result < 0)
    {
        fprintf(stderr, "Unable to start listening on port %d\n", listening_port);
        return 2;
    }

    while (1)
    {
        int client_socket = accept(listening_socket, NULL, NULL);

        if (client_socket < 0)
        {
            fprintf(stderr, "Unable to accept on listening socket\n");
            return 2;
        }

        result = handle_client(client_socket);
        if (result != 0)
        {
            close(listening_socket);
            return result;
        }
    }

    return 0;
}
