#include "server_cllist.h"

#include "server.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

pthread_mutex_t clients_mx;
struct client_t clients[CONF_CLIENTS_MAX];

void setup_client_mutex(){
	pthread_mutexattr_t attr;
	
	pthread_mutexattr_init(&attr);
	pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
	pthread_mutex_init(&clients_mx, &attr);
	pthread_mutexattr_destroy(&attr);
}

void teardown_client_mutex(){
	pthread_mutex_destroy(&clients_mx);
}

int client_freeid(void) {
	int ret = -1;
	lock_clients();
	
	for (int i = 0; i < CONF_CLIENTS_MAX; ++i) {
		if (clients[i].cl_active == 0) {
			ret = i;
			break;
		}
	}
	
	unlock_clients();
	return ret;
}

int client_for_addr(struct sockaddr *addr, socklen_t len) {
	int ret = -1;
	lock_clients();
	
	for (int i = 0; i < CONF_CLIENTS_MAX; ++i) {
		if (clients[i].cl_active != 0 && len == clients[i].addr.len
				&& memcmp(&clients[i].addr.in_addr, addr, len) == 0) {
			ret = i;
			break;
		}
	}
	
	unlock_clients();
	return ret;
}

int client_for_sock(int sock) {
	int ret = -1;
	lock_clients();
	
	for (int i = 0; i < CONF_CLIENTS_MAX; ++i) {
		if (clients[i].cl_active != 0 && sock == clients[i].cl_sock) {
			ret = i;
			break;
		}
	}
	
	unlock_clients();
	return ret;
}

int is_name_taken(const char *name) {
	int ret = 0;
	lock_clients();
	
	for (int i = 0; i < CONF_CLIENTS_MAX; ++i) {
		if (clients[i].cl_active && strcmp(clients[i].cl_name, name) == 0) {
			ret = 1;
			break;
		}
	}
	
	unlock_clients();
	return ret;
}

void register_client(int sock, struct socket_message *sm) {
	char *name = &sm->buffer[1];
	name[sm->length - 1] = 0;
	
	if (is_name_taken(name)) {
		printf("\rConnection refused to client: name '%s' taken\n", name);
		print_prompt();
		
		sm->length = 1;
		sm->buffer[0] = MSG_TYPE_NAMEINUSE;
		send_sm(sock, sm);
		return;
	}
	
	lock_clients();
	
	int id = client_freeid();
	if (id < 0) {
		fprintf(stderr, "Too many connections");
	} else {
		clients[id].cl_active = 1;
		clients[id].cl_sock = sock;
		clients[id].cl_inactivity = 0;
		clients[id].addr = sm->addr;
		strcpy(clients[id].cl_name, name);
		
#ifndef CONF_DATAGRAM
		// on TCP we need to listen
		add_to_poll(sock);
#endif
		
		printf("\rClient '%s' connected!\n", name);
		print_prompt();
	}
	
	unlock_clients();
}

void unregister_client(int client_id) {
	lock_clients();
	
	if (client_id >= 0) {
		clients[client_id].cl_active = 0;
		
#ifndef CONF_DATAGRAM
		// on UDP we do not close sockets as those are
		// server's sockets
		
		shutdown(clients[client_id].cl_sock, SHUT_RDWR);
		// sock is automatically removed from epoll upon closing
		close(clients[client_id].cl_sock);
#endif
		
		printf("\rClient '%s' disconnected\n", clients[client_id].cl_name);
		print_prompt();
	}
	
	unlock_clients();
}

void teardown_clients(void) {
	for (int i = 0; i < CONF_CLIENTS_MAX; ++i) {
		if (clients[i].cl_active) {
			clients[i].cl_active = 0;
			
			struct socket_message sm;
			sm.addr = clients[i].addr;
			sm.length = 1;
			sm.buffer[0] = MSG_TYPE_SHUTDOWN;
			send_sm(clients[i].cl_sock, &sm);
			
#ifndef CONF_DATAGRAM
			shutdown(clients[i].cl_sock, SHUT_RDWR);
			close(clients[i].cl_sock);
#endif
		}
	}
}

int free_cl = 0;

int next_free_client(void) {
	lock_clients();
	
	int from;
	for (from = free_cl; from != free_cl + CONF_CLIENTS_MAX; ++from) {
		if (clients[from % CONF_CLIENTS_MAX].cl_active) {
			break;
		}
	}
	
	if (from == free_cl + CONF_CLIENTS_MAX) {
		unlock_clients();
		return -1;
	}
	
	from %= CONF_CLIENTS_MAX;
	free_cl = from + 1;
	
	unlock_clients();
	return from;
}
