#ifndef BUFFER_H_
#define BUFFER_H_

int buffer_init(int size);
int buffer_empty(void);
int buffer_full(void);
int buffer_push(char *string);
int buffer_pop(char **stringp);

#endif
