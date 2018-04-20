#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <limits.h>

void list_file(char *filename, cond_t cond, struct date date) {
	struct stat st;
	if (lstat(filename, &st) != 0) {
		perror("lstat failed");
		return;
	}
	
	mode_t mode = st.st_mode;
	if (S_ISDIR(mode)) {
		list_files(filename, cond, date);
		return;
	}
	
	if (S_ISREG(mode)) {
		if (check_time(st.st_mtime, cond, date) != 0) {
#ifdef DEBUG
			printf("[DEBUG] %20s ", "!COND:");
#else
			return;
#endif
		}
		
		print_access(&st);
		print_last_mod(&st);
		print_size(&st);
		printf("%s\n", filename);
		
		return;
	}
	
#ifdef DEBUG
	printf("[DEBUG] %20s %s\n", "!REG && !DIR:", filename);
#endif
}

void list_files(char *dirname, cond_t cond, struct date date) {
	DIR *dir = opendir(dirname);
	if (dir == NULL) {
		perror("Cannot open directory");
		return;
	}
	
	struct dirent *dirent;
	while ((dirent = readdir(dir)) != NULL) {
		if (strcmp(dirent->d_name, ".") == 0 || strcmp(dirent->d_name, "..") == 0) {
			continue;
		}
		
		char path[PATH_MAX] = { 0 };
		strcat(path, dirname);
		
		if (dirname[strlen(dirname) - 1] != '/') {
			strcat(path, "/");
		}
		
		strcat(path, dirent->d_name);
		
#ifdef CLOSE_DIRS
		long int loc = telldir(dir);
		closedir(dir);
#endif
		
		list_file(path, cond, date);
		
#ifdef CLOSE_DIRS
		dir = opendir(dirname);
		if (dir == NULL) {
			perror("Cannot open directory");
			return;
		}
		
		seekdir(dir, loc);
#endif
	}
	
	closedir(dir);
}
