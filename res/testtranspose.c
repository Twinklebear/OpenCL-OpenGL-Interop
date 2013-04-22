#include <stdlib.h>
#include <stdio.h>
#define SIZE 4
#define BLOCK_SIZE 2

//Helper to print a SIZExSIZE matrix
void printMatrix(float *mat);
//Helper to swap two floats
void swap(float *a, float *b);
//Transpose an entry in the matrix, n is the nth entry
void transpose(float *mat, int n);
//Transpose a 2x2 block of values in the matrix
//i, j denote the 0,0 element of the block
void transposeBlock(float *mat, int i, int j);
//Transpose a 2x2 block by determining the column/row we're moving
//from n, where n corresponds to the nth transpose block
void transposeBlockN(float *mat, int n);

int main(int argc, char **argv){
	float *matrix = malloc(sizeof(float) * SIZE * SIZE);
	for (int i = 0; i < SIZE * SIZE; ++i){
			matrix[i] = 1.0f * i;
	}
	printf("Initial matrix:\n");
	printMatrix(matrix);

	//Transpose the matrix
	//As it is now this will actually transpose the matrix then transpose it back
	for (int n = 0; n < SIZE * SIZE; ++n)
		transpose(matrix, n);
	// transposeBlock(matrix, 0, 0);
	// transposeBlock(matrix, 0, 2);
	// transposeBlock(matrix, 2, 2);

	printf("Transposed matrix:\n");
	printMatrix(matrix);

	// for (int i = 0; i < (SIZE / BLOCK_SIZE * (SIZE / BLOCK_SIZE + 1)) / 2; ++i)
	transposeBlockN(matrix, 0);
	transposeBlockN(matrix, 1);
	transposeBlockN(matrix, 2);

	return 0;
}
void printMatrix(float *mat){
	for (int i = 0; i < SIZE * SIZE; ++i){
		if (i % SIZE == 0 && i != 0)
			printf("\n");
		printf(" %*.2f ", 5, mat[i]);
	}
	printf("\n");
}
void swap(float *a, float *b){
	float tmp = *a;
	*a = *b;
	*b = tmp;
}
void transpose(float *mat, int n){
	if (n > SIZE * SIZE){
		printf("Invalid index: %d\n", n);
		return;
	}
	//calculate row (i) and column (j) in a row major representation
	int i = n % SIZE;
	int j = n / SIZE;
	// printf("i, j: %d, %d\n", i, j);
	// printf("n: %d, computed n: %d\n", n, i + j * SIZE);
	// printf("off diagonal compute of n: %d\n\n", i * SIZE + j);
	//diagonal elements don't need any swapping
	if (i == j)
		return;
	//off-diagonal: swap it with the j-i entry
	swap(&mat[i + j * SIZE], &mat[i * SIZE + j]);
}
void transposeBlock(float *mat, int i, int j){
	if (i > SIZE || j > SIZE)
		printf("Invalid i or j: %d, %d\n", i, j);
	//Blocks on diagonals we swap the off-diagonal elements
	else if (i == j);
		// swap(&mat[i][j + 1], &mat[i + 1][j]);
	//Off-diagonal blocks swap with the corresponding block on the lower-triangle
	else {
		for (int m = 0; m < 2; ++m)
			for (int n = 0; n < 2; ++n);
				// swap(&mat[i + n][j + m], &mat[j + m][i + n]);
	}
}
void transposeBlockN(float *mat, int n){
	printf("\nFinding i,j for block # %d\n", n);

	int size = SIZE;
	int col = n;
	int row = 0;
	while(col >= size) {
		col -= size--;
		row++;
	}
	col += row;
	size += row;

	printf("block %d:\n\ti, j: %d, %d\n", n, row, col);
	printf("\tsource location at n = %d\n", row * size * BLOCK_SIZE + col);
	//Move float ptr to source block
   	float *src = mat + row * size * BLOCK_SIZE + col;
	printf("\tvalues:\n");
	for (int i = 0; i < BLOCK_SIZE; ++i)
		printf("\t%*.2f %*.2f\n", 5, src[i * size + 0], 5, src[i * size + 1]);
	printf("\n");
	/*
	* on 4x4 they should be
	* 0: 0, 0
	* 1: 0, 2
	* 2: 2, 2
	*/
	/*
	* on the 6x6 block pos results SHOULD be
	* 0: 0, 0
	* 1: 0, 2
	* 2: 0, 4
	* 3: 2, 2
	* 4: 2, 4
	* 5: 4, 4
	*/
}
/*
* + 0 1 2 3
* 0 a b c d
* 1 e f g h
* 2 i j k l
* 3 m n o p
*/