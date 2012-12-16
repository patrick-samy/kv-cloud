#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "filesystem.h"

static fs_file_t* g_fs_file = NULL;

int fs_init(const char* img_filepath)
{
    int                 result;
    struct s_fs_header  header;
    struct s_fs_node    root;
    FILE*               f;

    f = fopen(img_filepath, "w");
    if (f == NULL)
    {
        fprintf(stderr, "Unable to open file %s\n", img_filepath);
        return 0;
    }

    header.total_size = sizeof(header) + sizeof(root);
    header.root = sizeof (header);

    result  = fwrite(&header, sizeof(struct s_fs_header), 1, f);
    if (result != 1)
    {
        fprintf(stderr, "Unable to read fs header.\n");
        fs_close();
        return 0;
    }

    root.type = NODE_INDEX;
    root.index.nb_keys = 0;
    root.index.nb_children = 0;

    // TODO: Unhack
    result = fwrite(&root.type, sizeof(root.type), 1, f);
    result = fwrite(&root.index, sizeof(root.index), 1, f);
    if (result != 1)
    {
        fprintf(stderr, "Unable to read fs header.\n");
        fs_close();
        return 0;
    }

    fclose(f);

    return 1;
   
}

int fs_open(const char* img_filepath)
{
    int result;

    if (g_fs_file != NULL)
    {
        fprintf(stderr, "A file is already opened.\n");
        return 0;
    }

    g_fs_file = malloc(sizeof(fs_file_t));
    if (g_fs_file == NULL)
    {
        fprintf(stderr, "Out of memory.\n");
        return 0;
    }

    g_fs_file->file = fopen(img_filepath, "r+");
    if (g_fs_file->file == NULL)
    {
        fprintf(stderr, "Unable to open file %s\n", img_filepath);
        free(g_fs_file);
        g_fs_file = NULL;
        return 0;
    }

    result  = fread(&g_fs_file->header, sizeof(struct s_fs_header), 1, g_fs_file->file);
    if (result != 1)
    {
        fprintf(stderr, "Unable to read fs header.\n");
        fs_close();
        return 0;
    }

    return 1;
}

int fs_close()
{
    int result;

    if (g_fs_file == NULL)
    {
        fprintf(stderr, "A file must be open before calling fs_close().\n");
        return 0;
    }

    result = fclose(g_fs_file->file);
    if (result != 0)
    {
        fprintf(stderr, "Unable to close the filesystem file.\n");
        return 0;
    }

    free(g_fs_file);
    g_fs_file = NULL;

    return 1;
}

char* fs_search(const char* key)
{
    int                 result;
    struct s_fs_node    node;
    char*               value = NULL;

    if (g_fs_file == NULL)
    {
        fprintf(stderr, "A file must be opened before calling fs_search().\n");
        return 0;
    }
    
    result = fseek(g_fs_file->file, g_fs_file->header.root, SEEK_SET);
    if (result < 0)
    {
        fprintf(stderr, "Unable to seek.\n");
        return 0;
    }

    result = fread(&node.type, sizeof(node.type), 1, g_fs_file->file);
    if (result != 1)
    {
        fprintf(stderr, "Unable to read.\n");
        return 0;
    }

    // TODO: Unhack here
    result = fread(&node.index, sizeof(node.index), 1, g_fs_file->file);
    if (result != 1)
    {
        fprintf(stderr, "Unable to read.\n");
        return 0;
    }

    for (size_t i = 0; i < node.index.nb_keys; ++i)
    {
        if ((node.index.keys[i].size == strlen(key)) &&
            (strncmp(key, node.index.keys[i].data, node.index.keys[i].size) == 0))
        {
            struct s_fs_node data;

            fseek(g_fs_file->file, node.index.children[i], SEEK_SET);
            fread(&data.type, sizeof(data.type), 1, g_fs_file->file);
            fread(&data.block, sizeof(data.block), 1, g_fs_file->file);
            value = malloc(1 + data.block.size);
            fread(value, data.block.size, 1, g_fs_file->file);
            value[data.block.size] = 0;
            break;
        }
    }

    return value;
}

int fs_add(const char* key, const char* data)
{
    int                 result;
    struct s_fs_node    node;

    if (g_fs_file == NULL)
    {
        fprintf(stderr, "A file must be opened before calling fs_add().\n");
        return 0;
    }
    
    result = fseek(g_fs_file->file, g_fs_file->header.root, SEEK_SET);
    if (result < 0)
    {
        fprintf(stderr, "Unable to seek.\n");
        return 0;
    }

    result = fread(&node.type, sizeof(node.type), 1, g_fs_file->file);
    if (result != 1)
    {
        fprintf(stderr, "Unable to read.\n");
        return 0;
    }

    // TODO: Unhack here
    result = fread(&node.index, sizeof(node.index), 1, g_fs_file->file);
    if (result != 1)
    {
        fprintf(stderr, "Unable to read.\n");
        return 0;
    }

    if (node.index.nb_keys < BTREE_ORDER - 1)
    {
        struct s_fs_node    data_node;

        ++node.index.nb_keys;
        node.index.keys[node.index.nb_keys - 1].size = strlen(key);
        strncpy(node.index.keys[node.index.nb_keys - 1].data, key, node.index.keys[node.index.nb_keys - 1].size * sizeof(char));
        ++node.index.nb_children;
        fseek(g_fs_file->file, 0, SEEK_END);
        node.index.children[node.index.nb_children - 1] = ftell(g_fs_file->file);
        fseek(g_fs_file->file, g_fs_file->header.root + sizeof(node.type), SEEK_SET);
        fwrite(&node.index, sizeof(node.index), 1, g_fs_file->file);
        
        fseek(g_fs_file->file, 0, SEEK_END);
        data_node.type = NODE_BLOCK;
        data_node.block.size = strlen(data);
        fwrite(&data_node.type, sizeof(data_node.type), 1, g_fs_file->file);
        fwrite(&data_node.block, sizeof(data_node.block), 1, g_fs_file->file);
        fwrite(data, data_node.block.size, 1, g_fs_file->file);
    }

    fflush(g_fs_file->file);

    return 1;
}

int fs_delete(const char* key)
{
    return 0;
}

