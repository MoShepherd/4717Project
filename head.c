#include <zmq.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "common.h"

// algorithm: https://en.wikipedia.org/wiki/Matrix_multiplication_algorithm#Non-square_matrices
Matrix multiply(Matrix a, Matrix b) {
	if (a.rows > b.columns && a.rows > b.rows) {
		// split A vertically
	} else if (b.rows >= a.rows && b.rows > b.columns) {
		// split B horizontally
	} else {
		// split A and B
	}
	return (Matrix) {};
}

Matrix multiply_impl(Matrix a, Matrix b) {
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

int main() {
	void *zmq_ctx = zmq_ctx_new();
	void *push = zmq_socket(zmq_ctx, ZMQ_PUSH);
	int rc = zmq_bind(push, "tcp://*:4770");
	assert(rc == 0);
	void *pull = zmq_socket(zmq_ctx, ZMQ_PULL);
	rc = zmq_bind(pull, "tcp://*:4771");
	assert(rc == 0);
	
	printf("head started\n");
	
	float elems1[4] = {1, 2, 3, 4};
	float elems2[4] = {5, 6, 7, 8};
	Matrix m1 = (Matrix) {
		.elems = elems1,
		.rows = 2,
		.columns = 2
	};
	Matrix m2 = (Matrix) {
		.elems = elems2,
		.rows = 2,
		.columns = 2
	};
	char buf[256];
	memset(buf, 0, 256);
	char *b = buf;
	b += write_matrix(b, m1);
	b += write_matrix(b, m2);
	zmq_send(push, buf, b - buf, 0);
	
	memset(buf, 0, 256);
	zmq_recv(pull, buf, 256, 0);
	Matrix product = read_matrix(buf, NULL);
	print_matrix(product);
	
	zmq_close(push);
	zmq_close(pull);
	zmq_ctx_destroy(zmq_ctx);
	return 0;
}