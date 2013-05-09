//Multiply a 4x4 matrix with a 4x1 vector
//I should generalize this later to mult MxN with Nx1
__kernel void matVecMult(__global float4 *mat, float4 vec, __global float4 *res){
	res[0].x = dot(mat[0], vec);
	res[0].y = dot(mat[1], vec);
	res[0].z = dot(mat[2], vec);
	res[0].w = dot(mat[3], vec);
}
