#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

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
size_t next_client_id = 1;

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

static int queue_receive(void *buf, int wait) {
#ifdef POSIXQ
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

static int respond_client(long id, struct msg_t *msg) {
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
	to[dest] = 0;
}

void terminate_at_nl(char* buf) {
	char *c = &buf[0];
	while (*c != '\n' && *c != 0)
		++c;
	*c = 0;
}

// ===================================================================

int main(int argc, char **argv) {
	setup_home();
	
	prepare_queue();
	
	signal(SIGINT, sig_cleanup);
	atexit(cleanup);
	
	int end = 0;
	while (1) {
		struct msg_t msg;
		struct msg_t response;
		if (queue_receive(&msg, !end) < 0) {
			if (errno == ENOMSG) exit(EXIT_SUCCESS);
			perror("Cannot receive a message");
			exit(EXIT_FAILURE);
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
			
		case CALC:
			printf("Received CALC from pid %d\n", msg.from);
			
			char *cmd = malloc(sizeof(char) * (strlen(msg.buf) + 18));
			sprintf(cmd, "echo '%s' | bc 2>&1", msg.buf);
			FILE *calc = popen(cmd, "r");
			free(cmd);
			
			// --------------
			
			response.type = client.id;
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
			
			response.type = client.id;
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
		
		if (respond_client(response.type, &response) < 0) {
			perror("Cannot respond to the client");
		}
	}
}
