__constant sampler_t nearest = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP_TO_EDGE
	| CLK_FILTER_NEAREST;

__constant sampler_t linear = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP_TO_EDGE
	| CLK_FILTER_LINEAR;

//For this simple test we'll just take a vector as the velocity
__kernel void simpleAdvect(float dt, __constant float2 *vel, read_only image2d_t inImg,
	write_only image2d_t outImg)
{
	int2 coord = (int2)(get_global_id(0), get_global_id(1));
	float2 x_n = convert_float2(coord);

	float2 v = *vel;
	if (coord.y > (256.0f / 2.0f))
		v.x = -v.x;
	if (coord.x > (256.0f / 2.0f))
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
	//Wrap x_nS appropriately. Is there no built in function for this?
	//Or a faster way to do this?
	//get the width and height so we can wrap the coord around if needed
	float2 size = convert_float2(get_image_dim(inImg));
	//Wrap x, note that we wrap when at size and up because the image is 0 indexed
	//so 256 is an out of bounds value that should wrap back to 0
	if (x_nS.x > size.x - 1)
		x_nS.x = fmod(x_nS.x, size.x);
	else if (x_nS.x < 0)
		x_nS.x += size.x;
	//Wrap y
	if (x_nS.y > size.y - 1)
		x_nS.y = fmod(x_nS.y, size.y);
	else if (x_nS.y < 0)
		x_nS.y += size.y;

	//Now sample the value where we "started" and set the next value at x_n to that one
	float4 tVal = read_imagef(inImg, nearest, x_nS);
	write_imagef(outImg, coord, tVal);
}
