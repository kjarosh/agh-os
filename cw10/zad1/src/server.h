#ifndef SERVER_H_
#define SERVER_H_

#include "config.h"
#include "connection.h"

#include <stdlib.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/un.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>

extern int sock_net;
extern int sock_local;
extern int waiting_poll;

struct client_t {
	struct addr_t addr;

	int cl_active;
	int cl_sock;
	char cl_name[CONF_CLIENT_NAME_MAX];

	int cl_inactivity;
};

extern pthread_mutex_t clients_mx;

extern struct client_t clients[CONF_CLIENTS_MAX];

int client_for_addr(struct sockaddr *addr, socklen_t len);
int client_for_sock(int sock);
void add_to_poll(int sock);

void register_client(int sock, struct socket_message *sm);
void unregister_client(int client_id);

void print_prompt();

#endif
