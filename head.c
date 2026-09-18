#include <zmq.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

typedef struct Matrix {
	float *elems;
	int rows;
	int columns;
} Matrix;

// algorithm: https://en.wikipedia.org/wiki/Matrix_multiplication_algorithm#Non-square_matrices
Matrix multiply(Matrix a, Matrix b, int threshold) {
    
    // Check condition for matrix multiplication
    if(!(a.columns == b.rows)){
        a.columns = -1;
        a.rows = -1;
        printf("Violate condition for matrix multiplication");
        return a;
    }

    // Recursion Head
    if(a.columns && a.rows && b.rows < threshold){
        // Call workers
    }
    
    // Recursion Body
    Matrix firstHorSplit, secondHorSplit, firstVerSplit, secondVerSplit;
	
    // Split A horizontal
    if (a.rows > b.columns && a.rows > b.rows) {
        firstHorSplit.rows = a.rows / 2;
        firstHorSplit.columns = a.columns;
        secondHorSplit.rows = a.rows - a.rows / 2;
        secondHorSplit.columns = a.columns;
        for(int i = 0; i < a.rows; i++){
            for(int j = 0; j < a.columns){
                if(i < a.rows/2){
                    firstHorSplit.elems[i + j] = a.elems[i + j];
                }
                else{
                    secondHorSplit.elems[i + j] = a.elems[i + j];
                }
            }
        }
        // Next Recursion Step
        multiply(firstHorSplit, b, threshold);
        multiply(secondHorSplit, b, threshold);

	} else if (b.rows >= a.rows && b.rows > b.columns) {
        // Split B vertical
        firstVerSplit.columns = b.columns / 2;
        firstVerSplit.rows = b.rows;
        secondVerSplit.columns = b.columns - b.columns / 2;
        secondVerSplit.rows = b.rows;
        for(int i = 0; i < b.rows; i++){
            for(int j = 0; j < b.columns){
                if(i < b.columns / 2){
                    firstVerSplit.elems[i + j] = b.elems[i + j];
                }
                else{
                    secondVerSplit.elems[i + j] = b.elems[i + j];
                }
            }
        }
        // Next recursion step
        multiply(a, firstHorSplit, threshold);
        multiply(b, secondHorSplit, threshold);
        
	} else {
        // Split B horizontal
        firstHorSplit.rows = b.rows / 2;
        firstHorSplit.columns = b.columns;
        secondHorSplit.rows = b.rows - b.rows / 2;
        secondHorSplit.columns = b.columns;
        for(int i = 0; i < b.rows; i++){
            for(int j = 0; j < b.columns){
                if(i < b.rows / 2){
                    firstHorSplit.elems[i + j] = b.elems[i + j];
                }
                else{
                    secondHorSplit.elems[i + j] = b.elems[i + j];
                }
            }
        }

        // Split A vertical
        firstVerSplit.columns = a.columns / 2;
        firstVerSplit.rows = a.rows;
        secondVerSplit.columns = a.columns - a.columns / 2;
        secondVerSplit.rows = a.rows; 
        for(int i = 0; i < a.rows; i++){
            for(int j = 0; j < a.columns){
                if(i < a.columns / 2){
                    firstVerSplit.elems[i + j] = a.elems[i + j];
                }
                else{
                    secondVerSplit.elems[i + j] = a.elems[i + j];
                }
            }
        }
        // Next recursion step
        multiply(firstVerSplit, firstHorSplit, threshold);
        multiply(firstVerSplit, secondHorSplit, threshold);
        multiply(secondVerSplit, firstHorSplit, threshold);
        multiply(secondVerSplit, secondHorSplit, threshold);
	}
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
	void *resp = zmq_socket(zmq_ctx, ZMQ_REQ);
	int response = zmq_bind(resp, "tcp://*:4770");
	assert(response == 0);

	return 0;
}