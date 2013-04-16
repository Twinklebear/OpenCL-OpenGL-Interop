#define __CL_ENABLE_EXCEPTIONS

#include <stdexcept>
#include <string>
#include <iostream>
#include <SDL.h>
#include <SDL_opengl.h>
#include <CL/cl.hpp>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include "window.h"
#include "util.h"
#include "glfunctions.h"
#include "glshader.h"
#include "glprogram.h"
#include "glvertexbuffer.h"
#include "glvertexarray.h"
#include "tinycl.h"

GL::VertexBuffer testClGl();
GL::Texture clTweakTexture();

int main(int argc, char** argv){
	try {
		Window::Init();
	}
	catch (const std::runtime_error &e){
		std::cout << e.what() << std::endl;
	}
	Window window("OpenGL/OpenCL Interop");

	//Load a square plane model
	std::vector<glm::vec3> verts;
	std::vector<unsigned short> indices;
	Util::LoadObj("../res/square.obj", verts, indices);
	
	//Load texture from cl program that tweaks it some
	GL::Texture texture = clTweakTexture();

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
GL::Texture clTweakTexture(){
	CL::TinyCL tiny(CL::DEVICE::GPU, true);
	cl::Program program = tiny.LoadProgram("../res/simpleAdvect.cl");
	cl::Kernel kernel = tiny.LoadKernel(program, "simpleAdvect");
	//Make textures to work with
	GL::Texture initial("../res/map.png");
	GL::Texture texture("../res/blank.png");
	
	try {
		//Setup cl images and velocity buffers
		cl::Image2DGL clInit = tiny.ImageFromTexture(CL::MEM::READ_ONLY, initial);
		cl::Image2DGL clFinal = tiny.ImageFromTexture(CL::MEM::WRITE_ONLY, texture);
		int velocity[2] = { 10.0, 10.0 };
		cl::Buffer velBuf = tiny.Buffer(CL::MEM::READ_ONLY, 2 * sizeof(float), velocity);

		kernel.setArg(0, velBuf);
		kernel.setArg(1, clInit);
		kernel.setArg(2, clFinal);
		//Acquire the GL objects
		std::vector<cl::Memory> glObjs;
		glObjs.push_back(clInit);
		glObjs.push_back(clFinal);

		//Let OpenGL wrap up anything it's doing before we get the objects
		glFinish();
		tiny.mQueue.enqueueAcquireGLObjects(&glObjs);

		//the image is 256x256 TODO: add this information to the texture class
		//and the preferred work group size is 32
		cl::NDRange local(32, 32);
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
