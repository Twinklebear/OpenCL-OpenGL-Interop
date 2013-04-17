__constant sampler_t nearest = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP_TO_EDGE
	| CLK_FILTER_NEAREST;

__constant sampler_t linear = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP_TO_EDGE
	| CLK_FILTER_LINEAR;

//u is the velocity field, vals is the initial values, valsNext is the values advected by the field
__kernel void advect(read_only image2d_t u, read_only image2d_t vals,
	write_only image2d_t valsNext)
{
	//We'll pick a time step
	float dt = 1.0f / 60.0f;
	float2 x_n = (float2)(get_global_id(0), get_global_id(1));
	//We can get rid of coord and just use x_n right?
	int2 coord = convert_int2_rte(x_n);
	//Trace back the starting position of the particle
	//velocity at the point we're examining
	float2 u_x = read_imagef(u, nearest, coord).xy;
	//We take only s01 since we're doing 2d, x/y values
	float2 x_nHalf = x_n - 0.5f * dt * u_x;
	//find the starting point of this "particle"
	float2 u_xnHalf = read_imagef(u, nearest, x_nHalf).xy;
	float2 x_nS = x_n - dt * u_xnHalf;


	//Now sample the value where we "started" and set the next value at x_n to that one
	//We use the linear sampler to get linear interpolation at the traceposition for free
	float4 tVal = read_imagef(vals, linear, x_nS);
	write_imagef(valsNext, coord, tVal);
}
