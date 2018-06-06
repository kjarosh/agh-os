#ifndef CONFIG_H_
#define CONFIG_H_

#include <sys/types.h>
#include <sys/un.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>

#define CONF_INACTIVITY_MAX 5
// 2 seconds
#define CONF_PING_INTERVAL 2

#define CONF_CLIENTS_MAX 16
#define CONF_CLIENT_NAME_MAX 256
#define CONF_MAX_CONNECTIONS 10

#define DATAGRAM_SIZE 256

#define MSG_TYPE_REGISTER 0
#define MSG_TYPE_UNREGISTER 1
#define MSG_TYPE_PING 2
#define MSG_TYPE_PONG 3
#define MSG_TYPE_NAMEINUSE 4
#define MSG_TYPE_RESPONSE 5
#define MSG_TYPE_REQUEST 6

#ifdef CONF_DATAGRAM
#define CONF_SOCK_TYPE SOCK_DGRAM
#else
#define CONF_SOCK_TYPE SOCK_STREAM
#endif

struct addr_t {
	socklen_t len;
	union {
		struct sockaddr saddr;
		struct sockaddr_un un_addr;
		struct sockaddr_in in_addr;
	};
};

#define zero_or_fail(f, err) \
	if((f) != 0){ \
		perror(err); \
		exit(-1); \
	}

#define max(a, b) ((a) > (b) ? (a) : (b))
#define MAX_SOCKADDR_SIZE (max(sizeof(struct sockaddr_in), sizeof(struct sockaddr_un)))

#endif
