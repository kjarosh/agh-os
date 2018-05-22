#ifndef PGM_H_
#define PGM_H_

#include <stdlib.h>

#define pix_val(p) ((int)(unsigned char)(p))

typedef unsigned char pix_t;

typedef struct {
	size_t width;
	size_t height;
	pix_t data[0];
} pgm_image;

pgm_image *pgm_create(size_t width, size_t height);
void pgm_destroy(pgm_image *img);

pix_t pgm_get_pixel(pgm_image *img, int x, int y);
void pgm_set_pixel(pgm_image *img, int x, int y, pix_t pix);

int pgm_load(pgm_image **img, const char *filename);
int pgm_save(pgm_image *img, const char *filename);

#endif
