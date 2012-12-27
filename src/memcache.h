#ifndef MEMCACHE_H_
# define MEMCACHE_H_

# include <stdbool.h>
# include <stdint.h>
# include <time.h>

# include "config.h"

# define MEMCACHE_NODE_INDEX         0x1
# define MEMCACHE_NODE_BLOCK         0x2
# define MEMCACHE_NODE_LEAF          0x4

struct s_bptree_node
{
    uint8_t     type;
    clock_t     access_time;
    union
    {
        struct
        {
            uint8_t                 nb_keys;
            char*                   keys[CACHE_BPTREE_ORDER];
            struct s_bptree_node*   children[CACHE_BPTREE_ORDER + 1];
        } index;
        struct
        {
            size_t  size;
            void*   data;
        } block;
    };
};

typedef struct
{
    struct s_bptree_node root;
} memcache_t;

bool  memcache_init();
size_t memcache_get(const char* key, void** data);
bool memcache_put(const char* key, const void* data, size_t nb_bytes);
bool memcache_invalidate(const char* key);

#endif /* !MEMCACHE_H_ */
