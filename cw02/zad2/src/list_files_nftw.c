#include <ftw.h>

static cond_t gcond;
static struct date gdate;

int treewalk(const char *fpath, const struct stat *sb, int typeflag, struct FTW *ftwbuf) {
	if (typeflag != FTW_F) return 0;
	
	if (check_time(sb->st_mtime, gcond, gdate) != 0) {
#ifdef DEBUG
		printf("[DEBUG] %20s ", "!COND:");
#else
		return 0;
#endif
	}
	
	print_access(sb);
	print_last_mod(sb);
	print_size(sb);
	printf("%s\n", fpath);
	
	return 0;
}

void list_files(char *dirname, cond_t cond, struct date date) {
	gdate = date;
	gcond = cond;
	
	int ret = nftw(dirname, treewalk, 10, FTW_PHYS);
	
	if (ret != 0) {
		fprintf(stderr, "nftw failed\n");
	}
}
