#ifndef COMMON_H
#define COMMON_H

#define BUFFER_SIZE 1024

typedef struct Matrix {
	float *elems;
	int rows;
	int columns;
} Matrix;

typedef struct Result_Info {
	int row;
	int column;
} Result_Info;

void write_int(char *buf, int i) {
	buf[0] = i % 256;
	buf[1] = (i >> 8) % 256;
	buf[2] = (i >> 16) % 256;
	buf[3] = (i >> 24) % 256;
}

int read_int(char *buf) {
	return buf[0] + (buf[1] << 8) + (buf[2] << 16) + (buf[3] << 24);
}

int write_result_info(char *buf, Result_Info info) {
	memcpy(buf, &info, sizeof(Result_Info));
	return sizeof(Result_Info);
}

int read_result_info(char *buf, Result_Info *info) {
	memcpy(info, buf, sizeof(Result_Info));
	return sizeof(Result_Info);
}

int write_matrix(char *buf, Matrix m) {
	char *b = buf;
	write_int(b, m.rows);
	b += 4;
	write_int(b, m.columns);
	b += 4;
	int n = m.rows * m.columns * 4;
	memcpy(b, m.elems, n);
	b += n;
	return n + 8;
}

Matrix read_matrix(char *buf, int *n) {
	char *b = buf;
	int rows = read_int(b);
	b += 4;
	int columns = read_int(b);
	b += 4;
	int elems_size = rows * columns * 4;
	if (n != NULL) {
		*n = elems_size + 8;
	}
	float *elems = malloc(elems_size);
	memcpy(elems, b, elems_size);
	return (Matrix) {
		.elems = elems,
		.rows = rows,
		.columns = columns,
	};
}

void print_matrix(Matrix m) {
	printf("{ ");
	for (int r = 0; r < m.rows; r++) {
		for (int c = 0; c < m.columns; c++) {
			printf("%.0f ", m.elems[r * m.columns + c]);
		}
		if (r != m.rows - 1) {
			printf("\n  ");
		}
	}
	printf("}\n");
	fflush(stdout);
}

#endif