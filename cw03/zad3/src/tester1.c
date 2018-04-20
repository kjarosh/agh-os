#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

void alloc_mem(int times, size_t alloc_size) {
	if (times == 0) return;
	
	printf("Allocating...\n");
	
	void *mem = malloc(alloc_size);
	memset(mem, 0, alloc_size);
	
	sleep(1);
	
	alloc_mem(times - 1, alloc_size);
	
	free(mem);
}

int main(int argc, char **argv) {
	size_t maxmem = 100 * 1024 * 1024; // 100MB
	size_t alloc_size = 2 * 1024 * 1024; // 2MB
			
	int times = maxmem / alloc_size;
	
	alloc_mem(times, alloc_size);
	
	printf("Done\n");
	
	return 0;
}
