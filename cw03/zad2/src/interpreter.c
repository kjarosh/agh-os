#include "interpreter.h"

#include <ctype.h>
#include <sys/wait.h>
#include <sys/types.h>

#include "command_runner.h"

#define BUFFER_SIZE (8*1024)
#define ARGS_MAX (256)

static int is_eol(int ch) {
	return ch == '\n' || ch == 0;
}

static int interpret_line(char buf[BUFFER_SIZE], int *argc, char *args[ARGS_MAX]) {
	*argc = 0;
	for (int pos = 0; pos <= BUFFER_SIZE;) {
		if (is_eol(buf[pos])) {
			args[*argc] = NULL;
			return 0;
		}
		
		// parse argumet between ""
		if (buf[pos] == '"') {
			args[(*argc)++] = &buf[pos + 1];
			
			do {
				++pos;
				
				if (is_eol(buf[pos])) {
					fprintf(stderr, "Expected closing `\"'\n");
					return -1;
				}
			} while (buf[pos] != '"');
			
			buf[pos++] = 0;
			
			if (is_eol(buf[pos])) {
				args[*argc] = NULL;
				return 0;
			} else if (!isspace(buf[pos])) {
				fprintf(stderr, "Expected space after closing `\"'\n");
				return -1;
			}
		}
		
		// parse single word argument
		if (!isspace(buf[pos])) {
			args[(*argc)++] = &buf[pos];
			
			do {
				++pos;
				
				if (is_eol(buf[pos])) {
					break;
				}
			} while (!isspace(buf[pos]));
			
			buf[pos] = 0;
		}
		
		do {
			++pos;
			
			if (is_eol(buf[pos])) {
				break;
			}
		} while (isspace(buf[pos]));
	}
	
	args[*argc] = NULL;
	return 0;
}

int interpret_file(FILE *fp) {
	char buf[BUFFER_SIZE];
	
	int line = 1;
	while (1) {
		char *ret = fgets(buf, BUFFER_SIZE, fp);
		if (ret == NULL) {
			// end of file
			break;
		}
		
		char *args[ARGS_MAX];
		int argc;
		if (interpret_line(buf, &argc, args) < 0) {
			fprintf(stderr, "Error at line %d\n", line);
			return -1;
		}
		
		if (argc == 0) {
#ifdef DEBUG
			printf("[debug] No command\n");
#endif
		} else {
#ifdef DEBUG
			printf("[debug] Running:");
			for (int i = 0; i < argc; ++i) {
				printf(" `%s'", args[i]);
			}
			printf("\n");
#endif
			
			int status;
			int run = run_command(argc, args, &status);
			
			if (run != 0) {
				fprintf(stderr, "Running command failed\n");
				break;
			}
			
			if (WEXITSTATUS(status) != 0) {
				fprintf(stderr, "Process `%s' exited with status: %d\n", args[0],
						WEXITSTATUS(status));
				fprintf(stderr, "Line: %d\n", line);
				return -1;
			} else {
#ifdef DEBUG
				printf("[debug] Process exited normally\n");
#endif
			}
		}
		
		++line;
	}
	
	return 0;
}
