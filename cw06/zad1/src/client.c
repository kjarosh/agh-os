#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

#include "config.h"

/**
 * Client ID received from the server.
 */
long client_id;

/**
 * When 1, STOP shall not be sent when exiting.
 */
int nostop = 0;

/**
 * Opened server queue.
 */
q_type serv_queue;

/**
 * Opened client queue.
 */
q_type queue;

#ifdef POSIXQ
char *queue_name;
#else
key_t queue_key;
#endif

static int send_to_server(struct msg_t *msg);

static void cleanup(void) {
	if (!nostop) {
		struct msg_t msg;
		msg.client_id = client_id;
		msg.from = getpid();
		msg.type = STOP;
		send_to_server(&msg);
	}
	
#ifdef POSIXQ
	mq_close(serv_queue);
	mq_close(queue);
	mq_unlink(queue_name);
	free(queue_name);
#else
	msgctl(queue, IPC_RMID, NULL);
#endif
}

static void sig_cleanup(int sig) {
	cleanup();
	exit(0);
}

// ===================================================================

static void prepare_queues(void) {
	srand(time(NULL));
#ifdef POSIXQ
	queue_name = malloc(sizeof(char) * 255);
	sprintf(queue_name, QUEUE_PREFIX "%d", rand() ^ getpid());
	
	struct mq_attr attr = {0, 10, MSG_SIZE, 0};
	queue = mq_open(queue_name, O_RDONLY | O_CREAT | O_EXCL, S_IRWXU, &attr);
	if (queue == -1) {
		perror("Cannot create POSIX queue");
		exit(EXIT_FAILURE);
	}
	
	serv_queue = mq_open(QUEUE_NAME, O_WRONLY);
	if (serv_queue == -1) {
		perror("Cannot open server queue");
		exit(EXIT_FAILURE);
	}
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

static int send_to_server(struct msg_t *msg) {
#ifdef POSIXQ
	if (mq_send(serv_queue, (char*) msg, MSG_SIZE, 1) < 0) {
		return -1;
	}
#else
	if (msgsnd(serv_queue, msg, MSG_SIZE, 0) < 0) {
		return -1;
	}
#endif
	
	return 0;
}

static int receive_from_server(void *buf) {
#ifdef POSIXQ
	if (mq_receive(queue, buf, MSG_SIZE, NULL) < 0) {
		return -1;
	}
#else
	if (msgrcv(queue, buf, MSG_SIZE, 0, MSG_NOERROR) < 0) {
		return -1;
	}
#endif
	
	return 0;
}

// ===================================================================

static void handshake(void) {
	struct msg_t hs;
	hs.from = getpid();
	hs.type = HANDSHAKE;
	
#ifdef POSIXQ
	strcpy(hs.buf, queue_name);
#else
	hs.queue_key = queue_key;
#endif
	
	if (send_to_server(&hs) < 0) {
		perror("Cannot shake hands");
		exit(EXIT_FAILURE);
	}
	
	struct msg_t msg;
	if (receive_from_server(&msg) < 0) {
		perror("Cannot receive handshake");
		exit(EXIT_FAILURE);
	}
	printf("Assigned ID = %ld\n", msg.client_id);
	client_id = msg.client_id;
}

// ===================================================================

void print_help(char *program) {
	printf("Usage:\n");
	printf("\t%s\n", program);
	printf("\t%s -\n", program);
	printf("\t%s <file with commands>\n", program);
	printf("\n");
	printf("Available commands:\n");
	printf("\tmirror <text> -- mirror the text\n");
	printf("\tcalc <expr>   -- calculate the expression\n");
	printf("\ttime          -- send current date and time\n");
	printf("\tend           -- stop the server\n");
}

int main(int argc, char **argv) {
	if (argc > 2) {
		print_help(argv[0]);
		return -1;
	}
	
	char *file;
	if (argc != 2)
		file = "-";
	else
		file = argv[1];
	
	FILE *fp = strcmp(file, "-") == 0 ? stdin : fopen(file, O_RDONLY);
	if (fp == NULL) {
		perror("Cannot open input file");
		exit(EXIT_FAILURE);
	}
	
	setup_home();
	
	prepare_queues();
	
	signal(SIGINT, sig_cleanup);
	atexit(cleanup);
	
	handshake();
	
	printf("Connection with server established!\n");
	
	char buf[1024];
	while (fgets(buf, 1024, fp) != NULL) {
		char *command = &buf[0];
		char *text = &buf[0];
		
		struct msg_t msg;
		msg.client_id = client_id;
		msg.from = getpid();
		
		// iterate until a space or null-term encountered
		//   then insert null-term
		while (*text != ' ' && *text != 0)
			++text;
		if (*text != 0) *(text++) = 0;
		
		terminate_at_nl(text);
		
		if (strcmp(command, "mirror") == 0) {
			msg.type = MIRROR;
		} else if (strcmp(command, "calc") == 0) {
			msg.type = CALC;
		} else if (strcmp(command, "time") == 0) {
			msg.type = TIME;
		} else if (strcmp(command, "end") == 0) {
			msg.type = END;
			send_to_server(&msg);
			nostop = 1;
			exit(EXIT_SUCCESS);
			// not reachable
		} else {
			fprintf(stderr, "Unrecognized command: %s\n", command);
			continue;
			// not reachable
		}
		
		strcpy(msg.buf, text);
		send_to_server(&msg);
		
		struct msg_t resp;
		receive_from_server(&resp);
		
		printf("Response from the server: %s\n", resp.buf);
	}
}
