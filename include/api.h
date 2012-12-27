#ifndef API_H_
# define API_H_

# include <stdint.h>

// Connection
int kvcloud_connect(const char* address, uint16_t port);
void kvcloud_disconnect();

// Storage
size_t kvcloud_get(const char* key, void** value);
void kvcloud_set(const char* key, const void* value, size_t nb_bytes);
void kvcloud_delete(const char* key);

#endif /* !API_H_ */
