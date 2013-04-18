#define __CL_ENABLE_EXCEPTIONS

#include <stdexcept>
#include <string>
#include <iostream>
#include <iomanip>
#include <SDL.h>
#include <SDL_opengl.h>
#include <CL/cl.hpp>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <window.h>
#include <util.h>
#include <glfunctions.h>
#include <glshader.h>
#include <glprogram.h>
#include <glvertexbuffer.h>
#include <glvertexarray.h>
#include <timer.h>
#include "tinycl.h"

/*
* This function is a simple test of interop with OpenGL VBOs
* we make a VBO then use a kernel to fill in the vertex data
*/
GL::VertexBuffer testClGl();
/*
* This function advects a texture by the desired velocity over 
* a single time step
*/
GL::Texture advectTexture();
/*
* This function will shrink a texture down to a smaller size
*/
GL::Texture shrinkTexture();
/*
* This demo will move a texture with some velocity and draw the 
* the texture as it moves in real time
*/
void liveAdvectTexture();

int main(int argc, char** argv){
	try {
		Window::Init();
	}
	catch (const std::runtime_error &e){
		std::cout << e.what() << std::endl;
	}
	liveAdvectTexture();
	return 0;
	Window window("OpenGL/OpenCL Interop");

	//Load a square plane model
	std::vector<glm::vec3> verts;
	std::vector<unsigned short> indices;
	Util::LoadObj("../res/square.obj", verts, indices);
	
	//Load texture from cl program that tweaks it some
	GL::Texture texture = advectTexture();

	GL::VertexBuffer vbo(verts);
	GL::VertexArray vao;
	vao.Reference(vbo, "vbo");
	vao.ElementBuffer(indices);
	//Setup program
	GL::Program prog("../res/shader.v.glsl", "../res/shader.f.glsl");
	//Setup the attributes
	vao.SetAttribPointer("vbo", prog.GetAttribute("position"), 3, GL_FLOAT, GL_FALSE, 3 * sizeof(glm::vec3), 0);
	vao.SetAttribPointer("vbo", prog.GetAttribute("texIn"), 3, GL_FLOAT, GL_FALSE, 
		3 * sizeof(glm::vec3), (void*)(sizeof(glm::vec3) * 2));

	glm::mat4 view = glm::lookAt<float>(glm::vec3(0, 0, 1), glm::vec3(0, 0, -1), glm::vec3(0, 1, 0));
	//glm::mat4 proj = glm::ortho<float>(-1.0f, 1.0f, -1.0f, 1.0f, 0.1f, 100.0f);
	glm::mat4 proj = glm::perspective(60.0f, (float)(window.Box().w) /  (float)(window.Box().h), 0.1f, 100.0f);
	glm::mat4 model = glm::scale(0.5f, 0.5f, 1.0f);
	glm::mat4 mvp = proj * view * model;
	prog.UniformMat4x4("mvp", mvp);

	//Our event structure
	SDL_Event e;
	//For tracking if we want to quit
	bool quit = false;
	while (!quit){
		//Event Polling
		while (SDL_PollEvent(&e)){
			//If user closes he window
			if (e.type == SDL_QUIT)
				quit = true;
			//If user presses any key
			if (e.type == SDL_KEYDOWN){
				switch (e.key.keysym.sym){
					//For quitting, escape key
					case SDLK_ESCAPE:
						quit = true;
						break;
					default:
						break;
				}
			}
		}
		//RENDERING
		window.Clear();
		//window.DrawElements(vao, prog, GL_TRIANGLES, vao.NumElements("elem"));
		window.DrawElementsTextured(vao, prog, texture, GL_TRIANGLES, vao.NumElements("elem"));
		window.Present();
	}
	Window::Quit();
	
	return 0;
}
GL::VertexBuffer testClGl(){
	CL::TinyCL tiny(CL::DEVICE::GPU, true);
	cl::Program program = tiny.LoadProgram("../res/glinterop.cl");
	cl::Kernel kernel = tiny.LoadKernel(program, "test");
	//Make a VBO that can hold a triangle with vect4 verts
	GL::VertexBuffer vbo(3 * 4 * sizeof(float));
	
	try {
		//Create a CL buffer for the VBO
		cl::BufferGL clVbo = tiny.BufferGL(CL::MEM::READ_WRITE, vbo);
		//Set the vbo as an argument
		kernel.setArg(0, clVbo);

		//Acquire the gl objects
		std::vector<cl::Memory> memObjs;
		memObjs.push_back(clVbo);
		tiny.mQueue.enqueueAcquireGLObjects(&memObjs);

		//We want 3 work items per work group and a total of 1 work groups
		//note that: work groups = globalSize / localSize
		cl::NDRange local(3);
		cl::NDRange global(3);

		//Run the kernel
		tiny.RunKernel(kernel, local, global);
		//Release the GL objects
		tiny.mQueue.enqueueReleaseGLObjects(&memObjs);
	}
	catch (cl::Error err){
		std::cout << "Error! " << err.what() << " code: " << err.err() << std::endl;
	}
	return vbo;
}
GL::Texture advectTexture(){
	CL::TinyCL tiny(CL::DEVICE::GPU, true);
	cl::Program program = tiny.LoadProgram("../res/simpleAdvect.cl");
	cl::Kernel kernel = tiny.LoadKernel(program, "simpleAdvect");
	//Make textures to work with
	GL::Texture initial("../res/map.png");
	GL::Texture texture("../res/blank.png");
	
	try {
#ifdef CL_VERSION_1_2
		cl::ImageGL inImg = tiny.ImageFromTexture(CL::MEM::READ_ONLY, initial);
		cl::ImageGL outImg = tiny.ImageFromTexture(CL::MEM::WRITE_ONLY, texture);
#else
		cl::Image2DGL inImg = tiny.ImageFromTexture(CL::MEM::READ_ONLY, initial);
		cl::Image2DGL outImg = tiny.ImageFromTexture(CL::MEM::WRITE_ONLY, texture);
#endif
		float velocity[2] = { 400.0f, 400.0f };
		cl::Buffer velBuf = tiny.Buffer(CL::MEM::READ_ONLY, 2 * sizeof(float), velocity);

		float dt = 1.0f / 60.0f;
		kernel.setArg(0, sizeof(float), &dt);
		kernel.setArg(1, velBuf);
		kernel.setArg(2, inImg);
		kernel.setArg(3, outImg);
		//Acquire the GL objects
		std::vector<cl::Memory> glObjs;
		glObjs.push_back(inImg);
		glObjs.push_back(outImg);

		//Let OpenGL wrap up anything it's doing before we get the objects
		glFinish();
		tiny.mQueue.enqueueAcquireGLObjects(&glObjs);

		//the image is 256x256 TODO: add this information to the texture class
		//Query and use the preferred work group size for our local size
		size_t workSize = kernel.getWorkGroupInfo<CL_KERNEL_PREFERRED_WORK_GROUP_SIZE_MULTIPLE>(tiny.mDevices.at(0));
		cl::NDRange local(workSize, workSize);
		cl::NDRange global(256, 256);
		tiny.RunKernel(kernel, local, global);
		//release GL objects & wait for it to finish before doing anything else
		tiny.mQueue.enqueueReleaseGLObjects(&glObjs);
		tiny.mQueue.finish();
	}
	catch (cl::Error err){
		std::cout << "Error! " << err.what() << " code: " << err.err() << std::endl;
	}
	return texture;
}
GL::Texture shrinkTexture(){
	CL::TinyCL tiny(CL::DEVICE::GPU, true);
	cl::Program program = tiny.LoadProgram("../res/shrinkImage.cl");
	cl::Kernel kernel = tiny.LoadKernel(program, "bilinearResample");
	//Make textures to work with
	GL::Texture initial("../res/map.png");
	//I should add the ability to allocate empty images? Or is that a CL thing? Hmmm
	//Can I get a texture from a cl::ImageGL?
	GL::Texture texture("../res/blanksmall.png");

	try {
#ifdef CL_VERSION_1_2
		cl::ImageGL inImg = tiny.ImageFromTexture(CL::MEM::READ_ONLY, initial);
		cl::ImageGL outImg = tiny.ImageFromTexture(CL::MEM::WRITE_ONLY, texture);
#else
		cl::Image2DGL inImg = tiny.ImageFromTexture(CL::MEM::READ_ONLY, initial);
		//Why does this throw invalid GL object if i load blanksmall as the texture?
		cl::Image2DGL outImg = tiny.ImageFromTexture(CL::MEM::WRITE_ONLY, texture);
#endif
		kernel.setArg(0, inImg);
		kernel.setArg(1, outImg);
		//set the ratio to 4 (blanksmall is 64x64, map is 256x256), so 4 map pixels per small
		int ratio = 4;
		kernel.setArg(2, sizeof(int), &ratio);

		std::vector<cl::Memory> glObjs;
		glObjs.push_back(inImg);
		glObjs.push_back(outImg);

		//Let OpenGL wrap up anything it's doing before we get the objects
		glFinish();
		tiny.mQueue.enqueueAcquireGLObjects(&glObjs);

		//the image is 256x256 TODO: add this information to the texture class
		//Query and use the preferred work group size for our local size
		size_t workSize = kernel.getWorkGroupInfo<CL_KERNEL_PREFERRED_WORK_GROUP_SIZE_MULTIPLE>(tiny.mDevices.at(0));
		cl::NDRange local(workSize, workSize);
		//Here we now use the size of the small image
		cl::NDRange global(64, 64);
		tiny.RunKernel(kernel, local, global);
		//release GL objects & wait for it to finish before doing anything else
		tiny.mQueue.enqueueReleaseGLObjects(&glObjs);
		tiny.mQueue.finish();
	}
	catch (const cl::Error &e){
		std::cout << "cl::Error: " << e.what() << " code: " << e.err() << std::endl;
	}
	return texture;
}
void liveAdvectTexture(){
	Window window("Realtime Texture Advection");
	//Set an fps cap
	const float FPS = 60.0f;

	//Load and setup a square plane that we can draw the texture too
	std::vector<glm::vec3> verts;
	std::vector<unsigned short> indices;
	Util::LoadObj("../res/square.obj", verts, indices);
	GL::VertexBuffer vbo(verts);
	GL::VertexArray vao;
	vao.Reference(vbo, "vbo");
	vao.ElementBuffer(indices);
	//Setup program
	GL::Program prog("../res/shader.v.glsl", "../res/shader.f.glsl");
	//Setup the attributes
	vao.SetAttribPointer("vbo", prog.GetAttribute("position"), 3, GL_FLOAT, GL_FALSE, 3 * sizeof(glm::vec3), 0);
	vao.SetAttribPointer("vbo", prog.GetAttribute("texIn"), 3, GL_FLOAT, GL_FALSE, 
		3 * sizeof(glm::vec3), (void*)(sizeof(glm::vec3) * 2));

	glm::mat4 view = glm::lookAt<float>(glm::vec3(0, 0, 1), glm::vec3(0, 0, -1), glm::vec3(0, 1, 0));
	glm::mat4 proj = glm::perspective(60.0f, (float)(window.Box().w) /  (float)(window.Box().h), 0.1f, 100.0f);
	glm::mat4 model = glm::scale(0.5f, 0.5f, 1.0f);
	glm::mat4 mvp = proj * view * model;
	prog.UniformMat4x4("mvp", mvp);

	//Setup our OpenCL context + program and kernel
	CL::TinyCL tiny(CL::DEVICE::GPU, true);
	cl::Program program = tiny.LoadProgram("../res/simpleAdvect.cl");
	cl::Kernel kernel = tiny.LoadKernel(program, "simpleAdvect");
	/*
	* I don't think OpenCL or OpenGL provide a simple method for copying images/textures so 
	* instead we'll flip the in/out image each step and draw the out image by setting active = out
	*/
	//Make textures to work with
	GL::Texture texA("../res/map.png");
	GL::Texture texB("../res/blank.png");
	//Active is the actual texture we will draw
	GL::Texture active = texB;
	//Setup our OpenCL data
#ifdef CL_VERSION_1_2
	cl::ImageGL imgA = tiny.ImageFromTexture(CL::MEM::READ_WRITE, texA);
	cl::ImageGL imgB = tiny.ImageFromTexture(CL::MEM::READ_WRITE, texB);
#else
	cl::Image2DGL imgA = tiny.ImageFromTexture(CL::MEM::READ_WRITE, texA);
	cl::Image2DGL imgB = tiny.ImageFromTexture(CL::MEM::READ_WRITE, texB);
#endif
	float velocity[2] = { 25.0f, 0.0f };
	cl::Buffer velBuf = tiny.Buffer(CL::MEM::READ_ONLY, 2 * sizeof(float), velocity);
	//Setup our GL objects vector
	std::vector<cl::Memory> glObjs;
	glObjs.push_back(imgA);
	glObjs.push_back(imgB);
	//The time step and velocity for now will be constant
	float dt = 1 / FPS;
	kernel.setArg(0, sizeof(float), &dt); 
	kernel.setArg(1, velBuf);
	//Query the preferred work group size
	size_t workSize = kernel.getWorkGroupInfo<CL_KERNEL_PREFERRED_WORK_GROUP_SIZE_MULTIPLE>(tiny.mDevices.at(0));
	//fixed for now
	size_t imgSize = 256;
	cl::NDRange local(workSize, workSize);
	cl::NDRange global(imgSize, imgSize);
	//Track the run number so we know which texture to set as in/out and which to draw
	int run = 0;

	//Our event structure
	SDL_Event e;
	//Limit framerate with a timer
	Timer delta;
	//For tracking if we want to quit
	bool quit = false;
	while (!quit){
		delta.Start();
		//Event Polling
		while (SDL_PollEvent(&e)){
			//If user closes he window
			if (e.type == SDL_QUIT)
				quit = true;
			//If user presses any key
			if (e.type == SDL_KEYDOWN){
				switch (e.key.keysym.sym){
					//For quitting, escape key
					case SDLK_ESCAPE:
						quit = true;
						break;
					default:
						break;
				}
			}
		}
		//Run the kernel, setting the in/out textures properly. On even runs the output will be
		//in texB, on odd runs output will be in texA
		try {
			//On even runs and the first run texB/imgB is our output, on odd runs it's flipped
			//Is this really the best way to do this? Maybe there is some faster way to copy the image over
			//instead of updating this each time
			if (run % 2 == 0 || run == 0){
				kernel.setArg(2, imgA);
				kernel.setArg(3, imgB);
				active = texB;
			}
			else {
				kernel.setArg(2, imgB);
				kernel.setArg(3, imgA);
				active = texA;
			}
			glFinish();
			tiny.mQueue.enqueueAcquireGLObjects(&glObjs);

			tiny.RunKernel(kernel, local, global);
			
			tiny.mQueue.enqueueReleaseGLObjects(&glObjs);
			tiny.mQueue.finish();
			++run;
		}
		catch (const cl::Error &e){
			std::cout << "Error: " << e.what() << " code: " << e.err() << std::endl;
		}
		//RENDERING
		window.Clear();
		window.DrawElementsTextured(vao, prog, active, GL_TRIANGLES, vao.NumElements("elem"));
		window.Present();

		//Cap fps
		if (delta.Ticks() < 1000 / FPS)
			SDL_Delay(1000 / FPS - delta.Ticks());
	}
	Window::Quit();
}
