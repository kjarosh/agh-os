#include "server_pinger.h"

#include "server.h"
#include "connection.h"

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

pthread_t pinger;

void *thread_pinger(void *args) {
	while (1) {
		pthread_testcancel();
		
		for (int i = 0; i < CONF_CLIENTS_MAX; ++i) {
			if (!clients[i].cl_active) continue;
			
			if (clients[i].cl_inactivity > CONF_INACTIVITY_MAX) {
				// client is inactive
				printf("\rClient '%s' timed out\n", clients[i].cl_name);
				print_prompt();
				clients[i].cl_active = 0;
				
#ifndef CONF_DATAGRAM
				shutdown(clients[i].cl_sock, SHUT_RDWR);
				close(clients[i].cl_sock);
#endif
			} else {
				clients[i].cl_inactivity++;
				
				// ping
				
				struct socket_message sm;
				sm.addr = clients[i].addr;
				sm.length = 1;
				sm.buffer[0] = MSG_TYPE_PING;
				send_sm(clients[i].cl_sock, &sm);
			}
		}
		
		sleep(CONF_PING_INTERVAL);
	}
}

void setup_pinger(void) {
	if (pthread_create(&pinger, NULL, thread_pinger, NULL) != 0) {
		perror("Cannot create pinger thread");
		exit(-1);
	}
}

void cancel_pinger(void) {
	pthread_cancel(pinger);
}
