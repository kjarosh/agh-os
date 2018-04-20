#include <sys/time.h>
#include <sys/times.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/resource.h>

typedef struct watch_t {
	int error;
	struct rusage usage;
	struct timeval tv;
	
	double usec;
	double ssec;
	double rsec;
} watch_t;

void stopwatch(watch_t *watch) {
	struct rusage usage;
	struct timeval tv;
	
	if (getrusage(RUSAGE_SELF, &usage) != 0) {
		watch->error = -1;
		return;
	}
	
	if (gettimeofday(&tv, NULL) == -1) {
		watch->error = -1;
		return;
	}
	
	watch->usec = (double) (usage.ru_utime.tv_sec - watch->usage.ru_utime.tv_sec);
	watch->usec += (double) (usage.ru_utime.tv_usec - watch->usage.ru_utime.tv_usec) / 1000000.0;
	
	watch->ssec = (double) (usage.ru_stime.tv_sec - watch->usage.ru_stime.tv_sec);
	watch->ssec += (double) (usage.ru_stime.tv_usec - watch->usage.ru_stime.tv_usec) / 1000000.0;
	
	watch->rsec = (double) (tv.tv_sec - watch->tv.tv_sec);
	watch->rsec += (double) (tv.tv_usec - watch->tv.tv_usec) / 1000000.0;
	
	watch->usage = usage;
	watch->tv = tv;
}
