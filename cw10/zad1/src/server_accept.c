#include "server_accept.h"

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <sys/time.h>

#include "config.h"
#include "server.h"
#include "connection.h"

pthread_t thread_net;
pthread_t thread_local;

void setup_accept_threads(void) {
#ifndef CONF_DATAGRAM
	if (pthread_create(&thread_net, NULL, thread_accept, &sock_net) != 0) {
		perror("Cannot create accepter thread");
		exit(-1);
	}

	if (pthread_create(&thread_local, NULL, thread_accept, &sock_local) != 0) {
		perror("Cannot create accepter thread");
		exit(-1);
	}
#endif
}

void cancel_accept_threads(void) {
#ifndef CONF_DATAGRAM
	pthread_cancel(thread_net);
	pthread_cancel(thread_local);
	
	struct timespec ts;
	ts.tv_sec = 1;
	ts.tv_nsec = 0;
	pthread_timedjoin_np(thread_net, NULL, &ts);
	pthread_timedjoin_np(thread_local, NULL, &ts);
#endif
}

static void set_timeout(int sock) {
	struct timeval tv;
	// timeout = 5 sec
	tv.tv_sec = 5;
	tv.tv_usec = 0;
	setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (const char*) &tv, sizeof tv);
}

void *thread_accept(void *args) {
	int server_sock = *(int*) args;
	while (1) {
		pthread_testcancel();
		
		struct addr_t addr;
		int sock;
		
		addr.len = MAX_SOCKADDR_SIZE;
		if ((sock = accept(server_sock, &addr.saddr, &addr.len)) == -1) {
			perror("Cannot accept");
			continue;
		}
		
		set_timeout(sock);
		
		struct socket_message sm;
		if (receive_sm(sock, &sm) != 0) continue;
		
		if (sm.buffer[0] != MSG_TYPE_REGISTER) {
			// ignore invalid message
			fprintf(stderr, "Invalid register message type");
			continue;
		}
		
		sm.addr = addr;
		register_client(sock, &sm);
	}
}
