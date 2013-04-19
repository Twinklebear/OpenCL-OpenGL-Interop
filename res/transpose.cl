//This kernel will transpose matrices that are size 4nx4n (or will it only work on 8nx8n?)
//gMat is the global matrix, lMat is the local matrix that we use to store a copy of the 
//non-transposed matrix to then write the transposed values back to global
__kernel void transpose(__global float4 *gMat, __local float4 *lMat, uint size){
	//Source and destination pointers, locate where we
	//want to read and write our data from
	__global float4 *src, dst;
	//Compute row/column location
	int col = get_global_id(0);
	int row = 0;
	//col >= size vs. col > size + 1?
	while (col > size + 1){
		col -= size;
		--size;
		++row;
	}
	col += row;
	size += row;

	//Move our src pointer to where we'll the 4x4 block we're going to operate on
	src = gMat + row * size * 4 + col;
	//Move lMat pointer to our working area in local matrix
	lMat += get_local_id(0) * 8;
	//Store the block we're working on
	lMat[0] = src[0];
	lMat[1] = src[size];
	lMat[2] = src[2 * size];
	lMat[3] = src[3 * size];

	//Process on-diagonal block
	if (row == col){
		//Need to work this out and see how this is working
		src[0] = (float4)(lMat[0].x, lMat[1].x, lMat[2].x, lMat[3].x);
		src[size] = (float4)(lMat[0].y, lMat[1].y, lMat[2].y, lMat[3].y);
		src[2 * size] = (float4)(lMat[0].z, lMat[1].z, lMat[2].z, lMat[3].z);
		src[3 * size] = (float4)(lMat[0].w, lMat[1].w, lMat[2].w, lMat[3].w);
	}
	else {
		//If we're processing an off-diagonal block we need to read in the 
		//block we want to switch with
		dst = gMat + col * size * 4 + row;
		//Store the block we're working on
		lMat[4] = dst[0];
		lMat[5] = dst[size];
		lMat[6] = dst[2 * size];
		lMat[7] = dst[3 * size];
		//Swap the blocks
		dst[0] = (float4)(lMat[0].x, lMat[1].x, lMat[2].x, lMat[3].x);
		dst[size] = (float4)(lMat[0].y, lMat[1].y, lMat[2].y, lMat[3].y);
		dst[2 * size] = (float4)(lMat[0].z, lMat[1].z, lMat[2].z, lMat[3].z);
		dst[3 * size] = (float4)(lMat[0].w, lMat[1].w, lMat[2].w, lMat[3].w);
		src[0] = (float4)(lMat[4].x, lMat[5].x, lMat[6].x, lMat[7].x);
		src[size] = (float4)(lMat[4].y, lMat[5].y, lMat[6].y, lMat[7].y);
		src[2 * size] = (float4)(lMat[4].z, lMat[5].z, lMat[6].z, lMat[7].z);
		src[3 * size] = (float4)(lMat[4].w, lMat[5].w, lMat[6].w, lMat[7].w);
	}
}
