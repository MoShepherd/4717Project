#include <zmq.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "common.h"

#define MAX(i, j) (((i) > (j)) ? (i) : (j))
// algorithm: https://en.wikipedia.org/wiki/Matrix_multiplication_algorithm#Non-square_matrices

#define THRESHOLD 10

char send_buffer[BUFFER_SIZE];
char recv_buffer[BUFFER_SIZE];

void *push;
void *pull;

void split_horizontal(Matrix m, Matrix *top, Matrix *bottom) {
	top->rows = m.rows / 2;
	top->columns = m.columns;
	top->elems = (float *) calloc(top->rows * top->columns, sizeof(float));
	bottom->rows = m.rows - top->rows;
	bottom->columns = m.columns;
	bottom->elems = (float *) calloc(bottom->rows * bottom->columns, sizeof(float));
	
	for (int r = 0; r < m.rows; r++) {
		for (int c = 0; c < m.columns; c++) {
			if (r < top->rows) {
				top->elems[r * top->columns + c] = m.elems[r * m.columns + c];
			} else {
				bottom->elems[(r - top->rows) * bottom->columns + c] = m.elems[r * m.columns + c];
			}
		}
	}
}

void split_vertical(Matrix m, Matrix *left, Matrix *right) {
	left->rows = m.rows;
	left->columns = m.columns / 2;
	left->elems = (float *) calloc(left->rows * left->columns, sizeof(float));
	right->rows = m.rows;
	right->columns = m.columns - left->columns;
	right->elems = (float *) calloc(right->rows * right->columns, sizeof(float));
	
	for (int r = 0; r < m.rows; r++) {
		for (int c = 0; c < m.columns; c++) {
			if (c < left->columns) {
				left->elems[r * left->columns + c] = m.elems[r * m.columns + c];
			} else {
				right->elems[r * right->columns + c - left->columns] = m.elems[r * m.columns + c];
			}
		}
	}
}

int multiply_rec(Matrix a, Matrix b, int threshold, Result_Info ri) {
	
    // Check condition for matrix multiplication
    if(!(a.columns == b.rows)){
        printf("Violate condition for matrix multiplication");
        return -1;
    }

    // Recursion Head
	int largest_dimension = MAX(a.rows, MAX(a.columns, b.columns));
    
    if(largest_dimension <= threshold){
		char *buf = send_buffer;
		buf += write_result_info(buf, ri);
		buf += write_matrix(buf, a);
		buf += write_matrix(buf, b);
		zmq_send(push, send_buffer, buf - send_buffer, 0);
		return 1;
    }

    // Recursion Body

    if (largest_dimension == a.columns) {
        Matrix btop, bbottom;
		split_horizontal(b, &btop, &bbottom);

		Matrix aleft, aright;
        split_vertical(a, &aleft, &aright);

        // Next recursion step
        int messages = multiply_rec(aleft, btop, threshold, ri);
        messages += multiply_rec(aright, bbottom, threshold, ri);
		free(btop.elems);
		free(bbottom.elems);
		free(aleft.elems);
		free(aright.elems);
		return messages;
	} else if (largest_dimension == a.rows) {
		Matrix atop, abottom;
		split_horizontal(a, &atop, &abottom);
		
		Result_Info new_ri = ri;
		new_ri.row += atop.rows;
		
        // Next Recursion Step
        int messages = multiply_rec(atop, b, threshold, ri);
        messages += multiply_rec(abottom, b, threshold, new_ri);
		free(atop.elems);
		free(abottom.elems);
		return messages;
	} else {
		Matrix bleft, bright;
		split_vertical(b, &bleft, &bright);
		       
		Result_Info new_ri = ri;
		new_ri.column += bleft.columns;
		
        // Next recursion step
        int messages = multiply_rec(a, bleft, threshold, ri);
        messages += multiply_rec(a, bright, threshold, new_ri);
		free(bleft.elems);
		free(bright.elems);
		return messages;
	}
}

Matrix multiply(Matrix a, Matrix b) {
	Matrix result = {
		.rows = a.rows,
		.columns = b.columns,
		.elems = (float *) calloc(a.rows * b.columns, sizeof(float))
	};
	
	int messages = multiply_rec(a, b, THRESHOLD, (Result_Info) { 0, 0 });
	while (messages) {
		zmq_recv(pull, recv_buffer, BUFFER_SIZE, 0);
		Result_Info info;
		char *buf = recv_buffer;
		buf += read_result_info(buf, &info);
		Matrix product = read_matrix(buf, NULL);
		
		for (int r = 0; r < product.rows; r++) {
			for (int c = 0; c < product.columns; c++) {
				result.elems[(r + info.row) * result.columns + c + info.column] += product.elems[r * product.columns + c];
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

Matrix random_matrix(int rows, int columns) {
	Matrix m = (Matrix) {
		.elems = (float *) calloc(rows * columns, sizeof(float)),
		.rows = rows,
		.columns = columns,
	};
	for (int i = 0; i < rows * columns; i++) {
		m.elems[i] = (float) ( rand() % 10 );
	}
	return m;
}

int main() {
	void *zmq_ctx = zmq_ctx_new();
	push = zmq_socket(zmq_ctx, ZMQ_PUSH);
	int high_water_mark = 0;
	zmq_setsockopt(push, ZMQ_SNDHWM, &high_water_mark, 4);
	int rc = zmq_bind(push, "tcp://*:4770");
	assert(rc == 0);
	pull = zmq_socket(zmq_ctx, ZMQ_PULL);
	zmq_setsockopt(pull, ZMQ_RCVHWM, &high_water_mark, 4);
	rc = zmq_bind(pull, "tcp://*:4771");
	assert(rc == 0);
	
	printf("head started\n");
	
	srand(4770);
	
	Matrix m1 = random_matrix(1000, 1000);
	Matrix m2 = random_matrix(1000, 1000);
	
	Matrix result = multiply(m1, m2);
	Matrix reference = multiply_impl(m1, m2);

	bool do_match = false;
	if (result.rows == reference.rows && result.columns == reference.columns) {
		do_match = true;
		for (int i = 0; i < reference.rows * reference.columns; i++) {
			if (reference.elems[i] != result.elems[i]) {
				do_match = false;
			}
		}
	}
	if (do_match) {
		printf("multiplication successful");
	} else {
		printf("multiplication unsuccessful\n");
		printf("reference:\n");
		print_matrix(reference);
		printf("result:\n");
		print_matrix(result);
	}
	fflush(stdout);
	
	zmq_close(push);
	zmq_close(pull);
	zmq_ctx_destroy(zmq_ctx);
	return 0;
}