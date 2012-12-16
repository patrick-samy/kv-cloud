#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "api.h"
#include "protocol.h"

static int g_client_socket;

int kvcloud_connect(const char* address, uint16_t port)
{
    int                 result;
    struct sockaddr_in  socket_address;

    if (g_client_socket)
    {
        fprintf(stderr, "An existing connection needs to be terminated by calling kvcloud_disconnect().\n");
        return -1;
    }

    g_client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (g_client_socket < 0)
    {
        fprintf(stderr, "Unable to create client socket\n");
        return -1;
    }

    memset(&socket_address, 0, sizeof(socket_address));
    socket_address.sin_family = AF_INET;
    socket_address.sin_addr.s_addr = htonl(inet_addr(address));
    socket_address.sin_port = htons(port);

    result = connect(g_client_socket, (struct sockaddr*) &socket_address, sizeof(socket_address));
    if (result < 0)
    {
        fprintf(stderr, "Unable to connect to the server %s:%d.\n", address, port);
        close(g_client_socket);
        return -1;
    }

    return g_client_socket;
}

void kvcloud_disconnect()
{
    int                     result;
    struct s_request_header header;

    if (g_client_socket < 0)
    {
        fprintf(stderr, "A connection must be established to call kvcloud_disconnect().\n");
        return;
    }

    memset(&header, 0, sizeof(struct s_request_header));
    header.command = COMMAND_DISCONNECT;
    result = send(g_client_socket, &header, sizeof(header.command), 0);
    if (result != sizeof(header.command))
    {
        fprintf(stderr, "Unable to send disconnect command.\n");
    }

    result = close(g_client_socket);
    if (result < 0)
    {
        fprintf(stderr, "Unable to close the client socket.");
    }

    g_client_socket = -1;
}

char* kvcloud_get(const char* key)
{
    int                         result;
    struct s_request_header     header;
    struct s_response_header    response_header;
    char*                       value;

    if (g_client_socket < 0)
    {
        fprintf(stderr, "A connection must be established before calling kvcloud_get().\n");
        return NULL;
    }

    memset(&header, 0, sizeof(struct s_request_header));
    header.command = COMMAND_GET;
    header.k.key_length = strlen(key);
    result = send(g_client_socket, &header, sizeof(header.command) + sizeof(header.k), 0);
    if (result != sizeof(header.command) + sizeof(header.k))
    {
        fprintf(stderr, "Unable to send get request.\n");
        return NULL;
    }

    result = send(g_client_socket, key, header.k.key_length, 0);
    if (result != header.k.key_length)
    {
        fprintf(stderr, "Unable to send key for request.\n");
    }

    result = recv(g_client_socket, &response_header, sizeof(struct s_response_header), MSG_WAITALL);
    if (result != sizeof(struct s_response_header))
    {
        fprintf(stderr, "Unable to receive get response.\n");
        return NULL;
    }

    if (response_header.value_length == 0)
    {
        fprintf(stderr, "Unable to find value for key %s.\n", key);
        return NULL;
    }

    value = malloc(1 + response_header.value_length * sizeof(char));
    if (value == NULL)
    {
        fprintf(stderr, "Out of memory while allocating value buffer of %d bytes.\n", response_header.value_length);
        return NULL;
    }

    result = recv(g_client_socket, value, response_header.value_length, MSG_WAITALL);
    if (result != response_header.value_length)
    {
        fprintf(stderr, "Unable to receive value.\n");
        return NULL;
    }

    value[response_header.value_length] = 0;

    return value;
}

void kvcloud_set(const char* key, const char* value)
{

}

void kvcloud_delete(const char* key)
{

}

