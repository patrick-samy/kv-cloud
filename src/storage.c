#include <unistd.h>
#include <stdio.h>

#include "storage.h"
#include "filesystem.h"

char* storage_get(char* key)
{
    return fs_search(key);
}

void storage_set(char* key, char* value)
{
    fs_add(key, value);
}

void storage_delete(char* key)
{
    fs_delete(key);
}

