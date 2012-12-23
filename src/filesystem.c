#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "filesystem.h"

static fs_file_t* g_fs = NULL;

static bool fs_read_index(off_t offset, struct s_fs_node* node)
{
    int result;

    result = fseek(g_fs->file, offset, SEEK_SET);
    if (result < 0)
    {
        fprintf(stderr, "Unable to seek to index node.\n");
        return false;
    }

    result = fread(&node->type, sizeof(node->type), 1, g_fs->file);
    if (result != 1)
    {
        fprintf(stderr, "Unable to read node type.\n");
        return false;
    }

    if (node->type & NODE_INDEX)
    {
        result = fread(&node->index, sizeof(node->index), 1, g_fs->file);
        if (result != 1)
        {
            fprintf(stderr, "Unable to read index.\n");
            return false;
        }
    }

    return true;
}

static bool fs_write_index(off_t offset, struct s_fs_node* node)
{
    int result;

    result = fseek(g_fs->file, offset, SEEK_SET);
    if (result < 0)
    {
        fprintf(stderr, "Unable to seek to index node.\n");
        return false;
    }

    result = fwrite(&node->type, sizeof(node->type), 1, g_fs->file);
    if (result != 1)
    {
        fprintf(stderr, "Unable to write node type.\n");
    }

    if (node->type & NODE_INDEX)
    {
        result = fwrite(&node->index, sizeof(node->index), 1, g_fs->file);
        if (result != 1)
        {
            fprintf(stderr, "Unable to write index.\n");
            return false;
        }
    }
    
    result = fflush(g_fs->file);
    if (result != 0)
    {
        fprintf(stderr, "Unable to flush fs file");
        return false;
    }

    return true;
}

bool fs_init(const char* img_filepath)
{
    int                 result;
    struct s_fs_header  header;
    struct s_fs_node    root;
    FILE*               f;

    f = fopen(img_filepath, "w");
    if (f == NULL)
    {
        fprintf(stderr, "Unable to open file %s\n", img_filepath);
        return false;
    }

    header.total_size = sizeof(header) + sizeof(root);
    header.root = sizeof (header);

    result  = fwrite(&header, sizeof(struct s_fs_header), 1, f);
    if (result != 1)
    {
        fprintf(stderr, "Unable to read write header.\n");
        fs_close();
        return false;
    }
    
    memset(&root, 0, sizeof(root.type) + sizeof(root.index));
    root.type = NODE_INDEX | FLAG_LEAF;
    root.index.nb_keys = 0;

    result = fwrite(&root, sizeof(root.type) + sizeof(root.index), 1, f);
    if (result != 1)
    {
        fprintf(stderr, "Unable to write root node.\n");
        fs_close();
        return false;
    }

    result = fclose(f);
    if (result != 0)
    {
        fprintf(stderr, "Unable to close file.\n");
        return false;
    }

    return true;
   
}

bool fs_open(const char* img_filepath)
{
    int result;

    if (g_fs != NULL)
    {
        fprintf(stderr, "A file is already opened.\n");
        return false;
    }

    g_fs = malloc(sizeof(fs_file_t));
    if (g_fs == NULL)
    {
        fprintf(stderr, "Unable to allocate memory.\n");
        return false;
    }

    g_fs->file = fopen(img_filepath, "r+");
    if (g_fs->file == NULL)
    {
        fprintf(stderr, "Unable to open file %s, the filesystem may have not been initialized.\n", img_filepath);
        free(g_fs);
        g_fs = NULL;
        return false;
    }

    result = fread(&g_fs->header, sizeof(struct s_fs_header), 1, g_fs->file);
    if (result != 1)
    {
        fprintf(stderr, "Unable to read fs header.\n");
        fs_close();
        return false;
    }

    return true;
}

bool fs_close()
{
    int result;

    if (g_fs == NULL)
    {
        fprintf(stderr, "A file must be open before calling fs_close().\n");
        return false;
    }

    result = fclose(g_fs->file);
    if (result != 0)
    {
        fprintf(stderr, "Unable to close the filesystem file.\n");
        return false;
    }

    free(g_fs);
    g_fs = NULL;

    return true;
}

