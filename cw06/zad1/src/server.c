#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>

#ifdef POSIXQ
#include <mqueue.h>
#include <fcntl.h>
#else
#include <sys/msg.h>
#endif

#include "config.h"

#define CLIENT_MAX 256

#ifdef POSIXQ
mqd_t queue;
#else
int queue;
#endif

struct client_t {
	pid_t pid;
	long id;
#ifdef POSIXQ
	mqd_t queue;
#else
	int queue;
#endif
};

struct client_t clients[CLIENT_MAX];
size_t next_client_id = 0;

static void cleanup(void) {
#ifdef POSIXQ
	mq_close(queue);
	mq_unlink(QUEUE_NAME);
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
#ifdef POSIXQ
	queue = mq_open(QUEUE_NAME, O_RDONLY);
	if (queue == (mqd_t) -1) {
		perror("Cannot create POSIX queue");
		exit(EXIT_FAILURE);
	}
#else
	key_t k = ftok(home_dir, 'K');
	if (k == -1) {
		perror("Cannot generate key");
		exit(EXIT_FAILURE);
	}
	
	queue = msgget(k, IPC_CREAT | IPC_EXCL | S_IRWXU);
	if (queue < 0) {
		perror("Cannot create System V queue");
		exit(EXIT_FAILURE);
	}
#endif
}

static int queue_receive(void *buf) {
#ifdef POSIXQ
	if (mq_receive(queue, buf, MSG_T_SIZE, NULL) < 0) {
		return -1;
	}
#else
	if (msgrcv(queue, buf, MSG_T_SIZE, 0, MSG_NOERROR) < 0) {
		return -1;
	}
#endif
	
	return 0;
}

static int client_queue(long id, struct msg_t *msg) {
#ifdef POSIXQ
#error
#else
	int cqueue = msgget(msg->queue_key, 0);
	if (cqueue < 0) {
		return -1;
	}
	
	clients[id].queue = cqueue;
#endif
	
	return 0;
}

static int respond(long id, struct msg_t *msg) {
#ifdef POSIXQ
#error
#else
	if (msgsnd(clients[id].queue, msg, MSG_T_SIZE, 0) < 0) {
		return -1;
	}
#endif
	
	return 0;
}

// ===================================================================

static void mirror_text(char *from, char *to) {
	size_t len = strlen(from);
	size_t dest = 0;
	
	while (len > 0) {
		to[dest++] = from[--len];
	}
}

// ===================================================================

int main(int argc, char **argv) {
	setup_home();
	
	prepare_queue();
	
	signal(SIGINT, sig_cleanup);
	atexit(cleanup);
	
	while (1) {
		struct msg_t msg;
		struct msg_t response;
		if (queue_receive(&msg) < 0) {
			perror("Cannot receive a message");
			sleep(1);
			continue;
		}
		
		switch (msg.type) {
		case HANDSHAKE:
			;
			struct client_t client;
			
			if (next_client_id >= CLIENT_MAX) {
				fprintf(stderr, "Client tried to connect, but client limit reached\n");
				continue;
			}
			
			client.pid = msg.from;
			client.id = next_client_id++;
			
			printf("Received HANDSHAKE from pid %d\n", client.pid);
			printf("His ID will be %ld\n", client.id);
			
			clients[client.id] = client;
			if (client_queue(client.id, &msg) < 0) {
				perror("Cannot open client's queue");
				continue;
			}
			
			// --------------
			
			response.type = client.id;
			response.from = getpid();
			response.client_id = client.id;
			break;
			
		case MIRROR:
			printf("Received MIRROR from pid %d\n", msg.from);
			
			char *text = msg.buf;
			
			// --------------
			
			response.type = client.id;
			response.from = getpid();
			mirror_text(text, response.buf);
			break;
		}
		
		respond(msg.client_id, &response);
	}
}
