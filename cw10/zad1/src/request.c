#include "request.h"

#include <stdio.h>
#include <stdlib.h>

int current_request_id = 0;

int process_request(const struct request_t *req, struct response_t *resp) {
	resp->id = req->id;
	
	switch (req->type) {
	case ADDITION:
		resp->result = req->left + req->right;
		return 0;
	case SUBTRACTION:
		resp->result = req->left - req->right;
		return 0;
	case MULTIPLICATION:
		resp->result = req->left * req->right;
		return 0;
	case DIVISION:
		resp->result = req->left / req->right;
		return 0;
	default:
		return -1;
	}
}

int parse_request(char *data, struct request_t *req) {
	double left = strtod(data, &data);
	while(*data == ' ') ++data;
	
	char op = *(data++);
	while(*data == ' ') ++data;
	
	if(*data == '\n' || *data == 0){
		// no second operand
		return -1;
	}
	
	double right = strtod(data, &data);
	
	switch(op){
	case '+':
		req->type = ADDITION;
		break;
		
	case '-':
		req->type = SUBTRACTION;
		break;
		
	case '*':
		req->type = MULTIPLICATION;
		break;
		
	case '/':
		req->type = DIVISION;
		break;
		
	default:
		return -1;
	}
	
	req->left = left;
	req->right = right;
	return 0;
}
