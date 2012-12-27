#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "api.h"

int main(int argc, char** argv)
{
    int     socket = 0;
    size_t  line_length = 4096;
    char*   line = malloc(line_length * sizeof(char));

    if (argc != 3)
    {
        fprintf(stderr, "Usage: ./client ip port\n");
        return 1;
    }

    socket = kvcloud_connect(argv[1], atoi(argv[2]));
    if (socket < 0)
        return 2;

    printf("Connected to %s on port %s\n", argv[1], argv[2]);
    do
    {
        int         len = getline(&line, &line_length, stdin);
        const char* delimiter = " \n";
        char*       command;
        
        if (len <= 1)
            break;

        command = strtok(line, delimiter);
        if (command == NULL)
        {
            fprintf(stderr, "Unable to parse command.\n");
            continue;
        }
        else if (strcmp(command, "get") == 0)
        {
            char* key;
            char* value;

            key = strtok(NULL, delimiter);
            if (key == NULL)
            {
                fprintf(stderr, "Unable to parse key.\n");
                continue;
            }

            len = kvcloud_get(key, (void**) &value);
            printf("%s => %.*s\n", key, len, value);
        }
        else if (strcmp(command, "set") == 0)
        {
            char* key;
            char* value;

            key = strtok(NULL, delimiter);
            value = strtok(NULL, delimiter);
            if ((key == NULL) || (value == NULL))
            {
                fprintf(stderr, "Unable to parse key/value.\n");
                continue;
            }

            kvcloud_set(key, value, strlen(value));
        }
        else if (strcmp(command, "delete") == 0)
        {
            char* key;

            key = strtok(NULL, delimiter);
            if (key == NULL)
            {
                fprintf(stderr, "Unable to parse key.\n");
                continue;
            }

            kvcloud_delete(key);
        }
        else
        {
            fprintf(stderr, "Unknown command %s\n", command);
            continue;
        }
    }
    while (1);

    kvcloud_disconnect(socket);
    printf("Disconnected.\n");

    return 0;
}
