#ifndef STORAGE_H_
# define STORAGE_H_

char* storage_get(char* key);
void storage_set(char* key, char* value);
void storage_delete(char* key);

#endif /* !STORAGE_H_ */
