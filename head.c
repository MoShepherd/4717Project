#include <zmq.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "common.h"

#define MAX(i, j) (((i) > (j)) ? (i) : (j))
// algorithm: https://en.wikipedia.org/wiki/Matrix_multiplication_algorithm#Non-square_matrices

char send_buffer[256];

void *push;
void *pull;

void split_horizontal(Matrix m, Matrix *top, Matrix *bottom) {
	top->rows = m.rows / 2;
	top->columns = m.columns;
	top->elems = calloc(top->rows * top->columns, sizeof(float));
	bottom->rows = m.rows - top->rows;
	bottom->columns = m.columns;
	bottom->elems = calloc(bottom->rows * bottom->columns, sizeof(float));
	
	for (int r = 0; r < m.rows; r++) {
		for (int c = 0; c < m.columns; c++) {
			if (r < top->rows) {
				top->elems[r * top->rows + c] = m.elems[r * m.rows + c];
			} else {
				bottom->elems[(r - top->rows) * bottom->rows + c] = m.elems[r * m.rows + c];
			}
		}
	}
}

void split_vertical(Matrix m, Matrix *left, Matrix *right) {
	left->rows = m.rows;
	left->columns = m.columns / 2;
	left->elems = calloc(left->rows * left->columns, sizeof(float));
	right->rows = m.rows;
	right->columns = m.columns - left->columns;
	right->elems = calloc(right->rows * right->columns, sizeof(float));
	
	for (int r = 0; r < m.rows; r++) {
		for (int c = 0; c < m.columns; c++) {
			if (c < left->columns) {
				left->elems[r * left->rows + c] = m.elems[r * m.rows + c];
			} else {
				right->elems[r * right->rows + c - left->columns] = m.elems[r * m.rows + c];
			}
		}
	}
}

int multiply_rec(Matrix a, Matrix b, int threshold, Result_Info ri) {
    // Check condition for matrix multiplication
    if(!(a.columns == b.rows)){
        a.columns = -1;
        a.rows = -1;
        printf("Violate condition for matrix multiplication");
        return -1;
    }

    // Recursion Head
    if(a.columns <= threshold && a.rows <= threshold && b.columns <= threshold){
		char *buf = send_buffer;
		buf += write_result_info(buf, ri);
		buf += write_matrix(buf, a);
		buf += write_matrix(buf, b);
		zmq_send(push, send_buffer, buf - send_buffer, 0);
		printf("%d %d\n", ri.row, ri.column);
		print_matrix(a);
		print_matrix(b);
		
		return 1;
    }
    
	int largest_dimension = MAX(a.rows, MAX(a.columns, b.columns));

    // Recursion Body

    if (largest_dimension == a.columns) {
        Matrix btop, bbottom;
		split_horizontal(b, &btop, &bbottom);

		Matrix aleft, aright;
        split_vertical(a, &aleft, &aright);

        // Next recursion step
        int messages = multiply_rec(aleft, btop, threshold, ri);
        messages += multiply_rec(aright, bbottom, threshold, ri);
		return messages;
	} else if (largest_dimension == a.rows) {
		Matrix atop, abottom;
		split_horizontal(a, &atop, &abottom);
		
		Result_Info new_ri = ri;
		new_ri.row += atop.rows;
		
        // Next Recursion Step
        int messages = multiply_rec(atop, b, threshold, ri);
        messages += multiply_rec(abottom, b, threshold, new_ri);
		return messages;
	} else {
		Matrix bleft, bright;
		split_vertical(b, &bleft, &bright);
		       
		Result_Info new_ri = ri;
		new_ri.column += bleft.columns;
		
        // Next recursion step
        int messages = multiply_rec(a, bleft, threshold, ri);
        messages += multiply_rec(a, bright, threshold, new_ri);
		return messages;
	}
}

Matrix multiply(Matrix a, Matrix b) {
	Matrix result = {
		.rows = a.rows,
		.columns = b.columns,
		.elems = calloc(a.rows * b.columns, sizeof(float))
	};
	
	char recv_buffer[256];
	int messages = multiply_rec(a, b, 1, (Result_Info) { 0, 0 });
	while (messages) {
		zmq_recv(pull, recv_buffer, 256, 0);
		Result_Info info;
		char *buf = recv_buffer;
		buf += read_result_info(buf, &info);
		Matrix product = read_matrix(buf, NULL);
//		print_matrix(product);
		
		for (int r = 0; r < product.rows; r++) {
			for (int c = 0; c < product.columns; c++) {
				result.elems[(r + info.row) * result.columns + c + info.column] += product.elems[r * product.rows + c];
			}
		}
		
		messages--;
	}
	
	return result;
}

Matrix multiply_impl(Matrix a, Matrix b) {
	Matrix c = (Matrix) {
		.elems = (float *) calloc(a.rows * b.columns, sizeof(float)),
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
	push = zmq_socket(zmq_ctx, ZMQ_PUSH);
	int rc = zmq_bind(push, "tcp://*:4770");
	assert(rc == 0);
	pull = zmq_socket(zmq_ctx, ZMQ_PULL);
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
	
	print_matrix(multiply(m1, m2));
	
	zmq_close(push);
	zmq_close(pull);
	zmq_ctx_destroy(zmq_ctx);
	return 0;
}