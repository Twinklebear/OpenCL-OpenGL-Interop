__constant sampler_t nearest = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP_TO_EDGE
	| CLK_FILTER_NEAREST;

__constant sampler_t linear = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP_TO_EDGE
	| CLK_FILTER_LINEAR;

//For this simple test we'll just take a vector as the velocity
__kernel void simpleAdvect(__global float2 *vel, read_only image2d_t vals,
	write_only image2d_t valsNext)
{
	//We'll pick a time step
	float dt = 1.0f / 30.0f;
	int2 coord = (int2)(get_global_id(0), get_global_id(1));
	float2 x_n = (float2)(coord.x, coord.y);

	//We take only s01 since we're doing 2d, x/y values
	float2 x_nHalf = x_n - 0.5f * dt * (*vel);
	float2 x_nS = x_n - dt * (*vel);
	//Wrap x_nS appropriately. Is there no built in function for this?
	//Or a faster way to do this?
	//get the width and height so we can wrap the coord around if needed
	int2 size = get_image_dim(vals);
	//Wrap x, note that we wrap when at size and up because the image is 0 indexed
	//so 256 is an out of bounds value that should wrap back to 0
	if (x_nS.x > size.x - 1)
		x_nS.x = fmod(x_nS.x, (float)(size.x));
	else if (x_nS.x < 0)
		x_nS.x += size.x;
	//Wrap y
	if (x_nS.y > size.y - 1)
		x_nS.y = fmod(x_nS.y, (float)(size.y));
	else if (x_nS.y < 0)
		x_nS.y += size.y;

	//Now sample the value where we "started" and set the next value at x_n to that one
	//We use the linear sampler to get linear interpolation at the traceposition for free
	//This is ok on Nvidia
	float4 tVal = read_imagef(vals, nearest, x_nS);
	//But why can't I do this on Nvidia? (is it b/c Nvidia has CL1.1?)
	//float4 tVal = (float4)(1.0f, 0.0f, 0.0f, 1.0f);
	write_imagef(valsNext, coord, tVal);
}
