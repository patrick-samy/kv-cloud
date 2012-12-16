#ifndef FILESYSTEM_H_
# define FILESYSTEM_H_

# include <stdint.h>

# include "config.h"

# define BTREE_ORDER    25

# define NODE_INDEX     0x1
# define NODE_BLOCK     0x2

struct s_key
{
    uint8_t size;
    char    data[KEY_SIZE]; 
} __attribute__((packed));

struct s_fs_node
{
    uint8_t type;
    union
    {
        struct
        {
            size_t          nb_keys;
            struct s_key    keys[BTREE_ORDER - 1];
            size_t          nb_children;
            size_t          children[BTREE_ORDER];
        } index __attribute__((packed));
        struct
        {
            size_t  size;
        } block __attribute__((packed));
    };
} __attribute__((packed));

struct s_fs_header
{
    size_t total_size;
    size_t root;
} __attribute__((packed));

typedef struct
{
    FILE*               file;
    struct s_fs_header  header;
} fs_file_t;

int fs_init(const char* img_filepath);
int fs_open(const char* img_filepath);
int fs_close();
char* fs_search(const char* key);
int fs_add(const char* key, const char* data);
int fs_delete(const char* key);

#endif /* !FILESYSTEM_H_ */

