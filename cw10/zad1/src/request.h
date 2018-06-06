#ifndef REQUEST_H_
#define REQUEST_H_

extern int current_request_id;

enum request_type {
	ADDITION, SUBTRACTION, MULTIPLICATION, DIVISION
};

struct request_t {
	int id;
	enum request_type type;
	double left;
	double right;
};

struct response_t {
	int id;
	double result;
};

int process_request(const struct request_t *req, struct response_t *resp);
int parse_request(char *data, struct request_t *req);

#endif
