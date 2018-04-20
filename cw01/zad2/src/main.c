#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "stopwatch.h"

#ifndef DYNAMIC_LIB
#include <challoc.h>
#else
#include <dlfcn.h>
#include <challoc_struc.h>

void *libhandle;

blk_table (*challoc)(size_t, size_t);
void (*chfree)(blk_table);
int (*challoc_blk)(blk_table, size_t);
void (*chfree_blk)(blk_table, size_t);
size_t (*chfind)(blk_table, size_t, long long *);
void (*s_chrndfill)(size_t);
void (*s_chvoid)(size_t);
char *(*s_chgetblk)(size_t);
size_t (*s_chfind)(size_t, long long *);
#endif

void libuload() {
#ifdef DYNAMIC_LIB
	dlclose(libhandle);
#endif
}

void libload() {
#ifdef DYNAMIC_LIB
	libhandle = dlopen ("./libchalloc.so", RTLD_LAZY);
	if (!libhandle) {
		fprintf (stderr, "Error opening library: %s\n", dlerror());
		exit(1);
	}
	dlerror();

	challoc = dlsym(libhandle, "challoc");
	chfree = dlsym(libhandle, "chfree");
	challoc_blk = dlsym(libhandle, "challoc_blk");
	chfree_blk = dlsym(libhandle, "chfree_blk");
	chfind = dlsym(libhandle, "chfind");
	s_chrndfill = dlsym(libhandle, "s_chrndfill");
	s_chvoid = dlsym(libhandle, "s_chvoid");
	s_chgetblk = dlsym(libhandle, "s_chgetblk");
	s_chfind = dlsym(libhandle, "s_chfind");

	char *error = dlerror();
	if (error != NULL) {
		fprintf(stderr, "Error loading library components: %s\n", error);
		libuload();
		exit(1);
	}
#endif
}

enum alloc_t {
	DYNAMIC, STATIC
};

void print_help(char *prog) {
	printf("Usage:\n");
	printf("\t%s <alloc type> <operations>\n", prog);
	printf("\n");
	printf("alloc type: either `static' or `dynamic'\n");
	printf("ops: list of operations\n");
	printf("\tcreate_table <blk_count> <blk_size> <times>\n");
	printf("\t\tCreate table with `blk_count' blocks, each of size `blk_size'. ");
	printf("Do it `times' times.\n");
	printf("\tsearch_element <blk_no> <times>\n");
	printf("\t\tSearch for element with the closest ASCII sum to the `lbk_no's. ");
	printf("Do it `times' times.\n");
	printf("\tremove <times>\n");
	printf("\t\tRemove block `times' times.\n");
	printf("\tadd <times>\n");
	printf("\t\tAdd block `times' times.\n");
	printf("\tremove_and_add <times>\n");
	printf("\t\tRemove and add block `times' times.\n");
}

blk_table table = { 0, 0, NULL };
enum alloc_t alloc;

int check_times(size_t times) {
	if (alloc == DYNAMIC) {
		if (times > table.blk_count) {
			fprintf(stderr, "Error: cannot remove more blocks than allocated.\n");
			return -1;
		}
	} else {
		if (times > CHALLOC_STATIC_BLK_COUNT) {
			fprintf(stderr, "Error: cannot remove more blocks than allocated.\n");
			return -1;
		}
	}
	
	return 0;
}

void op_remove_and_add(watch_t *watch, size_t times) {
	if (check_times(times) != 0) return;
	
	stopwatch(watch);
	
	for (int i = 0; i < times; ++i) {
		if (alloc == DYNAMIC) {
			challoc_blk(table, i);
			chfree_blk(table, i);
		} else {
			s_chrndfill(i);
			s_chvoid(i);
		}
	}
	
	stopwatch(watch);
}

void op_add(watch_t *watch, size_t times) {
	if (check_times(times) != 0) return;
	
	stopwatch(watch);
	
	for (int i = 0; i < times; ++i) {
		if (alloc == DYNAMIC) {
			challoc_blk(table, i);
		} else {
			s_chrndfill(i);
		}
	}
	
	stopwatch(watch);
}

void op_remove(watch_t *watch, size_t times) {
	if (check_times(times) != 0) return;
	
	stopwatch(watch);
	
	for (int i = 0; i < times; ++i) {
		if (alloc == DYNAMIC) {
			chfree_blk(table, i);
		} else {
			s_chvoid(i);
		}
	}
	
	stopwatch(watch);
}

void op_search_element(watch_t *watch, size_t times, size_t blk_no) {
	stopwatch(watch);
	
	for (int i = 0; i < times; ++i) {
		if (alloc == DYNAMIC) {
			chfind(table, blk_no, NULL);
		} else {
			s_chfind(blk_no, NULL);
		}
	}
	
	stopwatch(watch);
}

void op_create_table(watch_t *watch, size_t times, size_t blk_count, size_t blk_size) {
	stopwatch(watch);
	
	for (int i = 0; i < times; ++i) {
		chfree(table);
		table = challoc(blk_count, blk_size);
	}
	
	stopwatch(watch);
}

int main(int argc, char **argv) {
	srand(time(NULL));
	
	libload();
	
	if (argc < 2) {
		print_help(argv[0]);
		return 1;
	}
	
	if (strcmp(argv[1], "static") == 0) {
		alloc = STATIC;
	} else if (strcmp(argv[1], "dynamic") == 0) {
		alloc = DYNAMIC;
	} else {
		printf("Unrecognized allocation type: %s\n", argv[3]);
		return 2;
	}
	
	fprintf(stdout, "| %20s | %10s | %10s | %10s |\n", "operation", "real", "user", "sys");
	
	int curr = 2;
	while (curr < argc) {
		char *name = argv[curr++];
		watch_t watch;
		
		if (strcmp(name, "create_table") == 0) {
			size_t blk_count = atoi(argv[curr++]);
			size_t blk_size = atoi(argv[curr++]);
			size_t times = atoi(argv[curr++]);
			
			if (alloc == STATIC) {
				fprintf(stderr, "Operation not supported in static mode\n");
				continue;
			}
			
			op_create_table(&watch, times, blk_count, blk_size);
		} else if (strcmp(name, "search_element") == 0) {
			size_t blk_no = atoi(argv[curr++]);
			size_t times = atoi(argv[curr++]);
			
			op_search_element(&watch, times, blk_no);
		} else if (strcmp(name, "remove") == 0) {
			size_t times = atoi(argv[curr++]);
			
			op_remove(&watch, times);
		} else if (strcmp(name, "add") == 0) {
			size_t times = atoi(argv[curr++]);
			
			op_add(&watch, times);
		} else if (strcmp(name, "remove_and_add") == 0) {
			size_t times = atoi(argv[curr++]);
			
			op_remove_and_add(&watch, times);
		} else {
			fprintf(stderr, "Unrecognized operation: %s\n", name);
			continue;
		}
		
		fprintf(stdout, "| %20s | %10lf | %10lf | %10lf |\n", name, watch.rsec, watch.usec,
				watch.ssec);
	}
	
	libuload();
	return 0;
}

/*
 void run() {
 watch_t watch;
 stopwatch(&watch);
 
 for (int i = 0; i < 1024 * 100 / 2; ++i) {
 chfree(challoc(16 * 1024));
 }
 
 stopwatch(&watch);
 
 fprintf(stdout, "| %10s | %10s | %10s |\n", "real", "user", "sys");
 fprintf(stdout, "| %10lf | %10lf | %10lf |\n", watch.rsec, watch.usec, watch.ssec);
 }
 */
