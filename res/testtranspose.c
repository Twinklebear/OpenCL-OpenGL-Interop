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

	for (int i = 0; i < (SIZE / BLOCK_SIZE * (SIZE / BLOCK_SIZE + 1)) / 2; ++i)
		transposeBlockN(matrix, i);

	printf("Transposed matrix:\n");
	printMatrix(matrix);

	free(matrix);
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
	else if (i == j)
		swap(&mat[i + 1 + j * SIZE], &mat[i + (j + 1) * SIZE]);
	//Off-diagonal blocks swap with the corresponding block on the lower-triangle
	else {
		//Setup the destination pointer
		float *dst = mat + i + j * SIZE;
		printf("swapping with block containing:\n");

		//Need to store 2 blocks of floats
		float *tmp = malloc(sizeof(float) * 2 * BLOCK_SIZE * BLOCK_SIZE);
		//Store the src block
		float *src = mat + i * SIZE + j;
		tmp[0] = src[0];
		tmp[1] = src[1];
		tmp[2] = src[SIZE];
		tmp[3] = src[SIZE + 1];
		printf("src block vals: %.2f, %.2f, %.2f, %.2f\n", src[0],
			src[1], src[SIZE], src[SIZE + 1]);
		//Store dst block
		tmp[4] = dst[0];
		tmp[5] = dst[1];
		tmp[6] = dst[SIZE];
		tmp[7] = dst[SIZE + 1];
		printf("dst block vals: %.2f, %.2f, %.2f, %.2f\n", dst[0],
			dst[1], dst[SIZE], dst[SIZE + 1]);

		//Perform the swap
		src[0] = tmp[4];
		src[SIZE] = tmp[5];
		src[1] = tmp[6];
		src[SIZE + 1] = tmp[7];
		dst[0] = tmp[0];
		dst[SIZE] = tmp[1];
		dst[1] = tmp[2];
		dst[SIZE + 1] = tmp[3];

		free(tmp);
	}
}
void transposeBlockN(float *mat, int n){
	printf("\nFinding i,j for transposeblock # %d\n", n);

	int blockPerRow = SIZE / BLOCK_SIZE;
	int row = 0, col = 0;
	//The index of the block we want to read should be offset
	//by the # of blocks to the diagonal, ie if we're reading
	//transpose block 2 in a 4x4 matrix [0-2] we should read 
	//matrix block # 3
	//Determine the matrix block number from the transpose block number
	//and also find the row
	int seenBlocks = blockPerRow;
	int tBlockPerRow = blockPerRow;
	while (n >= seenBlocks){
		seenBlocks += blockPerRow;
		--tBlockPerRow;
		row += BLOCK_SIZE;
		n += blockPerRow - tBlockPerRow;
	}
	//Find the column
	//n % tBlockPerRow * BLOCK_SIZE gives us the column relative to the diagonal
	//block, which we then offset by the offset # of blocks
	// (blockPerRow - tBlockPerRow) * BLOCK_SIZE
	col = n % tBlockPerRow * BLOCK_SIZE + (blockPerRow - tBlockPerRow) * BLOCK_SIZE;

	printf("\tmatrix block #: %d, row: %d, col: %d\n", n, row, col);
	int idx = row * SIZE + col;
	printf("\t0, 0 of source location idx: %d\n", idx);
	//Move float ptr to source block
	float *src = mat + idx;
	printf("\tvalues:\n");
	for (int i = 0; i < BLOCK_SIZE; ++i){
		for (int j = 0; j < BLOCK_SIZE; ++j)
			printf("\t%*.2f", 5, src[i * SIZE + j]);
		printf("\n");
	}

	transposeBlock(mat, row, col);
}