static bool fs_search_recurse(const char*       key,
                              struct s_fs_node* node,
                              char**            data)
{
    int result;

    if (node->type & NODE_INDEX)
    {
        size_t  i;
        int     distance;
        
        for (i = 0; i < node->index.nb_keys; ++i)
        {
            distance = strcmp(key, node->index.keys[i].data);
            if (distance <= 0)
                break;
        }
        
        if (node->type & FLAG_LEAF)
        {
            if (distance == 0)
            {
                struct s_fs_node data_node;

                result = fseek(g_fs->file, node->index.children[i], SEEK_SET);
                if (result < 0)
                {
                    fprintf(stderr, "Unable to seek to children index.\n");
                    return false;
                }

                result = fread(&data_node.block, sizeof(data_node.block), 1, g_fs->file);
                if (result != 1)
                {
                    fprintf(stderr, "Unable to read block node.\n");
                    return false;
                }

                *data = malloc(data_node.block.size + 1);
                if (*data == NULL)
                {
                    fprintf(stderr, "Unable to allocate memory for data block of %d bytes.\n", data_node.block.size + 1);
                    return false;
                }

                result = fread(*data, data_node.block.size + 1, 1, g_fs->file);
                if (result != 1)
                {
                    fprintf(stderr, "Unable to read data in block node.\n");
                    return false;
                }

                return true;
            }
        }
        else
        {
            struct s_fs_node child;

            if (!fs_read_index(node->index.children[i], &child))
                return false;
            
            return fs_search_recurse(key, &child, data);
        }
    }
 
    return false;
}

char* fs_search(const char* key)
{
    struct s_fs_node    root;
    char*               value = NULL;

    if (g_fs == NULL)
    {
        fprintf(stderr, "A file must be opened before calling fs_search().\n");
        return false;
    }
    
    if (!fs_read_index(g_fs->header.root, &root))
        return false;

    fs_search_recurse(key, &root, &value);
    
    return value;
}

static bool fs_add_recurse(const char*          key,
                           const char*          data,
                           off_t                node_offset,
                           struct s_fs_node*    node,
                           struct s_fs_node*    r_node)
{
    int     result;
    bool    splitted = false;
    bool    writeback = false;

    if (node->type & NODE_INDEX)
    {
        size_t i;
        
        for (i = 0; i < node->index.nb_keys; ++i)
        {
            if (strcmp(key, node->index.keys[i].data) <= 0)
                break;
        }
        
        if (node->type & FLAG_LEAF)
        {
            struct s_fs_node data_node;

            ++node->index.nb_keys;
            for (size_t j = node->index.nb_keys - 1; j > i; --j)
            {
                node->index.keys[j] = node->index.keys[j - 1];
                node->index.children[j] = node->index.children[j - 1];
            }

            node->index.keys[i].size = strlen(key);
            strcpy(node->index.keys[i].data, key);
            result = fseek(g_fs->file, 0, SEEK_END);
            if (result < 0)
                fprintf(stderr, "Unable to seek to end of file.\n");

            node->index.children[i] = ftell(g_fs->file);
            data_node.block.size = strlen(data);
            result = fwrite(&data_node.block, sizeof(data_node.block), 1, g_fs->file);
            if (result != 1)
                fprintf(stderr, "Unable to write block node.\n");

            result = fwrite(data, data_node.block.size + 1, 1, g_fs->file);
            if (result != 1)
                fprintf(stderr, "Unable to write block data.\n");

            if (fflush(g_fs->file) < 0)
                fprintf(stderr, "Unable to flush to file.\n");

            writeback = true;
        }
        else
        {
            struct s_fs_node child;
            struct s_fs_node new_node;

            fs_read_index(node->index.children[i], &child);
            if (fs_add_recurse(key, data, node->index.children[i], &child, &new_node))
            {
                // Add splitted node
                ++node->index.nb_keys;
                for (size_t j = node->index.nb_keys - 1; j > i; --j)
                {
                    node->index.keys[j] = node->index.keys[j - 1];
                    node->index.children[j] = node->index.children[j - 1];
                }

                node->index.keys[i].size = new_node.index.keys[0].size;
                strcpy(node->index.keys[i].data, new_node.index.keys[0].data);
                result = fseek(g_fs->file, 0, SEEK_END);
                if (result < 0)
                    fprintf(stderr, "Unable to flush to file.\n");

                node->index.children[i] = ftell(g_fs->file);
                result = fwrite(&new_node, sizeof(new_node.type) + sizeof(new_node.index), 1, g_fs->file);
                if (result != 1)
                    fprintf(stderr, "Unable to write node type.\n");

                result = fflush(g_fs->file);
                if (result < 0)
                    fprintf(stderr, "Unable to flush to file.\n");

                writeback = true;
            }
        }
        
        // Split if necessary
        if (node->index.nb_keys > BTREE_ORDER - 1)
        {
            memset(r_node, 0, sizeof(struct s_fs_node));
            r_node->type = node->type;
            r_node->index.nb_keys = node->index.nb_keys % 2 + node->index.nb_keys / 2;
            for (size_t j = 1; j <= r_node->index.nb_keys; ++j)
            {
                r_node->index.keys[r_node->index.nb_keys - j].size = node->index.keys[node->index.nb_keys - j].size;
                strcpy(r_node->index.keys[r_node->index.nb_keys - j].data, node->index.keys[node->index.nb_keys - j].data);
                r_node->index.children[r_node->index.nb_keys - j] = node->index.children[node->index.nb_keys - j];
            }

            for (size_t j = node->index.nb_keys - 1; j >= node->index.nb_keys / 2; --j)
                node->index.children[j] = 0;
            node->index.nb_keys = node->index.nb_keys / 2;

            writeback = true;
            splitted = true;
        }

        // Write node back
        if (writeback)
            fs_write_index(node_offset, node);

    }
    
