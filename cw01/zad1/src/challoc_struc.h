#ifndef CHALLOC_STRUC_H_
#define CHALLOC_STRUC_H_

#include <stdlib.h>

#ifndef CHALLOC_STATIC_BLK_SIZE
/**
 * To makro definiuje rozmiar w bajtach pojedynczego bloku zaalokowanego statycznie.
 * Wartość tę można zmienić podczas kompilacji.
 */
#define CHALLOC_STATIC_BLK_SIZE 8192
#endif

#ifndef CHALLOC_STATIC_BLK_COUNT
/**
 * To makro definiuje liczbę bloków zaalokowanych statycznie.
 * Wartość tę można zmienić podczas kompilacji.
 */
#define CHALLOC_STATIC_BLK_COUNT 4096
#endif

/**
 * Typ dynamicznego bloku znaków.
 */
typedef struct blk_t {
	char content[0];
} blk_t;

typedef blk_t *blk_ptr;

/**
 * Typ dynamicznej tablicy bloków.
 */
typedef struct blk_table {
	size_t blk_count;
	size_t blk_size;
	blk_ptr *blks;
} blk_table;

/**
 * Typ statycznego bloku znaków.
 */
typedef struct sblk_t {
	char content[CHALLOC_STATIC_BLK_SIZE];
} sblk_t;

/**
 * Typ statycznej tablicy bloków.
 */
typedef struct sblk_table {
	sblk_t blks[CHALLOC_STATIC_BLK_COUNT];
} sblk_table;

#endif
