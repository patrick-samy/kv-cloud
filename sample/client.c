#include <stdlib.h>
#include <stdio.h>

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
        int len = getline(&line, &line_length, stdin);

        printf("command %s", line);

        if (len <= 1)
            break;
    }
    while (1);

    kvcloud_disconnect(socket);
    printf("Disconnected.\n");

    return 0;
}
