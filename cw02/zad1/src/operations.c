#include "operations.h"

#include <string.h>
#include "file_desc.h"

int op_generate(char *filename, size_t record_count, size_t record_size) {
	unsigned char buffer[record_size];
	buffer[record_size - 1] = '\n';
	
	file_desc file = fd_openw(SYS, filename);
	file_desc rnd = fd_openr(SYS, "/dev/urandom");
	if (file.error != 0) {
		perror("Error opening file");
		return 1;
	}
	
	for (size_t i = 0; i < record_count; ++i) {
		fd_read(rnd, buffer, record_size - 1);
		
		for (size_t j = 0; j < record_size - 1; ++j) {
			buffer[j] = buffer[j] % ('z' - 'a' + 1) + 'a';
		}
		
		fd_write(file, buffer, record_size);
	}
	
	return 0;
}

int op_copy(char *filename, char *filename2, size_t record_count, size_t record_size, fmode_t mode) {
	unsigned char buffer[record_size];
	
	file_desc file1 = fd_openr(mode, filename);
	if (file1.error != 0) {
		perror("Error opening file 1");
		return 1;
	}
	
	file_desc file2 = fd_openw(mode, filename2);
	if (file2.error != 0) {
		perror("Error opening file 2");
		return 1;
	}
	
	for (size_t i = 0; i < record_count; ++i) {
		fd_read(file1, buffer, record_size);
		fd_write(file2, buffer, record_size);
	}
	
	return 0;
}

int op_sort(char *filename, size_t record_count, size_t record_size, fmode_t mode) {
	file_desc file = fd_openrw(mode, filename);
	if (file.error != 0) {
		perror("Error opening file");
		return 1;
	}
	
	unsigned char next[record_size];
	unsigned char inserted[record_size];
	
	// index of the last, sorted element
	size_t sorted_end = 0;
	
	while (sorted_end + 1 < record_count) {
		fd_seek_row(file, sorted_end + 1, record_size);
		
		fd_read(file, inserted, record_size);
		size_t next_pos = sorted_end + 1;
		
		fd_seek_row_rel(file, -1, record_size);
		
		// cursor at the start of the row to compare to
		do {
			fd_seek_row_rel(file, -1, record_size);
			
			fd_read(file, next, record_size);
			
			// if (strcmp((char *) next, (char *) inserted) > 0) {
			if (next[0] > inserted[0]) {
				fd_write(file, next, record_size);
				fd_seek_row_rel(file, -2, record_size);
				next_pos--;
			} else {
				break;
			}
		} while (next_pos > 0);
		
		fd_write(file, inserted, record_size);
		
		++sorted_end;
	}
	
	return 0;
}
