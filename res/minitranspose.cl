//Trying to figure out why the OpenCL in Action transpose isn't working right
__kernel void transpose(__global float2 *gMat, __local float2 *lMat, uint size){
	//We'll be transposing a 4x4 matrix by reading in blocks of 2x2 matrices from the
	//matrix and performing the appropriate swap
	//ie. if we read in a block on a diagonal we swap the off diagonals, if we read
	//in an off-diagonal block we swap it with the corresponding block on the other side
	//this is an in-place transpose, so we only look at the upper corner
	__global float2 *src, *dst;
	int col = get_global_id(0);
	int row = 0;
}

/*
* a b c d
* e f g h
* i j k l
* m n o p
*/