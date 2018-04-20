#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

size_t compute(size_t i, size_t j, size_t k) {
	size_t ret = 0;
	
	if (i * i + j * j == k * k) {
		++ret;
	}
	
	if (k * k + i * i == j * j) {
		++ret;
	}
	
	if (j * j + k * k == i * i) {
		++ret;
	}
	
	return ret;
}

int main(int argc, char **argv) {
	size_t count = 1024 * 1024;
	size_t ret = 0;
	size_t curr = 0;
	
	// it's about 10^18 calls for `compute'
	for (size_t i = 0; i < count; ++i)
		for (size_t j = 0; j < count; ++j) {
			for (size_t k = 0; k < count; ++k) {
				ret += compute(i, j, k);
			}
			
			++curr;
			
			if (curr % 70 == 0) {
				printf("Computing...\n");
			}
		}
	
	printf("Got: %u\n", (unsigned int) ret);
}
