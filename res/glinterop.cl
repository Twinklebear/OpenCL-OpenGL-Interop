//We want to use double precision
//I don't have support for this extension on my laptop
//#pragma OPENCL EXTENSION cl_khr_fp64 : enable

__kernel void test(__global float4 *vert){
	//idx of elems to add
	unsigned int n = get_global_id(0);
	//sum the nth element of the vectors a and b and store in c
	if (n == 0)
		vert[n] = (float4)(-0.5f, 0.5f, 0.0f, 1.0f);
	else if (n == 1)
		vert[n] = (float4)(-0.5f, 0.0f, 0.0f, 1.0f);
	else if (n == 2)
		vert[n] = (float4)(0.0f, 0.0f, 0.0f, 1.0f);
}