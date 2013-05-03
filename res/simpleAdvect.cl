__constant sampler_t nearest = CLK_NORMALIZED_COORDS_TRUE | CLK_ADDRESS_REPEAT
	| CLK_FILTER_NEAREST;

__constant sampler_t linear = CLK_NORMALIZED_COORDS_TRUE | CLK_ADDRESS_REPEAT
	| CLK_FILTER_LINEAR;

//For this simple test we'll just take a vector as the velocity
__kernel void simpleAdvect(float dt, __constant float2 *vel, read_only image2d_t inImg,
	write_only image2d_t outImg, __global float2 *dbgData)
{
	int2 coord = (int2)(get_global_id(0), get_global_id(1));
	float2 x_n = convert_float2(coord) / convert_float2(get_image_dim(inImg));

	float2 v = *vel;
	if (x_n.y > 0.5f)
		v.x = -v.x;
	if (x_n.x > 0.5f)
		v.y = -v.y;

	
	//TODO: Some issue exists with the math that causes +velocity to go much
	//faster than -velocity
	//Perform RK2 to traceback the "start" position of this pixel
	// x(t) = x + vt
	// dx/dt = v
	//What about this method?
	//float2 k_1 = dt * (*vel);
	//float2 k_2 = dt * (*vel + k_1 / 2.0f);
	//float2 x_nS = x_n - k_2;
	//Is this RK2 or the version in advect correct? will have to check with book
	float2 x_nHalf = x_n - 0.5f * dt * v;//(*vel);
	float2 x_nS = x_nHalf - dt * v;//(*vel);
	dbgData[coord.x + coord.y * 256] = (float2)(x_n.x, x_nS.x);

	//Now sample the value where we "started" and set the next value at x_n to that one
	float4 tVal = read_imagef(inImg, nearest, x_nS);
	write_imagef(outImg, coord, tVal);
}
