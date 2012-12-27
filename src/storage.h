#ifndef STORAGE_H_
# define STORAGE_H_

size_t storage_get(char* key, void** value);
void storage_set(char* key, void* value, size_t nb_bytes);
void storage_delete(char* key);

#endif /* !STORAGE_H_ */
