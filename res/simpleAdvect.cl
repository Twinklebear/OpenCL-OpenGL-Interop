__constant sampler_t nearest = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP_TO_EDGE
	| CLK_FILTER_NEAREST;

__constant sampler_t linear = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP_TO_EDGE
	| CLK_FILTER_LINEAR;

//For this simple test we'll just take a vector as the velocity
__kernel void simpleAdvect(__global float2 *vel, read_only image2d_t vals,
	write_only image2d_t valsNext, __global float4 *dataOut)
{
	//We'll pick a time step
	float dt = 1.0f / 30.0f;
	int2 coord = (int2)(get_global_id(0), get_global_id(1));
	float2 x_n = (float2)(coord.x, coord.y);

	//We take only s01 since we're doing 2d, x/y values
	float2 x_nHalf = x_n - 0.5f * dt * (*vel);
	float2 x_nS = x_n - dt * (*vel);

	//Now sample the value where we "started" and set the next value at x_n to that one
	//We use the linear sampler to get linear interpolation at the traceposition for free
	float4 tVal = read_imagef(vals, nearest, x_nS);
	//Why can't I do this on Nvidia?
	//float4 tVal = (float4)(0.5f, 0.0f, 0.0f, 1.0f);
	write_imagef(valsNext, coord, tVal);
	//Pixel colors are RGBA with values 0.0f-1.0f on Intel, but what is Nvida doing?
	//I don't get the same results just writing what I though was red
	dataOut[coord.x + coord.y] = tVal;
}
