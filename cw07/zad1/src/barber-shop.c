#include "barber-shop.h"

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <pwd.h>
#include <errno.h>
#include <time.h>

// SHARED
struct barber_shop_mem *bs = (void*) -1;

static int shared_memory;

int wr_get_count(void) {
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

struct wr_seat *wr_push(void) {
	if (bs->wr_count >= bs->wr_capacity) return NULL;
	
	struct wr_seat *ret = &bs->waiting_room[bs->wr_end];
	bs->wr_end = (bs->wr_end + 1) % bs->wr_capacity;
	++bs->wr_count;
	return ret;
}

struct wr_seat new_seat(void) {
	struct wr_seat ret = { true, getpid() };
	
#ifdef SYS_V
	ret.waiting = semget(IPC_PRIVATE, 1, S_IRUSR | S_IWUSR);
	semctl(ret.waiting, 0, SETVAL, (union semun) 0);
#else
	sem_init(conv_tp(ret.waiting), true, 0);
#endif
	
	return ret;
}

void delete_seat(struct wr_seat *seat) {
#ifdef SYS_V
	union semun dummy;
	semctl(seat->waiting, 0, IPC_RMID, dummy);
#endif
}

struct wr_seat empty_seat() {
	struct wr_seat ret = { false, (pid_t) -1 };
	return ret;
}

#ifndef SYS_V
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

static void unmap_shm(void) {
	if (bs != NULL && bs != (void*) -1) {
		munmap(bs, sizeof(struct barber_shop_mem));
	}
}
#endif

size_t get_bs_size(int wr_cap) {
	return sizeof(struct barber_shop_mem) + wr_cap * sizeof(struct wr_seat);
}

void initialize_barber(int wr_cap) {
	size_t bs_size = get_bs_size(wr_cap);
	
#ifdef SYS_V
	char *home = getpwuid(getuid())->pw_dir;
	key_t key = ftok(home, 'B');
	if (key == -1) {
		perror("Cannot create key");
		exit(1);
	}

	shared_memory = shmget(key, bs_size, IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR);
	if (shared_memory == -1) {
		perror("Cannot create shared memory");
		exit(1);
	}

	bs = shmat(shared_memory, NULL, 0);
#else
	shared_memory = shm_open(SHM_NAME, O_RDWR | O_CREAT | O_EXCL, S_IRUSR | S_IWUSR);
	if (shared_memory == -1) {
		perror("Cannot create shared memory");
		exit(1);
	}
	
	if (ftruncate(shared_memory, bs_size) < 0) {
		perror("Cannot truncate shared memory");
		exit(1);
	}
	
	map_shm(bs_size);
#endif
	
	bs->wr_capacity = wr_cap;
	bs->wr_count = 0;
	bs->wr_start = 0;
	bs->wr_end = 0;
	bs->barber_chair = empty_seat();
	bs->barber_sleeping = false;
	
#ifdef SYS_V
	bs->mx_waiting_room = semget(IPC_PRIVATE, 1, S_IRUSR | S_IWUSR);
	semctl(bs->mx_waiting_room, 0, SETVAL, (union semun) 1);

	bs->sem_barber_sleeping = semget(IPC_PRIVATE, 1, S_IRUSR | S_IWUSR);
	semctl(bs->sem_barber_sleeping, 0, SETVAL, (union semun) 0);

	bs->sem_barber_ready = semget(IPC_PRIVATE, 1, S_IRUSR | S_IWUSR);
	semctl(bs->sem_barber_ready, 0, SETVAL, (union semun) 0);

	bs->sem_customer_ready = semget(IPC_PRIVATE, 1, S_IRUSR | S_IWUSR);
	semctl(bs->sem_customer_ready, 0, SETVAL, (union semun) 0);
#else
	sem_init(conv_tp(bs->mx_waiting_room), true, 1);
	sem_init(conv_tp(bs->sem_barber_sleeping), true, 0);
	sem_init(conv_tp(bs->sem_barber_ready), true, 0);
	sem_init(conv_tp(bs->sem_customer_ready), true, 0);
#endif
	
	for (int i = 0; i < wr_cap; ++i) {
		bs->waiting_room[i] = empty_seat();
	}
}

void initialize_client(void) {
#ifdef SYS_V
	char *home = getpwuid(getuid())->pw_dir;
	key_t key = ftok(home, 'B');
	if (key == -1) {
		perror("Cannot create key");
		exit(1);
	}

	shared_memory = shmget(key, 0, 0);
	if (shared_memory == -1) {
		perror("Cannot open shared memory");
		exit(1);
	}

	bs = shmat(shared_memory, NULL, 0);
#else
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
#endif
}

void dispose_barber(void) {
#ifdef SYS_V
	if(bs != (void*) -1) {
		union semun dummy;
		semctl(bs->mx_waiting_room, 0, IPC_RMID, dummy);
		semctl(bs->sem_barber_sleeping, 0, IPC_RMID, dummy);
		semctl(bs->sem_barber_ready, 0, IPC_RMID, dummy);
		semctl(bs->sem_customer_ready, 0, IPC_RMID, dummy);
	}

	shmdt(bs);
	shmctl(shared_memory, IPC_RMID, NULL);
#else
	unmap_shm();
	
	shm_unlink(SHM_NAME);
#endif
}

void dispose_client(void) {
#ifdef SYS_V
	shmdt(bs);
#else
	unmap_shm();
#endif
}

void log_barber(const char *message) {
	struct timespec tp;
	clock_gettime(CLOCK_MONOTONIC, &tp);
	printf("%ld.%ld [BARBER] %s\n", (long) tp.tv_sec, (long) tp.tv_nsec, message);
	bs_sleep(1);
}

void log_barber2(const char *message, int i) {
	struct timespec tp;
	clock_gettime(CLOCK_MONOTONIC, &tp);
	printf("%ld.%ld [BARBER] %s %d\n", (long) tp.tv_sec, (long) tp.tv_nsec, message, i);
	bs_sleep(1);
}

void log_client(const char *message) {
	struct timespec tp;
	clock_gettime(CLOCK_MONOTONIC, &tp);
	printf("%ld.%ld [CLIENT %d] %s\n", (long) tp.tv_sec, (long) tp.tv_nsec, (int) getpid(),
			message);
	bs_sleep(1);
}

void bs_sleep(int seconds) {
#ifdef BS_SLEEP
	sleep(seconds);
#endif
}

int semaphore_wait(semaphore_p sem) {
#ifdef SYS_V
	struct sembuf buf;
	buf.sem_num = 0;
	buf.sem_op = -1;
	buf.sem_flg = 0;
	return semop(sem, &buf, 1);
#else
	return sem_wait(sem);
#endif
}

void semaphore_wait2(semaphore_p sem) {
	if (semaphore_wait(sem) != 0) {
		perror("Cannot wait");
		exit(1);
	}
}

void semaphore_post(semaphore_p sem) {
#ifdef SYS_V
	struct sembuf buf;
	buf.sem_num = 0;
	buf.sem_op = 1;
	buf.sem_flg = 0;
	if (semop(sem, &buf, 1) != 0) {
		perror("Cannot post");
		exit(1);
	}
#else
	if (sem_post(sem) != 0) {
		perror("Cannot post");
		exit(1);
	}
#endif
}
