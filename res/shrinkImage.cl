__constant sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP_TO_EDGE
	| CLK_FILTER_LINEAR;

//This kernel will perform bilinear resampling on an image to create an image that is 
// inSize / ratio in size
__kernel void bilinearResample(read_only image2d_t in, write_only image2d_t out, int ratio){
	//average the range of pixels, from our id to coord + ratio
	int2 gId = (int2)(get_global_id(0), get_global_id(1));
	/*
	* We want to sample every ratio pixels, so if we're writing to pixel 0,0
	* in the out we want to sample 0,0 but if we're writing 1,0 we want to sample at 
	* 4,0
	*/
	float4 color = read_imagef(in, sampler, gId * ratio);
	write_imagef(out, gId, color);
}