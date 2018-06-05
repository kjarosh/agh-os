#ifndef CL_H_
#define CL_H_

enum comparison_t {
	GT, LT, EQ
};

struct cl {
	int producers;
	int consumers;
	int buffer_size;
	char filename[256];
	
	int line_length;
	enum comparison_t cmp;
	
	int time_limit;
	int verbose;
	int sleep;
};

int cl_initialize(struct cl *cl, int argc, char **argv);
int cl_shouldprint(struct cl *cl, char *line);

#endif
