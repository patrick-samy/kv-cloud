#ifndef API_H_
# define API_H_

# include <stdint.h>

// Connection
int kvcloud_connect(const char* address, uint16_t port);
void kvcloud_disconnect(int client_socket);

// Storage
char* kvcloud_get(char* key);
void kvcloud_set(char* key, char* value);
void kvcloud_delete(char* key);

#endif /* !API_H_ */

