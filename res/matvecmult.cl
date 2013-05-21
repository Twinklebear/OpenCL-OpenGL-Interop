//Multiply a 4nx4n matrix with a 4nx1 vector
//If I need more granularity I can shrink this and my matrix mult kernels
//to 2nx2n since my grids should at least be even sized
__kernel void matVecMult(__constant float4 *mat, __constant float4 *vec, 
	__global float *res)
{
	int row = get_global_id(0);
	int vecPerRow = get_global_size(0) / 4;

	float sum = 0.0f;
	for (int i = 0; i < vecPerRow; ++i)
		sum += dot(mat[row * vecPerRow + i], vec[i]);

	res[row] = sum;
}
