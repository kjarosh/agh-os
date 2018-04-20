#ifndef FILE_DESC_H_
#define FILE_DESC_H_

#include <stdio.h>

typedef enum mode {
	SYS = 0, LIB
} fmode_t;

typedef struct file_desc {
	int error;
	fmode_t mode;
	
	union {
		int unix_desc;
		FILE *file;
	};
} file_desc;

file_desc fd_openr(fmode_t mode, char *filename);
file_desc fd_openw(fmode_t mode, char *filename);
file_desc fd_openrw(fmode_t mode, char *filename);

void fd_read(file_desc fd, unsigned char *buffer, size_t record_size);
void fd_write(file_desc fd, unsigned char *buffer, size_t record_size);

void fd_seek_row(file_desc fd, size_t row_no, size_t record_size);
void fd_seek_row_rel(file_desc fd, int row_rel, size_t record_size);

void fd_close(file_desc fd);

#endif
