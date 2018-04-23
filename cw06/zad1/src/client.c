#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "config.h"

#ifdef POSIXQ
mqd_t serv_queue;
mqd_t queue;
#else
int serv_queue;
int queue;
key_t queue_key;
#endif

static void cleanup(void) {
#ifdef POSIXQ
	mq_close(serv_queue);
	mq_close(queue);
#error mq_unlink(QUEUE_NAME);
#else
	msgctl(queue, IPC_RMID, NULL);
#endif
}

static void sig_cleanup(int sig) {
	cleanup();
	exit(0);
}

// ===================================================================

static void prepare_queue(void) {
	srand(time(NULL));
#ifdef POSIXQ
#error
#else
	int proj_id = rand() ^ getpid();
	queue_key = ftok(home_dir, proj_id);
	if (queue_key == -1) {
		perror("Cannot generate key");
		exit(EXIT_FAILURE);
	}
	
	queue = msgget(queue_key, IPC_CREAT | IPC_EXCL | S_IRWXU);
	if (queue < 0) {
		perror("Cannot create System V queue");
		exit(EXIT_FAILURE);
	}
	
	key_t serv_key = ftok(home_dir, 'K');
	if (queue_key == -1) {
		perror("Cannot generate server key");
		exit(EXIT_FAILURE);
	}
	
	serv_queue = msgget(serv_key, S_IRWXU);
	if (serv_queue < 0) {
		perror("Cannot open server queue");
		exit(EXIT_FAILURE);
	}
#endif
}

static int respond(struct msg_t *msg) {
#ifdef POSIXQ
#error
#else
	if (msgsnd(serv_queue, msg, MSG_T_SIZE, 0) < 0) {
		return -1;
	}
#endif
	
	return 0;
}

static void handshake(void) {
	struct msg_t hs;
	hs.from = getpid();
	hs.queue_key = queue_key;
	hs.type = HANDSHAKE;
	
	if (respond(&hs) < 0) {
		perror("Cannot shake hands");
		exit(EXIT_FAILURE);
	}
}

// ===================================================================

int main(int argc, char **argv) {
	setup_home();
	
	prepare_queue();
	
	signal(SIGINT, sig_cleanup);
	atexit(cleanup);
	
	handshake();
}
