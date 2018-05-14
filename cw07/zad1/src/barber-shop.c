#include "barber-shop.h"

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>

// SHARED
struct barber_shop_mem *bs;

static int shared_memory;

int wr_get_count() {
	return bs->wr_count;
}

int wr_pop(struct wr_seat **to_pop) {
	if (bs->wr_count <= 0) return 1;
	
	struct wr_seat *ret = &bs->waiting_room[bs->wr_start];
	bs->wr_start = (bs->wr_start + 1) % bs->wr_capacity;
	--bs->wr_count;
	*to_pop = ret;
	return 0;
}

struct wr_seat *wr_push() {
	if (bs->wr_count >= bs->wr_capacity) return NULL;
	
	struct wr_seat *ret = &bs->waiting_room[bs->wr_end];
	bs->wr_end = (bs->wr_end + 1) % bs->wr_capacity;
	++bs->wr_count;
	return ret;
}

struct wr_seat new_seat() {
	struct wr_seat ret = { true, getpid() };
	sem_init(&ret.waiting, true, 0);
	return ret;
}

struct wr_seat empty_seat() {
	struct wr_seat ret = { false, (pid_t) -1 };
	return ret;
}

static void map_shm(size_t size) {
	if ((bs = mmap(NULL, //
			size, //
			PROT_READ | PROT_WRITE, //
			MAP_SHARED, //
			shared_memory, 0)) == (void*) -1) {
		perror("Cannot mmap");
		exit(1);
	}
}

size_t get_bs_size(int wr_cap) {
	return sizeof(struct barber_shop_mem) + wr_cap * sizeof(struct wr_seat);
}

void initialize_barber(int wr_cap) {
	shared_memory = shm_open(SHM_NAME, O_RDWR | O_CREAT | O_EXCL, S_IRUSR | S_IWUSR);
	if (shared_memory == -1) {
		perror("Cannot create shared memory");
		exit(1);
	}
	
	if (ftruncate(shared_memory, get_bs_size(wr_cap)) < 0) {
		perror("Cannot truncate shared memory");
		exit(1);
	}
	
	map_shm(get_bs_size(wr_cap));
	
	bs->wr_capacity = wr_cap;
	bs->wr_count = 0;
	bs->wr_start = 0;
	bs->wr_end = 0;
	bs->barber_chair = empty_seat();
	bs->barber_sleeping = false;
	sem_init(&bs->mx_waiting_room, true, 1);
	sem_init(&bs->sem_barber_sleeping, true, 0);
	sem_init(&bs->sem_barber_ready, true, 0);
	sem_init(&bs->sem_customer_ready, true, 0);
	
	for (int i = 0; i < wr_cap; ++i) {
		bs->waiting_room[i] = empty_seat();
	}
}

void initialize_client() {
	shared_memory = shm_open(SHM_NAME, O_RDWR, 0);
	if (shared_memory == -1) {
		perror("Cannot open shared memory");
		exit(1);
	}
	
	struct stat statbuf;
	if (fstat(shared_memory, &statbuf) != 0) {
		perror("Cannot fstat shared memory");
		exit(1);
	}
	
	map_shm(statbuf.st_size);
}

static void unmap_shm() {
	if (bs != NULL && bs != (void*) -1) {
		munmap(bs, sizeof(struct barber_shop_mem));
	}
}

void dispose_barber() {
	unmap_shm();
	
	shm_unlink(SHM_NAME);
}

void dispose_client() {
	unmap_shm();
}

void log_barber(const char *message) {
	printf("[BARBER] %s\n", message);
	bs_sleep(1);
}

void log_barber2(const char *message, int i) {
	printf("[BARBER] %s %d\n", message, i);
	bs_sleep(1);
}

void log_client(const char *message) {
	printf("[CLIENT %d] %s\n", (int) getpid(), message);
	bs_sleep(1);
}

void bs_sleep(int seconds) {
#ifdef BS_SLEEP
	sleep(seconds);
#endif
}
