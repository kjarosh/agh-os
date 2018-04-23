#ifndef CONFIG_H
#define CONFIG_H

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <signal.h>

#include <sys/types.h>
#include <pwd.h>

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#define QUEUE_NAME "/oslab-server"

enum message_t {
	HANDSHAKE = 1, MIRROR = 2, CALC = 3, TIME = 4, END = 5
};

#define MSG_T_SIZE (sizeof(struct msg_t) - sizeof(long))
#define MSG_BUF_SIZE 256

struct msg_t {
	long type;
	long client_id;
	pid_t from;
	union {
		char buf[MSG_BUF_SIZE];

#ifndef POSIXQ
		key_t queue_key;
#endif
	};
};

char *home_dir;

static void setup_home(void) {
	if ((home_dir = getenv("HOME")) == NULL) {
		home_dir = getpwuid(getuid())->pw_dir;
	}
}

#endif