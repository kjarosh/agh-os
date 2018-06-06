#ifndef CONNECTION_H_
#define CONNECTION_H_

#include "config.h"

#include <sys/types.h>
#include <sys/un.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>

struct socket_message {
	struct addr_t addr;
	
	__uint16_t length;
	char buffer[DATAGRAM_SIZE];
};

int receive_sm(int sock, struct socket_message *sm);
int send_sm(int sock, struct socket_message *sm);

#endif
