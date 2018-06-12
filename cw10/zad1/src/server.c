#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include <errno.h>

#include <sys/socket.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <netinet/ip.h>

#include <sys/epoll.h>

#include "config.h"
#include "server.h"
#include "server_accept.h"
#include "server_listen.h"
#include "server_pinger.h"
#include "server_cllist.h"
#include "request.h"

int port;
char *local_path;

int sock_net;
int sock_local;

int waiting_poll;

void print_prompt() {
	printf("in [%d]: ", current_request_id);
	fflush(stdout);
}

void add_to_poll(int sock) {
	struct epoll_event event;
	event.events = EPOLLIN;
	event.data.fd = sock;
	zero_or_fail(epoll_ctl(waiting_poll, EPOLL_CTL_ADD, sock, &event), "Cannot add to epoll");
}

void teardown_socks() {
	shutdown(sock_net, SHUT_RDWR);
	shutdown(sock_local, SHUT_RDWR);
	close(sock_net);
	close(sock_local);
	unlink(local_path);
}

void setup_socks() {
	if ((sock_net = socket(AF_INET, CONF_SOCK_TYPE, 0)) == -1) {
		perror("Cannot create network socket");
		exit(-1);
	}
	
	if ((sock_local = socket(AF_LOCAL, CONF_SOCK_TYPE, 0)) == -1) {
		perror("Cannot create local socket");
		exit(-1);
	}
	
	struct sockaddr_in address_net;
	address_net.sin_family = AF_INET;
	address_net.sin_port = htons(port);
	address_net.sin_addr.s_addr = INADDR_ANY;
	
	struct sockaddr_un address_local;
	address_local.sun_family = AF_LOCAL;
	strcpy(address_local.sun_path, local_path);
	
	if (bind(sock_net, (struct sockaddr *) &address_net, sizeof(struct sockaddr_in)) == -1) {
		perror("Cannot bind network socket");
		exit(-1);
	}
	
	if (bind(sock_local, (struct sockaddr *) &address_local, sizeof(struct sockaddr_un)) == -1) {
		perror("Cannot bind local socket");
		exit(-1);
	}
	
#ifndef CONF_DATAGRAM
	if (listen(sock_net, CONF_MAX_CONNECTIONS) != 0
			|| listen(sock_local, CONF_MAX_CONNECTIONS) != 0) {
		perror("Cannot listen");
		exit(-1);
	}
#endif
}

void cleanup(void) {
	printf("\rShutting down...\n");
	
	cancel_pinger();
	cancel_accept_threads();
	teardown_clients();
	teardown_socks();
	cancel_listener();
}

void sighandler(int sig) {
	exit(0);
}

static void print_help(char *program) {
	printf("Usage:\n");
	printf("\t%s <port number> <local socket path>\n", program);
}

int read_request(void);

int main(int argc, char **argv) {
#ifdef CONF_DATAGRAM
	printf("Server works on UDP\n");
#else
	printf("Server works on TCP\n");
#endif
	
	if (argc != 3) {
		print_help(argv[0]);
		return -1;
	}
	
	for (int i = 0; i < CONF_CLIENTS_MAX; ++i) {
		clients[i].cl_active = 0;
	}
	
	atexit(cleanup);
	signal(SIGINT, sighandler);
	signal(SIGTERM, sighandler);
	
	waiting_poll = epoll_create1(0);
	
	port = atoi(argv[1]);
	local_path = argv[2];
	// 107: a constant from manual, max path = 108
	if (strlen(local_path) > 107) {
		fprintf(stderr, "Local path too long");
		exit(-1);
	}
	
	setup_socks();
	setup_accept_threads();
	setup_listener();
	setup_pinger();
	
	print_prompt();
	
	while (1) {
		read_request();
		
		print_prompt();
	}
}

int send_request(int clid, struct request_t *req) {
	struct socket_message sm;
	sm.addr = clients[clid].addr;
	sm.length = 1 + sizeof(struct request_t);
	sm.buffer[0] = MSG_TYPE_REQUEST;
	memcpy(&sm.buffer[1], req, sizeof(struct request_t));
	
	return send_sm(clients[clid].cl_sock, &sm);
}

int read_request(void) {
	int request_id = current_request_id;
	
	char *line = NULL;
	size_t len = 0;
	if (getline(&line, &len, stdin) <= 0) {
		// Ctrl-D
		printf("\rTo close the server use Ctrl-C\n");
		return 1;
	}
	
	// skip space
	while (*line == ' ')
		++line;
	
	if (*line == '\n' || *line == 0) {
		// empty line
		return 1;
	}
	
	struct request_t req;
	req.id = request_id;
	if (parse_request(line, &req) != 0) {
		printf("error: invalid data\n");
		return 1;
	}
	
	int clid = next_free_client();
	if (clid < 0) {
		printf("error: no available clients\n");
		return 1;
	}
	
	if (send_request(clid, &req) != 0) {
		perror("");
		printf("error: failed to send request\n");
		return 1;
	}
	
	++current_request_id;
	return 0;
}
