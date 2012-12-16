#include <unistd.h>
#include <stdio.h>

#include "storage.h"

char* storage_get(char* key)
{
    return "42";
}

void storage_set(char* key, char* value)
{
    printf("set %s => %s\n", key, value);
}

void storage_delete(char* key)
{
    printf("delete %s\n", key);
}