    return splitted;
}

bool fs_add(const char* key, const char* data)
{
    int                 result;
    struct s_fs_node    root;
    struct s_fs_node    node;

    if (g_fs == NULL)
    {
        fprintf(stderr, "A file must be opened before calling fs_add().\n");
        return false;
    }
    
    if (!fs_read_index(g_fs->header.root, &root))
        return false;

    if (fs_add_recurse(key, data, g_fs->header.root, &root, &node))
    {
        struct s_fs_node new_root;

        memset(&new_root, 0, sizeof(struct s_fs_node));
        new_root.type = NODE_INDEX;
        new_root.index.nb_keys = 1;
        new_root.index.keys[0].size = node.index.keys[0].size;
        strcpy(new_root.index.keys[0].data, node.index.keys[0].data);
        new_root.index.children[0] = g_fs->header.root;
        result = fseek(g_fs->file, 0, SEEK_END);
        if (result < 0)
        {
            fprintf(stderr, "Unable to seek to end of file.\n");
            return false;
        }

        new_root.index.children[1] = ftell(g_fs->file);
        result = fwrite(&node, sizeof(node.type) + sizeof(node.index), 1, g_fs->file);
        if (result != 1)
        {
            fprintf(stderr, "Unable to write node type.\n");
            return false;
        }

        result = fflush(g_fs->file);
        if (result < 0)
        {
            fprintf(stderr, "Unable to flush file.\n");
            return false;
        }

        result = fseek(g_fs->file, 0, SEEK_END);
        if (result < 0)
        {
            fprintf(stderr, "Unable to seek to end of file.\n");
            return false;
        }

        g_fs->header.root = ftell(g_fs->file);
        result = fwrite(&new_root, sizeof(new_root.type) + sizeof(new_root.index), 1, g_fs->file);
        if (result != 1)
        {
            fprintf(stderr, "Unable to tell current position in file.\n");
            return false;
        }

        result = fflush(g_fs->file);
        if (result < 0)
        {
            fprintf(stderr, "Unable to flush to file.\n");
            return false;
        }

        result = fseek(g_fs->file, 0, SEEK_SET);
        if (result < 0)
        {
            fprintf(stderr, "Unable to seek to beginning of file.\n");
            return false;
        }

        result = fwrite(&g_fs->header, sizeof(struct s_fs_header), 1, g_fs->file);
        if (result != 1)
        {
            fprintf(stderr, "Unable to write filesystem header.\n");
            return false;
        }

        result = fflush(g_fs->file);
        if (result < 0)
        {
            fprintf(stderr, "Unable to flush to file.\n");
            return false;
        }
    }

    return true;
}

bool fs_delete(const char* key)
{
    return false;
}

