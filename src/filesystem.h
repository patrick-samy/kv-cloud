#ifndef FILESYSTEM_H_
# define FILESYSTEM_H_

# include <stdint.h>
# include <stdbool.h>
# include <sys/types.h>

# include "config.h"

# define FS_NODE_INDEX         0x1
# define FS_NODE_BLOCK         0x2
# define FS_NODE_LEAF          0x4

# define FS_CHILDREN_PRESENT   0x1

struct s_fs_key
{
    uint8_t size;
    char    data[KEY_SIZE + 1]; 
} __attribute__((packed));

struct s_fs_node
{
    uint8_t type;
    union
    {
        struct
        {
            uint8_t         nb_keys;
            struct s_fs_key keys[FS_BPTREE_ORDER];
            off_t           children[FS_BPTREE_ORDER + 1];
        } index __attribute__((packed));
        struct
        {
            size_t  size;
        } block __attribute__((packed));
    };
} __attribute__((packed));

struct s_fs_header
{
    size_t  total_size;
    off_t   root;
} __attribute__((packed));

typedef struct
{
    FILE*               file;
    struct s_fs_header  header;
} fs_file_t;

bool fs_init(const char* img_filepath);
bool fs_open(const char* img_filepath);
bool fs_close();
size_t fs_search(const char* key, void** data);
bool fs_add(const char* key, const void* data, size_t nb_bytes);
bool fs_delete(const char* key);

#endif /* !FILESYSTEM_H_ */

