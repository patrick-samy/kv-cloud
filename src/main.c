#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>

#include "server.h"

int main(int argc, char** argv)
{
    int                     result = 0;
    int                     option_index = 1;
    static struct option    long_options[] =
    {
        {"daemonize", no_argument, 0, 'd'},
        {0, 0, 0, 0}
    };

    result = getopt_long(argc, argv, "d", long_options, &option_index);
    while (result != -1)
    {
        
        if (result == '?')
            exit(1);

        switch (result)
        {
            case 'd':
                printf("daemonize.\n");
                break;

            default:
                fprintf(stderr, "Unhandled getopt return code: %d.\n", result);
        }
        
        result = getopt_long(argc, argv, "d", long_options, &option_index);
    }

    result = start_server();

    return result;
}
