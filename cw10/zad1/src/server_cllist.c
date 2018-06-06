#include "server_cllist.h"

#include "server.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

pthread_mutex_t clients_mx = PTHREAD_MUTEX_INITIALIZER;
struct client_t clients[CONF_CLIENTS_MAX];

int client_freeid(void) {
	for (int i = 0; i < CONF_CLIENTS_MAX; ++i) {
		if (clients[i].cl_active == 0) {
			return i;
		}
	}
	
	return -1;
}

int client_for_addr(struct sockaddr *addr, socklen_t len) {
	for (int i = 0; i < CONF_CLIENTS_MAX; ++i) {
		if (clients[i].cl_active != 0 && len == clients[i].addr.len
				&& memcmp(&clients[i].addr.in_addr, addr, len) == 0) {
			return i;
		}
	}
	
	return -1;
}

int client_for_sock(int sock) {
	for (int i = 0; i < CONF_CLIENTS_MAX; ++i) {
		if (clients[i].cl_active != 0 && sock == clients[i].cl_sock) {
			return i;
		}
	}
	
	return -1;
}

int is_name_taken(const char *name) {
	for (int i = 0; i < CONF_CLIENTS_MAX; ++i) {
		if (clients[i].cl_active && strcmp(clients[i].cl_name, name) == 0) {
			return 1;
		}
	}
	
	return 0;
}

void register_client(int sock, struct socket_message *sm) {
	char *name = &sm->buffer[1];
	name[sm->length - 1] = 0;
	
	if (is_name_taken(name)) {
		fprintf(stderr, "Connection refused to client: name '%s' taken\n", name);
		
		sm->length = 1;
		sm->buffer[0] = MSG_TYPE_NAMEINUSE;
		send_sm(sock, sm);
		return;
	}
	
	zero_or_fail(pthread_mutex_lock(&clients_mx), "Cannot lock clients");
	
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
	
	zero_or_fail(pthread_mutex_unlock(&clients_mx), "Cannot unlock clients");
}

void unregister_client(int client_id) {
	zero_or_fail(pthread_mutex_lock(&clients_mx), "Cannot lock clients");
	
	if (client_id >= 0) {
		clients[client_id].cl_active = 0;
		
		shutdown(clients[client_id].cl_sock, SHUT_RDWR);
		// sock is automatically removed from epoll upon closing
		close(clients[client_id].cl_sock);
		
		printf("\rClient '%s' disconnected\n", clients[client_id].cl_name);
		print_prompt();
	} else {
		printf("Received unregister from an unknown client\n");
	}
	
	zero_or_fail(pthread_mutex_unlock(&clients_mx), "Cannot unlock clients");
}

void teardown_clients(void) {
	for (int i = 0; i < CONF_CLIENTS_MAX; ++i) {
		if (clients[i].cl_active) {
			clients[i].cl_active = 0;
			shutdown(clients[i].cl_sock, SHUT_RDWR);
			close(clients[i].cl_sock);
		}
	}
}

int free_cl = 0;

int next_free_client(void) {
	int from;
	for(from = free_cl; from != free_cl + CONF_CLIENTS_MAX; ++from){
		if(clients[from % CONF_CLIENTS_MAX].cl_active){
			break;
		}
	}
	
	if(from == free_cl + CONF_CLIENTS_MAX) return -1;
	
	from %= CONF_CLIENTS_MAX;
	
	free_cl = from + 1;
	return from;
}
