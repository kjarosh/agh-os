#include "challoc.h"

#include <stdint.h>
#include <stdio.h>

sblk_table s_chtable;

blk_table challoc(size_t blk_count, size_t blk_size) {
	blk_table ret;
	ret.blk_count = blk_count;
	ret.blk_size = blk_size;
	ret.blks = (blk_ptr*) calloc(blk_count, sizeof(blk_ptr));
	
	if (ret.blks == NULL) {
		ret.blk_count = 0;
	}
	
	return ret;
}

void chfree(blk_table tbl) {
	if (tbl.blk_count == 0 || tbl.blks == NULL) return;
	
	size_t count = tbl.blk_count;
	for (size_t i = 0; i < count; ++i) {
		chfree_blk(tbl, i);
	}
	
	free(tbl.blks);
}

int challoc_blk(blk_table tbl, size_t blk_no) {
	if (tbl.blks == NULL) return -2;
	
	if (tbl.blks[blk_no] != NULL) {
		chfree_blk(tbl, blk_no);
	}
	
	tbl.blks[blk_no] = (blk_t *) calloc(1, sizeof(blk_t) + tbl.blk_size * sizeof(char));
	
	if (tbl.blks[blk_no] == NULL) {
		return -1;
	}
	
	return 0;
}

void chfree_blk(blk_table tbl, size_t blk_no) {
	if (tbl.blks == NULL) return;
	if (tbl.blks[blk_no] == NULL) return;
	
	free(tbl.blks[blk_no]);
	tbl.blks[blk_no] = NULL;
}

long long chblk_sum(blk_table tbl, size_t blk_no) {
	long long ret = 0;
	
	if (tbl.blks == NULL) return -2;
	
	blk_ptr blk = tbl.blks[blk_no];
	
	if (blk == NULL) return -1;
	
	size_t blk_size = tbl.blk_size;
	
	for (size_t i = 0; i < blk_size; ++i) {
		ret += tbl.blks[blk_no]->content[i];
	}
	
	return ret;
}

size_t chfind(blk_table tbl, size_t blk_no, long long *difference) {
	if (tbl.blks[blk_no] == NULL) {
		if (difference != NULL) {
			*difference = 0;
		}
		
		return blk_no;
	}
	
	size_t blk_count = tbl.blk_count;
	long long blk_sums[blk_count];
	
	for (size_t i = 0; i < blk_count; ++i) {
		blk_sums[i] = chblk_sum(tbl, i);
	}
	
	long long c_sum = blk_sums[blk_no];
	size_t index = (blk_no + 1) % blk_count;
	long long diff = abs(c_sum - blk_sums[index]);
	
	for (size_t i = 0; i < blk_count; ++i) {
		if (i == blk_no) continue;
		
		uintmax_t diff2 = abs(c_sum - blk_sums[index]);
		if (diff2 < diff) {
			index = i;
			diff = diff2;
		}
	}
	
	if (blk_sums[index] == -1) {
		index = blk_no;
		diff = 0;
	}
	
	if (difference != NULL) {
		*difference = diff;
	}
	
	return index;
}

char *s_chgetblk(size_t blk_no) {
	return &s_chtable.blks[blk_no].content[0];
}

long long s_chblk_sum(size_t blk_no) {
	long long ret = 0;
	for (size_t i = 0; i < CHALLOC_STATIC_BLK_SIZE; ++i) {
		ret += s_chtable.blks[blk_no].content[i];
	}
	
	return ret;
}

size_t s_chfind(size_t blk_no, long long *difference) {
	const int blk_count = CHALLOC_STATIC_BLK_COUNT;
	
	long long blk_sums[blk_count];
	
	for (size_t i = 0; i < blk_count; ++i) {
		blk_sums[i] = s_chblk_sum(i);
	}
	
	long long c_sum = blk_sums[blk_no];
	size_t index = (blk_no + 1) % blk_count;
	long long diff = abs(c_sum - blk_sums[index]);
	
	for (size_t i = 0; i < blk_count; ++i) {
		if (i == blk_no) continue;
		
		uintmax_t diff2 = abs(c_sum - blk_sums[index]);
		if (diff2 < diff) {
			index = i;
			diff = diff2;
		}
	}
	
	if (difference != NULL) {
		*difference = diff;
	}
	
	return index;
}

void s_chrndfill(size_t blk_no) {
	for (int i = 0; i < CHALLOC_STATIC_BLK_SIZE; ++i) {
		s_chtable.blks[blk_no].content[i] = rand();
	}
}

void s_chvoid(size_t blk_no) {
	for (int i = 0; i < CHALLOC_STATIC_BLK_SIZE; ++i) {
		s_chtable.blks[blk_no].content[i] = 0;
	}
}
