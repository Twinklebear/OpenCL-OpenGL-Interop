//This kernel will multiply two matrices of size 4nx4n. It's required that matrix b
//has been transposed
__kernel void matrixMult(__constant float4 *a, __constant float4 *b, 
	__global float *c)
{
	int nRows = get_global_size(0);
	int vectorsPerRow = nRows / 4;
	int start = get_global_id(0) * vectorsPerRow;
	a += start;
	c += start * 4;

	float sum;
	//We fill in a row in c, technically nRows means nCols but this is only
	//for square matrices so they're the same
	for (int i = 0; i < nRows; ++i){
		sum = 0.0f;
		for (int j = 0; j < vectorsPerRow; ++j)
			sum += dot(a[j], b[i * vectorsPerRow + j]);

		c[i] = sum;
	}
}
