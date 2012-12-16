#ifndef API_H_
# define API_H_

# include <stdint.h>

// Connection
int kvcloud_connect(const char* address, uint16_t port);
void kvcloud_disconnect();

// Storage
char* kvcloud_get(const char* key);
void kvcloud_set(const char* key, const char* value);
void kvcloud_delete(const char* key);

#endif /* !API_H_ */
