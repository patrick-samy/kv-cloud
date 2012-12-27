#ifndef SERVER_H_
# define SERVER_H_

# include <pthread.h>
# include <semaphore.h>

# include "config.h"

# define DEFAULT_LISTEN_QUEUE 1024

struct s_queue_node
{
    int                     client_socket;
    struct s_queue_node*    next;
    struct s_queue_node*    previous;
};

struct s_queue
{
    struct s_queue_node*    head;
    struct s_queue_node*    tail;
};

typedef struct
{
    sem_t           semaphore;
    pthread_t       threads[THREAD_POOL_SIZE];
    pthread_mutex_t queue_lock;
    struct s_queue  queue;
} pool_t;

int start_server(unsigned short listening_port);

#endif /* !SERVER_H_ */
