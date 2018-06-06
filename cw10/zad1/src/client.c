#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>

#include <sys/socket.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>

#include "config.h"
#include "request.h"
#include "connection.h"

int server_sock;
struct addr_t server_addr;

static void sighandler(int sig) {
	exit(0);
}

static void cleanup(void) {
	if(server_sock < 0) return;
	
	printf("\rShutting down...\n");
	
	struct socket_message sm;
	sm.addr = server_addr;
	sm.length = 1;
	sm.buffer[0] = MSG_TYPE_UNREGISTER;
	send_sm(server_sock, &sm);
	
	shutdown(server_sock, SHUT_RDWR);
	close(server_sock);
}

static void print_help(char *program) {
	printf("Usage:\n");
	printf("\t%s <name> local <local path>\n", program);
	printf("\t%s <name> net <address>:<port>\n", program);
}

void register_me(char* name) {
	struct socket_message sm;
	
	sm.addr = server_addr;
	
	sm.length = strlen(name) + 1;
	sm.buffer[0] = MSG_TYPE_REGISTER;
	strncpy(&sm.buffer[1], name, strlen(name));
	
	if (send_sm(server_sock, &sm) != 0) {
		fprintf(stderr, "Cannot send client name\n");
		exit(-1);
	}
}

void receive_request(void);

int main(int argc, char **argv) {
#ifdef CONF_DATAGRAM
	printf("Client works on UDP\n");
#else
	printf("Client works on TCP\n");
#endif
	
	if (argc != 4) {
		print_help(argv[0]);
		return -1;
	}
	
	atexit(cleanup);
	signal(SIGINT, sighandler);
	signal(SIGTERM, sighandler);
	
	char name[CONF_CLIENT_NAME_MAX];
	strcpy(name, argv[1]);
	
	if (strcmp(argv[2], "local") == 0) {
		if ((server_sock = socket(AF_LOCAL, CONF_SOCK_TYPE, 0)) == -1) {
			perror("Cannot create network socket");
			exit(-1);
		}
		
		char *local_path = argv[3];
		printf("Connecting to local '%s'\n", local_path);
		
		server_addr.len = sizeof(struct sockaddr_un);
		
		struct sockaddr_un *addr = &server_addr.un_addr;
		addr->sun_family = AF_LOCAL;
		strcpy(addr->sun_path, local_path);
	} else if (strcmp(argv[2], "net") == 0) {
		if ((server_sock = socket(AF_INET, CONF_SOCK_TYPE, 0)) == -1) {
			perror("Cannot create network socket");
			exit(-1);
		}
		
		char *addrname = strtok(argv[3], ":");
		int port = atoi(strtok(NULL, ""));
		
		server_addr.len = sizeof(struct sockaddr_in);
		
		struct sockaddr_in *addr = &server_addr.in_addr;
		addr->sin_family = AF_INET;
		addr->sin_port = htons(port);
		addr->sin_addr.s_addr = inet_addr(addrname);
		
		printf("Connecting to %s:%d\n", addrname, port);
	} else {
		fprintf(stderr, "Invalid connection type: %s\n", argv[2]);
		exit(-1);
	}
	
	if (connect(server_sock, &server_addr.saddr, server_addr.len) != 0) {
		perror("Cannot connect");
		exit(-1);
	}
	
	register_me(name);
	
	printf("Registered\n");
	
	while (1) {
		receive_request();
	}
}

void receive_request(void) {
	struct socket_message sm;
	if (receive_sm(server_sock, &sm) != 0) {
		if(errno == 0){
			// socket closed
			printf("Server closed\n");
			exit(0);
		}
		
		return;
	}
	
	char msg_type = sm.buffer[0];
	switch (msg_type) {
	case MSG_TYPE_PING:
		sm.length = 1;
		sm.buffer[0] = MSG_TYPE_PONG;
		break;
		
	case MSG_TYPE_REQUEST:
		; // statement
		struct request_t req;
		struct response_t resp;
		
		if (sm.length != 1 + sizeof(struct request_t)) {
			printf("Ill-formed message\n");
			return;
		}
		
		memcpy(&req, &sm.buffer[1], sizeof(struct request_t));
		if (process_request(&req, &resp) != 0) {
			printf("Unknown operator, ignoring\n");
			return;
		}
		
		sm.length = 1 + sizeof(struct response_t);
		sm.buffer[0] = MSG_TYPE_RESPONSE;
		memcpy(&sm.buffer[1], &resp, sizeof(struct response_t));
		printf("Received request, processed\n");
		break;
		
	case MSG_TYPE_NAMEINUSE:
		printf("Error: name in use\n");
		exit(0);
		
	default:
		printf("Unknown message type: %d\n", (int)msg_type);
		return;
	}
	
	send_sm(server_sock, &sm);
}
