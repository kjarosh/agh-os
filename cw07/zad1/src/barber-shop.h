#include <semaphore.h>
#include <stdbool.h>
#include <sys/types.h>
#include <unistd.h>

#ifdef SYS_V
union semun {
	int val;
	struct semid_ds *buf;
	unsigned short *array;
	struct seminfo *__buf;
};
#endif

#ifdef SYS_V
typedef int semaphore_t;
typedef int semaphore_p;

#define conv_tp(s) (s)
#else
typedef sem_t semaphore_t;
typedef sem_t *semaphore_p;

#define conv_tp(s) (&(s))
#endif

#define SHM_NAME "/barber"

struct wr_seat {
	int taken;
	pid_t pid;
	semaphore_t waiting;
};

struct barber_shop_mem {
	struct wr_seat barber_chair;

	semaphore_t mx_waiting_room;
	semaphore_t sem_barber_sleeping;
	semaphore_t sem_barber_ready;
	semaphore_t sem_customer_ready;

	bool barber_sleeping;

	int wr_capacity; /* private */
	int wr_count; /* private */
	int wr_start; /* private */
	int wr_end; /* private */
	
	struct wr_seat waiting_room[0];
};

extern struct barber_shop_mem *bs;

int wr_pop(struct wr_seat **to_pop);
struct wr_seat *wr_push(void);
int wr_get_count(void);

struct wr_seat new_seat(void);
struct wr_seat empty_seat(void);
void delete_seat(struct wr_seat *seat);

void initialize_barber(int wr_cap);
void initialize_client(void);
void dispose_barber(void);
void dispose_client(void);

void log_barber(const char *message);
void log_barber2(const char *message, int i);
void log_client(const char *message);

void bs_sleep(int seconds);

int semaphore_wait(semaphore_p sem);
void semaphore_wait2(semaphore_p sem);
void semaphore_post(semaphore_p sem);
