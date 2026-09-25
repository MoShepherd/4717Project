#include <zmq.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "common.h"

Matrix multiply(Matrix a, Matrix b) {
	assert(a.columns == b.rows);
	Matrix c = (Matrix) {
		.elems = (float *) malloc(a.rows * b.columns * sizeof(float)),
		.rows = a.rows,
		.columns = b.columns
	};
	for (int i = 0; i < a.rows; i++) {
		for (int j = 0; j < b.columns; j++) {
			float sum = 0;
			for (int k = 0; k < a.columns; k++) {
				sum += a.elems[i * a.columns + k] * b.elems[k * b.columns + j];
			}
			c.elems[i * c.columns + j] = sum;
		}
	}
	return c;
}

int main(){
	void *zmq_ctx = zmq_ctx_new();
	void *pull = zmq_socket(zmq_ctx, ZMQ_PULL);
	int rc = zmq_connect(pull, "tcp://head:4770");
	assert(rc == 0);
	void *push = zmq_socket(zmq_ctx, ZMQ_PUSH);
	rc = zmq_connect(push, "tcp://head:4771");
	
	printf("worker started\n");
	
	while (1) {
		char buf[256];
		int n = zmq_recv(pull, buf, 256, 0);
		assert(n != -1);
		
		char *b = buf;
		Result_Info info;
		b += read_result_info(b, &info);
		
		Matrix m1 = read_matrix(b, &n);
		b += n;
		Matrix m2 = read_matrix(b, &n);
		b += n;
				
		Matrix product = multiply(m1, m2);
		b = buf;
		b += write_result_info(b, info);
		b += write_matrix(b, product);
		zmq_send(push, buf, b - buf, 0);
	}
	
	zmq_close(push);
	zmq_ctx_destroy(zmq_ctx);
	return 0;
}