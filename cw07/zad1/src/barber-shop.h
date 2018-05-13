#include <sys/time.h>
#include <semaphore.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdbool.h>

struct wr_seat {
	int taken;
	pid_t pid;
	sem_t waiting;
};

extern int wr_capacity;
extern struct wr_seat *waiting_room;
extern struct wr_seat barber_chair;

extern sem_t *mx_waiting_room;
extern sem_t *sem_barber_sleeping;
extern sem_t *sem_barber_ready;

extern bool barber_sleeping;

int wr_pop(struct wr_seat *to_pop);
struct wr_seat *wr_push();
int wr_get_count();

struct wr_seat new_seat();
struct wr_seat empty_seat();

