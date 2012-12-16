#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>

#include "server.h"
#include "protocol.h"
#include "filesystem.h"

int main(int argc, char** argv)
{
    int                     result = 0;
    unsigned short          port = DEFAULT_PORT;
    int                     option_index = 1;
    static struct option    long_options[] =
    {
        {"daemonize", no_argument, 0, 'd'},
        {"port", required_argument, 0, 'p'},
        {0, 0, 0, 0}
    };

    result = getopt_long(argc, argv, "dp:f", long_options, &option_index);
    while (result != -1)
    {
        
        if (result == '?')
            exit(1);

        switch (result)
        {
            case 'd':
                printf("daemonize.\n");
                break;

            case 'p':
                port = atoi(argv[option_index + 1]);
                break;

            case 'f':
                fs_init("kvcloud.fs");
                break;

            default:
                fprintf(stderr, "Unhandled getopt return code: %d.\n", result);
        }
        
        result = getopt_long(argc, argv, "d", long_options, &option_index);
    }

    fs_open("kvcloud.fs");
    printf("Starting server on port %d.\n", port);
    result = start_server(port);

    return result;
}
