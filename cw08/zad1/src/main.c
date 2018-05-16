#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "filter.h"
#include "pgm.h"

static void print_help(char *program) {
	printf("Usage:\n");
	printf("\t %s gen <size> <filter>\n", program);
	printf("or:\n");
	printf("\t %s <threads> <input> <filter> <output>\n", program);
}

int main(int argc, char **argv) {
	if (argc < 4) {
		print_help(argv[0]);
		return 1;
	}
	
	if (strcmp(argv[1], "gen") == 0) {
		if (argc != 4) {
			print_help(argv[0]);
			return 1;
		}
		
		filter_t *f = create_filter(atoi(argv[2]));
		random_filter(f);
		if (write_filter(f, argv[3]) != 0) {
			perror("Cannot write filter");
			return 1;
		}
		
		return 0;
	}
	
	if (argc != 5) {
		print_help(argv[0]);
		return 1;
	}
	
	int threads = atoi(argv[1]);
	char *input = argv[2];
	char *filter = argv[3];
	char *output = argv[4];
	
	filter_t *f = NULL;
	if (read_filter(&f, filter) != 0) {
		perror("Cannot read filter");
		goto error_exit;
	}
	
	pgm_image *in_image = NULL;
	if (pgm_load(&in_image, input) != 0) {
		perror("Cannot read input file");
		goto error_exit;
	}
	
	pgm_image *out_image = pgm_create(in_image->width, in_image->height);
	if (out_image == NULL) {
		perror("Cannot create image");
		goto error_exit;
	}
	
	for (int x = 0; x < in_image->width; ++x) {
		for (int y = 0; y < in_image->height; ++y) {
			pgm_set_pixel(out_image, x, y, apply_filter(f, in_image, x, y, FILTER_DISCARD));
		}
	}
	
	if (pgm_save(out_image, output) != 0) {
		perror("Cannot save file");
		goto error_exit;
	}
	
	// ------------------------------------
	error_exit: //
	if (f != NULL) destroy_filter(f);
	if (in_image != NULL) pgm_destroy(in_image);
	return 0;
}
