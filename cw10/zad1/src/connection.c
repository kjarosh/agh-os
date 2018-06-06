#include "connection.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <endian.h>

int receive_sm(int sock, struct socket_message *sm) {
#ifdef CONF_DATAGRAM
	int is_datagram = 1;
#else
	int is_datagram = 0;
#endif
	
	sm->addr.len = MAX_SOCKADDR_SIZE;
	
	if (is_datagram) {
		char buffer[DATAGRAM_SIZE];
		if (recvfrom(sock, &buffer[0], DATAGRAM_SIZE, 0, &sm->addr.saddr, &sm->addr.len) <= 0) {
			return -1;
		}
		
		memcpy(&sm->length, &buffer[0], 2);
		memcpy(&sm->buffer[0], &buffer[2], DATAGRAM_SIZE - 2);
		
		return 0;
	} else {
		// receive length
		u_int16_t length;
		if (recv(sock, &length, 2, MSG_WAITALL) != 2) {
			return -1;
		}
		
		sm->length = length;
		
		if (recvfrom(sock, &sm->buffer, length, MSG_WAITALL, &sm->addr.saddr, &sm->addr.len)
				!= length) {
			return -1;
		}
		
		return 0;
	}
}

int send_sm(int sock, struct socket_message *sm) {
#ifdef CONF_DATAGRAM
	int is_datagram = 1;
#else
	int is_datagram = 0;
#endif
	
	if (is_datagram) {
		char buffer[DATAGRAM_SIZE];
		memcpy(&buffer[0], &sm->length, 2);
		memcpy(&buffer[2], &sm->buffer[0], DATAGRAM_SIZE - 2);
		
		if (sendto(sock, &buffer[0], DATAGRAM_SIZE, 0, &sm->addr.saddr,
				sm->addr.len) != DATAGRAM_SIZE) {
			return -1;
		}
		
		return 0;
	} else {
		// send length
		u_int16_t length = sm->length;
		if (send(sock, &length, 2, 0) != 2) {
			return -1;
		}
		
		if (send(sock, &sm->buffer, length, 0) != length) {
			return -1;
		}
		
		return 0;
	}
}
