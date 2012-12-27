#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>

#include "server.h"
#include "protocol.h"
#include "storage.h"

static pool_t g_pool;

static void queue(int client_socket)
{
    struct s_queue_node* node;

    node = malloc(sizeof(struct s_queue_node));
    if (node == NULL)
    {
        fprintf(stderr, "Unable to allocate memory for queue node.\n");
        return;
    }

    node->client_socket = client_socket;
    printf("Queuing %d\n", client_socket);

    pthread_mutex_lock(&g_pool.queue_lock);
    node->next = g_pool.queue.head;
    if (g_pool.queue.head)
        g_pool.queue.head->previous = node;
    g_pool.queue.head = node;
    if (g_pool.queue.tail == NULL)
        g_pool.queue.tail = g_pool.queue.head;
    pthread_mutex_unlock(&g_pool.queue_lock);
    
    sem_post(&g_pool.semaphore);
}

static int dequeue()
{
    int                     client_socket;
    struct s_queue_node*    previous;

    sem_wait(&g_pool.semaphore);

    pthread_mutex_lock(&g_pool.queue_lock);
    client_socket = g_pool.queue.tail->client_socket;
    previous = g_pool.queue.tail->previous;
    if (previous)
        previous->next = NULL;
    free(g_pool.queue.tail);
    g_pool.queue.tail = previous;
    if (g_pool.queue.tail == NULL)
        g_pool.queue.head = NULL;
    pthread_mutex_unlock(&g_pool.queue_lock);

    printf("Dequeueing %d\n", client_socket);

    return client_socket;
}

static void* handle_client(void* data)
{
    int                     client_socket;
    int                     result = 0;
    char*                   key = NULL;
    void*                   value = NULL;
    struct s_request_header header;

    while (1)
    {
        client_socket = dequeue();
        while (1)
        {
            result = recv(client_socket, &header.command, sizeof(header.command), MSG_WAITALL);
            if (result != sizeof(header.command))
            {
                fprintf(stderr, "Unable to read command.\n");
                break;
            }

            if (header.command & COMMAND_DISCONNECT)
                break;

            result = recv(client_socket, &header.k, sizeof(header.k), MSG_WAITALL);
            if (result != sizeof(header.k))
            {
                fprintf(stderr, "Unable to read key.\n");
                break;
            }

            key = malloc(1 + header.k.key_length * sizeof(char));
            if (key == NULL)
            {
                fprintf(stderr, "Out of memory while allocating %d bytes for the requested key.\n", header.k.key_length);
                break;
            }

            if (header.command & COMMAND_SET)
            {
                result = recv(client_socket, &header.kv.value_length, sizeof(header.kv.value_length), MSG_WAITALL);
                if (result != sizeof(header.kv.value_length))
                {
                    fprintf(stderr, "Unable to read command.\n");
                    break;
                }

                value = malloc(header.kv.value_length * sizeof(char));
                if (value == NULL)
                {
                    fprintf(stderr, "Out of memory while allocating %d bytes for the requested key.\n", header.kv.value_length);
                    break;
                }
            }

            result = recv(client_socket, key, header.k.key_length, MSG_WAITALL);
            if (result != header.k.key_length)
            {
                fprintf(stderr, "Unable to read key.\n");
                break;
            }
            key[header.k.key_length] = 0;

            if (header.command & COMMAND_GET)
            {
                struct s_response_header response_header;

                memset(&response_header, 0, sizeof(struct s_response_header));
                response_header.value_length = storage_get(key, &value);

                result = send(client_socket, &response_header, sizeof(struct s_response_header), 0);
                if (result != sizeof(struct s_response_header))
                {
                    fprintf(stderr, "Unable to send response.\n");
                    break;
                }

                if (value)
                {
                    result = send(client_socket, value, response_header.value_length, 0);
                    if (result != response_header.value_length)
                    {
                        fprintf(stderr, "Unable to send value.\n");
                        break;
                    }

                    // Allocated by the filesystem
                    free(value);
                }
            }

            if (header.command & COMMAND_SET)
            {
                result = recv(client_socket, value, header.kv.value_length, MSG_WAITALL);
                if (result != header.kv.value_length)
                {
                    fprintf(stderr, "Unable to read value\n");
                    break;
                }

                storage_set(key, value, header.kv.value_length);
                free(value);
            }

            if (header.command & COMMAND_DELETE)
                storage_delete(key);

            free(key);
        }

        result = close(client_socket);
        if (result < 0)
        {
            fprintf(stderr, "Unable to close client socket.\n");
            return NULL;;
        }
    }

    return NULL;
}

int start_server(unsigned short listening_port)
{
    int                 result = 0;
    int                 listening_socket;
    struct sockaddr_in  socket_address;

    // Start listening
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

    // Create thread pool
    sem_init(&g_pool.semaphore, 0, 0);
    for (unsigned int i = 0; i < THREAD_POOL_SIZE; ++i)
        pthread_create(&g_pool.threads[i], NULL, handle_client, NULL);
    pthread_mutex_init(&g_pool.queue_lock, NULL);
    g_pool.queue.head = NULL;
    g_pool.queue.tail = NULL;

    // Server loop
    while (1)
    {
        int client_socket = accept(listening_socket, NULL, NULL);

        if (client_socket < 0)
        {
            fprintf(stderr, "Unable to accept on listening socket\n");
            close(listening_socket);
            return 2;
        }

        queue(client_socket);
    }

    sem_destroy(&g_pool.semaphore);
    pthread_mutex_lock(&g_pool.queue_lock);
    while (g_pool.queue.head)
    {
        struct s_queue_node* next = g_pool.queue.head;

        free(g_pool.queue.head);
        g_pool.queue.head = next;
    }
    pthread_mutex_unlock(&g_pool.queue_lock);
    pthread_mutex_destroy(&g_pool.queue_lock);

    return 0;
}
