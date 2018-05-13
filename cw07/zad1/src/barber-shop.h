#include <semaphore.h>
#include <stdbool.h>
#include <sys/types.h>
#include <unistd.h>

#define SHM_NAME "/barber"

struct wr_seat {
	int taken;
	pid_t pid;
	sem_t waiting;
};

struct barber_shop_mem {
	struct wr_seat barber_chair;

	sem_t mx_waiting_room;
	sem_t sem_barber_sleeping;
	sem_t sem_barber_ready;

	bool barber_sleeping;

	int wr_capacity; /* private */
	int wr_count; /* private */
	int wr_start; /* private */
	int wr_end; /* private */
	
	struct wr_seat waiting_room[0];
};

extern struct barber_shop_mem *bs;

int wr_pop(struct wr_seat *to_pop);
struct wr_seat *wr_push();
int wr_get_count();

struct wr_seat new_seat();
struct wr_seat empty_seat();

void initialize_barber();
void initialize_client();
void dispose_barber();
void dispose_client();
