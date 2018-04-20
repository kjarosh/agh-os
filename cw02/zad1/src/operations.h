#ifndef OPERATIONS_H_
#define OPERATIONS_H_

#include <stdlib.h>
#include "file_desc.h"

int op_generate(char *filename, size_t record_count, size_t record_size);
int op_sort(char *filename, size_t record_count, size_t record_size, fmode_t mode);
int op_copy(char *filename, char *filename2, size_t record_count, size_t record_size,
		fmode_t mode);

#endif
