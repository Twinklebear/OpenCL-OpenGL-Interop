#version 430 core

//Hello world program, fill a buffer [0-16]

layout (local_size_x = 16, local_size_y = 1) in;

layout (std430, binding = 0) buffer helloBuffer {
	uint msg[16];
};

void main(){
	msg[gl_GlobalInvocationID.x] = gl_GlobalInvocationID.x;
}