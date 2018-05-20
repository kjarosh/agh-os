#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <sched.h>
#include <sys/times.h>

#include "filter.h"
#include "pgm.h"

#define run_or_fail(f, msg) do{ \
		if(f != 0){ \
			perror(msg); \
			dispose(); exit(1); \
		} \
	}while(0)

struct thread_args {
	int thread_id;
	int thread_count;
};

static void print_help(char *program) {
	printf("Usage:\n");
	printf("\t %s gen <size> <filter>\n", program);
	printf("or:\n");
	printf("\t %s <threads> <input> <filter> <output>\n", program);
}

filter_t *f = NULL;
pgm_image *in_image = NULL;
pgm_image *out_image = NULL;
long pixel_count = 0;

int pixels_processed = 0;
sem_t pp_sem;

void dispose() {
	destroy_filter(f);
	pgm_destroy(in_image);
	pgm_destroy(out_image);
}

void *thread_run(void *args0) {
	struct thread_args *args = (struct thread_args*) args0;
	int id = args->thread_id;
	int count = args->thread_count;
	
	long processed = 0;
	long total = 0;
	for (long pixel = id; pixel < pixel_count; pixel += count) {
		++total;
		int x = pixel % in_image->width;
		int y = pixel / in_image->width;
		pgm_set_pixel(out_image, x, y, apply_filter(f, in_image, x, y, FILTER_STRETCH));
		
		++processed;
		
		if (processed > count + 5 && sem_trywait(&pp_sem) == 0) {
			pixels_processed += processed;
			processed = 0;
			
			sem_post(&pp_sem);
		}
	}
	
	free(args);
	printf("Thread %d done, %ld/%ld pixels\n", id, total, pixel_count);
	return NULL;
}

void *thread_progress(void *args) {
	int pixels_count = in_image->width * in_image->height;
	
	while (1) {
		printf("\rProgress: %.2lf%%", pixels_processed * 100. / pixels_count);
		fflush(stdout);
		usleep(1000000 / 5);
	}
}

int main(int argc, char **argv) {
	sem_init(&pp_sem, 0, 1);
	
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
	
	int thread_count = atoi(argv[1]);
	char *input = argv[2];
	char *filter = argv[3];
	char *output = argv[4];
	
	if (thread_count <= 0) {
		fprintf(stderr, "Invalid number of threads\n");
		return 1;
	}
	
	run_or_fail(read_filter(&f, filter), "Cannot read filter");
	run_or_fail(pgm_load(&in_image, input), "Cannot read input file");
	pixel_count = in_image->height * in_image->width;
	
	out_image = pgm_create(in_image->width, in_image->height);
	if (out_image == NULL) {
		perror("Cannot create image");
		dispose();
		exit(1);
	}
	
	printf("Using %d threads\n", thread_count);
	if (thread_count > sysconf(_SC_NPROCESSORS_ONLN)) {
		printf("Warning: this system has only %ld processors available, "
				"using greater number will be slower\n", sysconf(_SC_NPROCESSORS_ONLN));
	}
	
	struct tms tms;
	clock_t time = times(&tms);
	
	pthread_t progress;
	pthread_create(&progress, NULL, thread_progress, NULL);
	
	pthread_t threads[thread_count];
	for (int i = 0; i < thread_count; ++i) {
		struct thread_args *args = malloc(sizeof(struct thread_args));
		if (args == NULL) {
			perror("Cannot allocate memory for a thread");
			dispose();
			exit(1);
		}
		args->thread_id = i;
		args->thread_count = thread_count;
		
		pthread_create(&threads[i], NULL, thread_run, (void*) args);
	}
	
	for (int i = 0; i < thread_count; ++i) {
		void *retval;
		run_or_fail(pthread_join(threads[i], &retval), "Cannot join threads");
		if (retval != NULL) {
			fprintf(stderr, "Thread failed: %s\n", (char*) retval);
			dispose();
			exit(1);
		}
	}
	
	time = time == ((clock_t) -1) ? time : (times(&tms) - time);
	
	pthread_cancel(progress);
	// print a new line after the progress
	printf("\n");
	
	run_or_fail(pgm_save(out_image, output), "Cannot save file");
	
	printf("Total time: %lfs\n", (double) time / sysconf(_SC_CLK_TCK));
	// ------------------------------------
	
	dispose();
	return 0;
}
