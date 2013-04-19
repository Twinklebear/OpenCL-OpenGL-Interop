//this kernel will perform a big dot product on arbitrary size vectors
__kernel void bigDot(__global float4 *a, __global float4 *b, __global float *out, 
	int numElem)
{
	int gid = get_global_id(0);
	if (gid > numElem + 1)
		return;

	//Process the multiplication
	out[gid] = dot(a[gid], b[gid]);
}