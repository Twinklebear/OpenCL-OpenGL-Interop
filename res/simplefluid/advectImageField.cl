//We want blending between the velocities across cells so we use a linear blending sampler
__constant sampler_t linear = CLK_NORMALIZED_COORDS_TRUE | CLK_ADDRESS_REPEAT
	| CLK_FILTER_LINEAR;
//Nearest sampler for testing
__constant sampler_t nearest = CLK_NORMALIZED_COORDS_TRUE | CLK_ADDRESS_REPEAT
	| CLK_FILTER_NEAREST;

//Some upscaling to apply to the velocity so we can move at higher speeds
#define VSCALE 15.f
/*
* This kernel will take an input image to be advected by a velocity field
* The velocity field is also passed as an RGBA image where R -> x and G -> y
* and 127/255 is taken as 0 (since I couldn't set color to 127.5 in GIMP)
* to enable negative values
* @param dt The elapsed time to move for
* @param velocity The velocity field image
* @param inField The input image data to be moved by the velocity field
* @param outField The output image to write the moved inField data
*/
__kernel void advectImageField(float dt, read_only image2d_t velocity, read_only image2d_t inField,
	write_only image2d_t outField)
{
	int2 coord = (int2)(get_global_id(0), get_global_id(1));
	float2 pos = convert_float2(coord);
	/*
	* Because we're working on a MAC grid and the velocity values are stored at the edges of
	* cells we need to read x velocities and y velocities at different offsets
	* x is read at an offset of +.5 pixel width in x 
	* y is read at an offset of +.5 pixel height in y
	* Because the grid is square the offsets for these reads is the same
	* I think we also need an additional offset of 1 pixel in y since 0 starts
	* at the bottom left of the image, and the 34th row is junk
	* TODO: The offsets still aren't right.
	*/
	float vOffset = 0.5f / convert_float2(get_image_width(velocity));
	float zero = 126.f / 255.f;
	//Read the initial velocity values from the velocity image and take the first step
	float2 vPos = (pos + (float2)(0.5f, 0.5f)) / convert_float2(get_image_dim(velocity));
	float2 vel;
	vel = read_imagef(velocity, linear, vPos + (float2)(vOffset, vOffset)).xy - (float2)(zero, zero);
	//vel.x = read_imagef(velocity, linear, vPos + (float2)(vOffset, 0.f)).x - zero;
	//vel.y = read_imagef(velocity, linear, vPos + (float2)(0.f, vOffset)).y - zero;
	//Apply some scaling so we can go faster
	vel = vel * VSCALE;
	//TODO: Is this integration method outlined in the book really RK2? Or is it some tweaked version?
	//Is it alright to use/did I misrecall it?
	pos = pos - 0.5f * dt * vel;

	//Sample the velocity field again at the halfway point and take the second step
	vPos = (pos + (float2)(0, 1)) / convert_float2(get_image_dim(velocity));
	vel = read_imagef(velocity, linear, vPos + (float2)(vOffset, vOffset)).xy - (float2)(zero, zero);
	//vel.x = read_imagef(velocity, linear, vPos + (float2)(vOffset, 0.f)).x - zero;
	//vel.y = read_imagef(velocity, linear, vPos + (float2)(0.f, vOffset)).y - zero;
	vel = vel * VSCALE;
	pos = pos - dt * vel;

	//Now we're were we came from so sample the input field and write that to the output
	pos = (pos + (float2)(0.5f, 0.5f)) / convert_float2(get_image_dim(inField));
	float4 val = read_imagef(inField, linear, pos);
	write_imagef(outField, coord, val);
}
