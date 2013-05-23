//This kernel will perform a big dot product on arbitrary size vectors that 4n in size
__kernel void bigDot(__global float4 *a, __global float4 *b, __global float *out){
	int id = get_global_id(0);
	out[id] = dot(a[id], b[id]);
}
