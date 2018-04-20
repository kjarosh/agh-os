#include "file_desc.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

file_desc fd_openr(fmode_t mode, char *filename) {
	file_desc ret;
	ret.error = 0;
	ret.mode = mode;
	
	if (mode == SYS) {
		ret.unix_desc = open(filename, O_RDONLY);
		
		if (ret.unix_desc == -1) {
			ret.error = -1;
		}
	} else if (mode == LIB) {
		ret.file = fopen(filename, "r");
		
		if (ret.file == NULL) {
			ret.error = -1;
		}
	}
	
	return ret;
}

file_desc fd_openw(fmode_t mode, char *filename) {
	file_desc ret;
	ret.error = 0;
	ret.mode = mode;
	
	if (mode == SYS) {
		ret.unix_desc = open(filename, O_WRONLY | O_CREAT, S_IRWXU);
		
		if (ret.unix_desc == -1) {
			ret.error = -1;
		}
	} else if (mode == LIB) {
		ret.file = fopen(filename, "w");
		
		if (ret.file == NULL) {
			ret.error = -1;
		}
	}
	
	return ret;
}

file_desc fd_openrw(fmode_t mode, char *filename) {
	file_desc ret;
	ret.error = 0;
	ret.mode = mode;
	
	if (mode == SYS) {
		ret.unix_desc = open(filename, O_RDWR);
		
		if (ret.unix_desc == -1) {
			ret.error = -1;
		}
	} else if (mode == LIB) {
		ret.file = fopen(filename, "r+");
		
		if (ret.file == NULL) {
			ret.error = -1;
		}
	}
	
	return ret;
}

void fd_read(file_desc fd, unsigned char *buffer, size_t record_size) {
	if (fd.mode == SYS) {
		ssize_t r = read(fd.unix_desc, buffer, record_size);
		
		if (r == -1) {
			perror("Error reading file");
			fd.error = -1;
		}
	} else if (fd.mode == LIB) {
		fread(buffer, record_size, 1, fd.file);
	}
}

void fd_write(file_desc fd, unsigned char *buffer, size_t record_size) {
	if (fd.mode == SYS) {
		ssize_t w = write(fd.unix_desc, buffer, record_size);
		
		if (w == -1) {
			perror("Error writing to file");
			fd.error = -1;
		}
	} else if (fd.mode == LIB) {
		fwrite(buffer, record_size, 1, fd.file);
	}
}

void fd_close(file_desc fd) {
	if (fd.mode == SYS) {
		close(fd.unix_desc);
	} else if (fd.mode == LIB) {
		fclose(fd.file);
	}
}

void fd_seek_row(file_desc fd, size_t row_no, size_t record_size) {
	if (fd.mode == SYS) {
		off_t s = lseek(fd.unix_desc, row_no * record_size, SEEK_SET);
		
		if (s == (off_t) -1) {
			perror("Error seeking");
			fd.error = -1;
		}
	} else if (fd.mode == LIB) {
		int s = fseek(fd.file, row_no * record_size, SEEK_SET);
		
		if (s == -1) {
			perror("Error seeking");
			fd.error = -1;
		}
	}
}

void fd_seek_row_rel(file_desc fd, int row_rel, size_t record_size) {
	if (fd.mode == SYS) {
		off_t s = lseek(fd.unix_desc, row_rel * record_size, SEEK_CUR);
		
		if (s == (off_t) -1) {
			perror("Error seeking");
			fd.error = -1;
		}
	} else if (fd.mode == LIB) {
		int s = fseek(fd.file, row_rel * record_size, SEEK_CUR);
		
		if (s == -1) {
			perror("Error seeking");
			fd.error = -1;
		}
	}
}
