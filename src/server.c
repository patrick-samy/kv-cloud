#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "server.h"
#include "protocol.h"
#include "storage.h"

static int handle_client(int client_socket)
{
    int                     result = 0;
    struct s_request_header header;
    char*                   key = NULL;
    char*                   value = NULL;

    while (1)
    {
        result = recv(client_socket, &header.command, sizeof(header.command), MSG_WAITALL);
        if (result != sizeof(header.command))
        {
            fprintf(stderr, "Unable to read command.\n");
            return 3;
        }

        if (header.command & COMMAND_DISCONNECT)
            break;

        result = recv(client_socket, &header.k, sizeof(header.k), MSG_WAITALL);
        if (result != sizeof(header.k))
        {
            fprintf(stderr, "Unable to read key.\n");
            return 3;
        }

        key = malloc(1 + header.k.key_length * sizeof(char));
        if (key == NULL)
        {
            fprintf(stderr, "Out of memory while allocating %d bytes for the requested key.\n", header.k.key_length);
            return 4;
        }

        if (header.command & COMMAND_SET)
        {
            result = recv(client_socket, &header.kv.value_length, sizeof(header.kv.value_length), MSG_WAITALL);
            if (result != sizeof(header.kv.value_length))
            {
                fprintf(stderr, "Unable to read command.\n");
                return 3;
            }

            value = malloc(1 + header.kv.value_length * sizeof(char));
            if (value == NULL)
            {
                fprintf(stderr, "Out of memory while allocating %d bytes for the requested key.\n", header.kv.value_length);
                return 4;
            }
        }

        result = recv(client_socket, key, header.k.key_length, MSG_WAITALL);
        if (result != header.k.key_length)
        {
            fprintf(stderr, "Unable to read key.\n");
            return 3;
        }
        key[header.k.key_length] = 0;

        if (header.command & COMMAND_GET)
        {
            struct s_response_header response_header;

            memset(&response_header, 0, sizeof(struct s_response_header));
            value = storage_get(key);
            if (value)
                response_header.value_length = strlen(value);

            result = send(client_socket, &response_header, sizeof(struct s_response_header), 0);
            if (result != sizeof(struct s_response_header))
            {
                fprintf(stderr, "Unable to send response.\n");
                return 3;
            }

            if (value)
            {
                result = send(client_socket, value, response_header.value_length, 0);
                if (result != response_header.value_length)
                {
                    fprintf(stderr, "Unable to send value.\n");
                    return 3;
                }
            }
        }

        if (header.command & COMMAND_SET)
        {
            result = recv(client_socket, value, header.kv.value_length, MSG_WAITALL);
            if (result != header.kv.value_length)
            {
                fprintf(stderr, "Unable to read value\n");
                return 3;
            }
            value[header.kv.value_length] = 0;

            storage_set(key, value);
        }

        if (header.command & COMMAND_DELETE)
        {
            storage_delete(key);
        }
    }

    return 0;
}

int start_server(unsigned short listening_port)
{
    int                 result = 0;
    int                 listening_socket;
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
        close(listening_socket);
        return 2;
    }

    result = listen(listening_socket, DEFAULT_LISTEN_QUEUE);
    if (result < 0)
    {
        fprintf(stderr, "Unable to start listening on port %d\n", listening_port);
        close(listening_socket);
        return 2;
    }

    while (1)
    {
        int client_socket = accept(listening_socket, NULL, NULL);

        if (client_socket < 0)
        {
            fprintf(stderr, "Unable to accept on listening socket\n");
            close(listening_socket);
            return 2;
        }

        result = handle_client(client_socket);
        if (result != 0)
        {
            close(client_socket);
            close(listening_socket);
            return result;
        }

        result = close(client_socket);
        if (result < 0)
        {
            fprintf(stderr, "Unable to close client socket.\n");
            return 2;
        }
    }

    return 0;
}
