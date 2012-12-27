#include <unistd.h>
#include <stdio.h>
#include <pthread.h>

#include "storage.h"
#include "filesystem.h"
#include "memcache.h"

static pthread_mutex_t g_storage_lock;

size_t storage_get(char* key, void** value)
{
    size_t nb_bytes = 0;

    pthread_mutex_lock(&g_storage_lock);
    nb_bytes = memcache_get(key, value);
    if (*value == NULL)
    {
        nb_bytes = fs_search(key, value);
        if (value != NULL)
            memcache_put(key, value, nb_bytes);
    }
    pthread_mutex_unlock(&g_storage_lock);
    
    return nb_bytes;
}

void storage_set(char* key, void* value, size_t nb_bytes)
{
    pthread_mutex_lock(&g_storage_lock);
    fs_add(key, value, nb_bytes);
    memcache_put(key, value, nb_bytes);
    pthread_mutex_unlock(&g_storage_lock);
}

void storage_delete(char* key)
{
    pthread_mutex_lock(&g_storage_lock);
    fs_delete(key);
    memcache_invalidate(key);
    pthread_mutex_unlock(&g_storage_lock);
}

