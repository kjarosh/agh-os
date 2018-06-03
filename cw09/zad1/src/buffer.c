#include "buffer.h"

#include <stdlib.h>

char **buffer;
int buffer_size;

int buffer_start;
int buffer_length;

char **__buffer_at(int index) {
	return &buffer[index % buffer_size];
}

int buffer_init(int size) {
	buffer = calloc(sizeof(char *), size);
	if (buffer == NULL) {
		return -1;
	}
	
	buffer_size = size;
	buffer_start = 0;
	buffer_length = 0;
	
	return 0;
}

int buffer_empty(void) {
	return buffer_length == 0;
}

int buffer_full(void) {
	return buffer_length == buffer_size;
}

int buffer_push(char *string) {
	if (buffer_full()) {
		return -1;
	}
	
	*__buffer_at(buffer_start + buffer_length) = string;
	++buffer_length;
	
	return 0;
}

int buffer_pop(char **stringp) {
	if (buffer_empty()) {
		return -1;
	}
	
	*stringp = *__buffer_at(buffer_start);
	buffer_start = (buffer_start + 1) % buffer_size;
	--buffer_length;
	
	return 0;
}
