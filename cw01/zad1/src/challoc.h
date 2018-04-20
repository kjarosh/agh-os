#ifndef CHALLOC_H_
#define CHALLOC_H_

#include "challoc_struc.h"

/**
 * Globalna tablica bloków.
 */
extern sblk_table s_chtable;

/**
 * Alokuje tablicę wskaźników na bloki. Domyślnie wszystkie bloki są niezaalokowane,
 * tzn ich adres jest równy 0.
 *
 * Jeśli alokacja się nie powiedzie to ustawia adres tablicy na NULL oraz ilość
 * zaalokowanych bloków na 0.
 * 
 * Alokowane bloki będą mieć wielkość `blk_size'.
 */
blk_table challoc(size_t blk_count, size_t blk_size);

/**
 * Zwalnia pamięć po tablicy bloków.
 */
void chfree(blk_table tbl);

/**
 * Alokuje blok w danej tablicy pod indeksem `blk_no'. Jeśli pod tym
 * indeksem znajduje się inny blok, jest on zwalniany z pamięci.
 *
 * Zwraca 0 w przypadku powodzenia, inną wartośc w przypadku błędu.
 */
int challoc_blk(blk_table tbl, size_t blk_no);

/**
 * Zwalnia pamięć po bloku o indeksie `blk_no'.
 */
void chfree_blk(blk_table tbl, size_t blk_no);

/**
 * Zwraca indeks bloku, którego suma znaków jest najbliższa sumie
 * znaków bloku o indeksie `blk_no'.
 *
 * Jeśli jest takich bloków więcej to funkcja może zwrócić dowolny.
 *
 * Jeśli blok taki nie istnieje, funkcja zwraca `blk_no'.
 *
 * Jeśli blok o indeksie `blk_no' nie jest zaalokowany to funkcja zwraca `blk_no'.
 *
 * Jeśli `difference' nie jest równe NULL to różnica sum jest tam umieszczona.
 */
size_t chfind(blk_table tbl, size_t blk_no, long long *difference);

/**
 * Wypełnia statyczny blok o indeksie `blk_no' losowymi danymi.
 */
void s_chrndfill(size_t blk_no);

/**
 * Zeruje statyczny blok o indeksie `blk_id'.
 */
void s_chvoid(size_t blk_no);

/**
 * Zwraca wskaźnik na blok o indeksie `blk_no' ze statycznej tablicy
 * bloków.
 */
char *s_chgetblk(size_t blk_no);

/**
 * Przeszukuje statyczną tablicę bloków w poszukiwaniu bloku o sumie znaków
 * najbliższej sumie znaków bloku o indeksie `blk_no'.
 *
 * Jeśli jest takich bloków więcej to funkcja może zwrócić dowolny.
 *
 * Jeśli `difference' nie jest równe NULL to różnica sum jest tam umieszczona.
 */
size_t s_chfind(size_t blk_no, long long *difference);

#endif
