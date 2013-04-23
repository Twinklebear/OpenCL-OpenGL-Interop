//We're using float4 in this kernel to store blocks, so block size is 4
#define BLOCK_SIZE 2

//This kernel will transpose matrices that are size 4nx4n (or will it only work on 8nx8n?)
//gMat is the global matrix, lMat is the local matrix that we use to store a copy of the 
//non-transposed matrix to then write the transposed values back to global
//Should I make size, block size and block per row __constants?
__kernel void transpose(__global float4 *gMat, __local float4 *lMat, uint size){
	//Source and destination pointers, locate where we
	//want to read and write our data from
	__global float4 *src, *dst;
	//Compute row/column location
	int col = get_global_id(0);
	int row = 0;
	int blockPerRow = size / BLOCK_SIZE;
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
}
	