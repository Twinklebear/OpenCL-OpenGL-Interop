__constant sampler_t nearest = CLK_NORMALIZED_COORDS_TRUE | CLK_ADDRESS_CLAMP_TO_EDGE
	| CLK_FILTER_NEAREST;

__constant sampler_t linear = CLK_NORMALIZED_COORDS_TRUE | CLK_ADDRESS_CLAMP_TO_EDGE
	| CLK_FILTER_LINEAR;

//For this simple test we'll just take a vector as the velocity
__kernel void simpleAdvect(__global float2 *vel, read_only image2d_t vals, write_only image2d_t valsNext)
{
	//We'll pick a time step
	float dt = 1.0f / 60.0f;
	int2 coord = (int2)(get_global_id(0) / 2, get_global_id(1) / 2);
	float2 x_n = (float2)(coord.x, coord.y);

	//We take only s01 since we're doing 2d, x/y values
	float2 x_nHalf = x_n - 0.5f * dt * (*vel);
	float2 x_nS = x_n - dt * (*vel);

	//Now sample the value where we "started" and set the next value at x_n to that one
	//We use the linear sampler to get linear interpolation at the traceposition for free
	//float4 tVal = read_imagef(vals, linear, x_nS);
	//float4 tVal = (float4)(1.0f, 0.0f, 0.0f, 1.0f);
	//write_imagef(valsNext, coord, tVal);
	//uint4 tVal = (uint4)(0, 0, 0, 255);
	//write_imageui(valsNext, coord, tVal);
}
