#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "api.h"
#include "protocol.h"

int kvcloud_connect(const char* address, uint16_t port)
{
    int                 result;
    int                 client_socket;
    struct sockaddr_in  socket_address;

    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket < 0)
    {
        fprintf(stderr, "Unable to create client socket\n");
        return -1;
    }

    memset(&socket_address, 0, sizeof(socket_address));
    socket_address.sin_family = AF_INET;
    socket_address.sin_addr.s_addr = htonl(inet_addr(address));
    socket_address.sin_port = htons(port);

    result = connect(client_socket, (struct sockaddr*) &socket_address, sizeof(socket_address));
    if (result < 0)
    {
        fprintf(stderr, "Unable to connect to the server %s:%d\n", address, port);
        close(client_socket);
        return -1;
    }

    return client_socket;
}

void kvcloud_disconnect(int client_socket)
{
    int                     result;
    struct s_request_header header;

    header.command = COMMAND_DISCONNECT;
    result = send(client_socket, &header, sizeof(header.command), 0);
    if (result != sizeof(header.command))
    {
        fprintf(stderr, "Unable to send disconnect command\n");
    }

    result = close(client_socket);
    if (result < 0)
    {
        fprintf(stderr, "Unable to close the client socket");
    }
}

char* kvcloud_get(char* key)
{
    return NULL;
}

void kvcloud_set(char* key, char* value)
{

}

void kvcloud_delete(char* key)
{

}

