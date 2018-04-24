#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

#include "config.h"

#define CLIENT_MAX 256

q_type queue;

struct client_t {
	pid_t pid;
	long id;
	int stopped;
	q_type queue;
};

struct client_t clients[CLIENT_MAX];

static void cleanup(void) {
#ifdef POSIXQ
	for (int i = 0; i < CLIENT_MAX; ++i) {
		mq_close(clients[i].queue);
	}
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
	struct mq_attr attr = { 0, 10, MSG_T_SIZE, 0 };
	queue = mq_open(QUEUE_NAME, O_RDONLY | O_CREAT | O_EXCL, S_IRWXU, &attr);
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

static int receive_from_client(void *buf, int wait) {
#ifdef POSIXQ
	if (!wait) {
		struct mq_attr attr;
		mq_getattr(queue, &attr);
		attr.mq_flags |= O_NONBLOCK;
		mq_setattr(queue, &attr, NULL);
	}
	
	if (mq_receive(queue, buf, MSG_T_SIZE, NULL) < 0) {
		return -1;
	}
#else
	if (msgrcv(queue, buf, MSG_T_SIZE, 0, MSG_NOERROR | (wait ? 0 : IPC_NOWAIT)) < 0) {
		return -1;
	}
#endif
	
	return 0;
}

static int prepare_client_queue(long cid, struct msg_t *msg) {
#ifdef POSIXQ
	mqd_t cqueue = mq_open(msg->buf, O_WRONLY);
	if (cqueue == (mqd_t) -1) {
		return -1;
	}
#else
	int cqueue = msgget(msg->queue_key, 0);
	if (cqueue < 0) {
		return -1;
	}
#endif
	clients[cid].queue = cqueue;
	
	return 0;
}

static int send_to_client(long cid, struct msg_t *msg) {
#ifdef POSIXQ
	if (mq_send(clients[cid].queue, (char*) msg, MSG_T_SIZE, 1) < 0) {
		return -1;
	}
#else
	if (msgsnd(clients[cid].queue, msg, MSG_T_SIZE, 0) < 0) {
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
	to[dest] = 0;
}

void terminate_at_nl(char* buf) {
	char *c = &buf[0];
	while (*c != '\n' && *c != 0)
		++c;
	*c = 0;
}

// ===================================================================

static void print_help(char *program) {
	printf("Usage:\n");
	printf("\t%s\n", program);
}

int main(int argc, char **argv) {
	if (argc != 1) {
		print_help(argc == 0 ? "server" : argv[0]);
		return -1;
	}
	
	for (int i = 0; i < CLIENT_MAX; ++i) {
		clients[i].stopped = 1;
	}
	
	setup_home();
	
	prepare_queue();
	
	signal(SIGINT, sig_cleanup);
	atexit(cleanup);
	
	int end = 0;
	while (1) {
		struct msg_t msg;
		struct msg_t response;
		if (receive_from_client(&msg, !end) < 0) {
			if (errno == ENOMSG || errno == EAGAIN) exit(EXIT_SUCCESS);
			perror("Cannot receive a message");
			exit(EXIT_FAILURE);
		}
		
		if (msg.type != HANDSHAKE && clients[msg.client_id].stopped) continue;
		
		switch (msg.type) {
		case HANDSHAKE:
			;
			struct client_t client;
			client.stopped = 0;
			
			long cid = -1;
			for (int i = 0; i < CLIENT_MAX; ++i) {
				if (!clients[i].stopped) {
					cid = i;
					break;
				}
			}
			
			if (cid == -1) {
				fprintf(stderr, "Client tried to connect, but client limit reached\n");
				continue;
			}
			
			client.pid = msg.from;
			client.id = cid;
			
			printf("Received HANDSHAKE from pid %d\n", client.pid);
			printf("His ID will be %ld\n", client.id);
			
			clients[client.id] = client;
			if (prepare_client_queue(client.id, &msg) < 0) {
				perror("Cannot open client's queue");
				continue;
			}
			
			// --------------
			
			response.type = client.id;
			response.from = getpid();
			response.client_id = client.id;
			break;
			
		case STOP:
			printf("Received STOP from pid %d\n", msg.from);
			
			clients[msg.client_id].stopped = 1;
#ifdef POSIXQ
			mq_close(clients[msg.client_id].queue);
#endif
			
			continue;
			
		case MIRROR:
			printf("Received MIRROR from pid %d\n", msg.from);
			
			char *text = msg.buf;
			
			// --------------
			
			response.type = msg.client_id;
			response.client_id = msg.client_id;
			response.from = getpid();
			mirror_text(text, response.buf);
			break;
			
		case CALC:
			printf("Received CALC from pid %d\n", msg.from);
			
			char *cmd = malloc(sizeof(char) * (strlen(msg.buf) + 18));
			sprintf(cmd, "echo '%s' | bc 2>&1", msg.buf);
			FILE *calc = popen(cmd, "r");
			free(cmd);
			
			// --------------
			
			response.type = msg.client_id;
			response.client_id = msg.client_id;
			response.from = getpid();
			response.buf[0] = 0;
			fgets(response.buf, MSG_BUF_SIZE, calc);
			
			terminate_at_nl(response.buf);
			pclose(calc);
			break;
			
		case TIME:
			printf("Received TIME from pid %d\n", msg.from);
			
			FILE *date = popen("date", "r");
			
			// --------------
			
			response.type = msg.client_id;
			response.client_id = msg.client_id;
			response.from = getpid();
			response.buf[0] = 0;
			fgets(response.buf, MSG_BUF_SIZE, date);
			
			terminate_at_nl(response.buf);
			
			pclose(date);
			break;
			
		case END:
			printf("Received END from pid %d\n", msg.from);
			
			end = 1;
			continue;
		}
		
		if (send_to_client(response.client_id, &response) < 0) {
			perror("Cannot respond to the client");
		}
	}
}
