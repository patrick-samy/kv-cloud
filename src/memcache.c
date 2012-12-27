#include "memcache.h"

static memcache_t g_memcache;

/*
static memcache_update_node(struct s_bptree_node* node)
{
    node->access_time = clock();
}
*/
bool memcache_init()
{
    g_memcache.root.type = MEMCACHE_NODE_INDEX | MEMCACHE_NODE_LEAF;
    g_memcache.root.access_time = 0;

    return true;
}

size_t memcache_get(const char* key, void** data)
{
    *data = NULL;
    return 0;
}

bool memcache_put(const char* key, const void* data, size_t nb_bytes)
{
    return false;
}

bool memcache_invalidate(const char* key)
{
    return false;
}

