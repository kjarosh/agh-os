#include "barber-shop.h"

#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <semaphore.h>
#include <stdbool.h>

int wr_capacity;
int wr_start = 0;
int wr_end = 0;
int wr_count = 0;
struct wr_seat *waiting_room;
struct wr_seat barber_chair = { 0, (pid_t) -1 };

bool barber_sleeping;

sem_t *mx_waiting_room;
sem_t *sem_barber_sleeping;
sem_t *sem_barber_ready;

int wr_get_count() {
	return wr_count;
}

int wr_pop(struct wr_seat *to_pop) {
	if (wr_count <= 0) return 1;
	
	struct wr_seat *ret = &waiting_room[wr_start];
	wr_start = (wr_start + 1) % wr_capacity;
	--wr_count;
	*to_pop = *ret;
	return 0;
}

struct wr_seat *wr_push() {
	if (wr_count >= wr_capacity) return NULL;
	
	struct wr_seat *ret = &waiting_room[wr_end];
	wr_end = (wr_end + 1) % wr_capacity;
	++wr_count;
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
