#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

#include "config.h"

long client_id;

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

static int respond_server(struct msg_t *msg) {
#ifdef POSIXQ
#error
#else
	if (msgsnd(serv_queue, msg, MSG_T_SIZE, 0) < 0) {
		return -1;
	}
#endif
	
	return 0;
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

// ===================================================================

static void handshake(void) {
	struct msg_t hs;
	hs.from = getpid();
	hs.queue_key = queue_key;
	hs.type = HANDSHAKE;
	
	if (respond_server(&hs) < 0) {
		perror("Cannot shake hands");
		exit(EXIT_FAILURE);
	}
	
	struct msg_t msg;
	queue_receive(&msg);
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
	printf("\tmirror <text> -- mirror text\n");
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
	
	prepare_queue();
	
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
		
		while (*text != ' ' && *text != 0)
			++text;
		if (*text != 0) *(text++) = 0;
		
		size_t textlen = strlen(text);
		if (text[textlen - 1] == '\n') text[textlen - 1] = 0;
		
		if (strcmp(command, "mirror") == 0) {
			msg.type = MIRROR;
			strcpy(msg.buf, text);
		} else if (strcmp(command, "calc") == 0) {
			msg.type = CALC;
			strcpy(msg.buf, text);
		} else if (strcmp(command, "time") == 0) {
			msg.type = TIME;
			strcpy(msg.buf, text);
		} else if (strcmp(command, "end") == 0) {
			msg.type = END;
			strcpy(msg.buf, text);
		} else {
			fprintf(stderr, "Unrecognized command: %s\n", command);
			continue;
		}
		
		respond_server(&msg);
		
		struct msg_t resp;
		queue_receive(&resp);
		
		printf("Response from server: %s\n", resp.buf);
	}
}
