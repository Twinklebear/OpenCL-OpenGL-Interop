#include <stdlib.h>
#include <stdio.h>
#define SIZE 4
#define BLOCK_SIZE 2

//Helper to print a SIZExSIZE matrix
void printMatrix(float *mat);
//Helper to log a SIZExSIZE matrix to some file
void logMatrix(float *mat, const char *fname);
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
	logMatrix(matrix, "init.txt");

	for (int i = 0; i < (SIZE / BLOCK_SIZE * (SIZE / BLOCK_SIZE + 1)) / 2; ++i)
		transposeBlockN(matrix, i);

	printf("Transposed matrix:\n");
	printMatrix(matrix);
	logMatrix(matrix, "out.txt");

	free(matrix);
	return 0;
}
void printMatrix(float *mat){
	for (int i = 0; i < SIZE * SIZE; ++i){
		if (i % SIZE == 0 && i != 0)
			printf("\n");
		printf(" %*.3f ", 6, mat[i]);
	}
	printf("\n");
}
void logMatrix(float *mat, const char *fname){
	FILE *fp = fopen(fname, "w");
	if (fp == NULL)
		printf("Failed to open %s!\n", fname);
	else {
		for (int i = 0; i < SIZE * SIZE; ++i){
			if (i % SIZE == 0 && i != 0)
				fprintf(fp, "\n");
			fprintf(fp, "%*.1f", 7, mat[i]);
		}
		fprintf(fp, "\n");
		fclose(fp);
	}
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
	else if (i == j){
		for (int l = 0; l < BLOCK_SIZE; ++l)
			for (int k = l + 1; k < BLOCK_SIZE; ++k)
				swap(&mat[i + k + (j + l) * SIZE], &mat[i + l + (j + k) * SIZE]);
	}
	//Off-diagonal blocks swap with the corresponding block on the lower-triangle
	else {
		//Need to store 2 blocks of floats
		float *tmp = malloc(sizeof(float) * 2 * BLOCK_SIZE * BLOCK_SIZE);
		//Setup dst and src pointers
		float *dst = mat + i + j * SIZE;
		float *src = mat + i * SIZE + j;
		//Read src block
		for (int k = 0; k < BLOCK_SIZE * BLOCK_SIZE; ++k)
			tmp[k] = src[k / BLOCK_SIZE * SIZE + k % BLOCK_SIZE];
		//Read dst block
		for (int k = 0; k < BLOCK_SIZE * BLOCK_SIZE; ++k)
			tmp[k + BLOCK_SIZE * BLOCK_SIZE] = dst[k / BLOCK_SIZE * SIZE + k % BLOCK_SIZE];

		//Perform the swap
		for (int k = 0; k < BLOCK_SIZE * BLOCK_SIZE; ++k){
			src[k / BLOCK_SIZE + k % BLOCK_SIZE * SIZE] = tmp[k + BLOCK_SIZE * BLOCK_SIZE];
			dst[k / BLOCK_SIZE + k % BLOCK_SIZE * SIZE] = tmp[k];
		}

		free(tmp);
	}
}
void transposeBlockN(float *mat, int n){
	int blockPerRow = SIZE / BLOCK_SIZE;
	int row = 0, col = n;
	//The index of the block we want to read should be offset
	//by the # of blocks to the diagonal, ie if we're reading
	//transpose block 2 in a 4x4 matrix [0-2] we should read 
	//matrix block # 3
	//Determine the matrix block number from the transpose block number
	//and also find the row
	int seenBlocks = blockPerRow;
	int tBlockPerRow = blockPerRow;
	while (col >= seenBlocks){
		seenBlocks += blockPerRow;
		--tBlockPerRow;
		row += BLOCK_SIZE;
		col += blockPerRow - tBlockPerRow;
	}
	//Find the column
	//n % tBlockPerRow * BLOCK_SIZE gives us the column relative to the diagonal
	//block, which we then offset by the offset # of blocks
	// (blockPerRow - tBlockPerRow) * BLOCK_SIZE
	//col comes out equal to the matrix block number
	col = col % tBlockPerRow * BLOCK_SIZE + (blockPerRow - tBlockPerRow) * BLOCK_SIZE;
	transposeBlock(mat, row, col);
}
