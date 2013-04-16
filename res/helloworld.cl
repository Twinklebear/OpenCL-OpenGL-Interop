//We want to use double precision
//I don't have support for this extension on my laptop
//#pragma OPENCL EXTENSION cl_khr_fp64 : enable

__kernel void vectorAdd(__global float *a, __global float *b, __global float *c){
	//idx of elems to add
	unsigned int n = get_global_id(0);
	//sum the nth element of the vectors a and b and store in c
	c[n] = a[n] + b[n];
}