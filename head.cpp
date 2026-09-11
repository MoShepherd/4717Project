#include <array>

struct Matrix {
	std::array<float> elems;
	int rows;
	int columns;
};

// algorithm: https://en.wikipedia.org/wiki/Matrix_multiplication_algorithm#Non-square_matrices
Matrix multiply(Matrix a, Matrix b) {
	if (a.rows > b.columns && a.rows > b.rows) {
		// split A vertically
	} else if (b.rows >= a.rows && b.rows > b.columns) {
		// split B horizontally
	} else {
		// split A and B
	}
}

Matrix multiply_impl(Matrix a, Matrix b) {
	Matrix c = Matrix {
		.elems = std::array<float, a.rows * b.columns>{},
		.rows = a.rows,
		.columns = b.columns,
	}
	for (int i = 0; i < a.rows; i++) {
		for (int j = 0; j < b.columns; j++) {
			float sum = 0;
			for (int k = 0; k < a.columns; k++) {
				sum += a.elems[i * a.columns + k] * b[k * b.columns + j]
			}
		}
	}
	return c
}

int main() {
	
}