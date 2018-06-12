#ifndef SERVER_CLLIST_H_
#define SERVER_CLLIST_H_

#include <sys/socket.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <netinet/ip.h>

#define lock_clients() do { \
		zero_or_fail(pthread_mutex_lock(&clients_mx), "Cannot lock clients") \
	} while (0)
#define unlock_clients() do { \
		zero_or_fail(pthread_mutex_unlock(&clients_mx), "Cannot unlock clients") \
	} while (0)

#include "connection.h"

int client_freeid(void);
int client_for_addr(struct sockaddr *addr, socklen_t len);
int client_for_sock(int sock);
int next_free_client(void);

void teardown_clients(void);
int is_name_taken(const char *name);
void register_client(int sock, struct socket_message *sm);
void unregister_client(int client_id);


void setup_client_mutex();
void teardown_client_mutex();

#endif
