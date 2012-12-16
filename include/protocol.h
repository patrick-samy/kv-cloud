#ifndef PROTOCOL_H_
# define PROTOCOL_H_

# include <stdint.h>

# define DEFAULT_PORT           60000

# define COMMAND_DISCONNECT 0x01
# define COMMAND_GET        0x02
# define COMMAND_SET        0x04
# define COMMAND_DELETE     0x08
# define FLAG_TIMEOUT       0x10

struct s_request_header
{
    uint8_t command;
    union
    {
        struct
        {
            uint32_t key_length;
            uint32_t value_length;
        } k;
        struct
        {
            uint32_t value_length;
        } kv;
    };
} __attribute__((packed));

struct s_response_header
{
    uint32_t value_length;
};

#endif /* !PROTOCOL_H_ */

