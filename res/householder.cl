//Compute the householder matrix for the vector
__kernel void householder(float4 v, __global float4 *mat){
	v *= M_SQRT2_F / length(v);
	mat[0] = (float4)(1.0f, 0.0f, 0.0f, 0.0f) - (v * v.x);
	mat[1] = (float4)(0.0f, 1.0f, 0.0f, 0.0f) - (v * v.y);
	mat[2] = (float4)(0.0f, 0.0f, 1.0f, 0.0f) - (v * v.z);
	mat[3] = (float4)(0.0f, 0.0f, 0.0f, 1.0f) - (v * v.w);
}