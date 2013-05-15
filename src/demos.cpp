#define __CL_ENABLE_EXCEPTIONS

#include <stdexcept>
#include <string>
#include <iostream>
#include <iomanip>
#include <GL/glew.h>
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
#include "demos.h"

void liveAdvectTexture(){
	Window::Init();
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
	const float speed = 0.2f;
	float velocity[2] = { 0.0f, 0.0f };
	cl::Buffer velBuf = tiny.Buffer(CL::MEM::READ_ONLY, 2 * sizeof(float), velocity);
	//Debug buffer, write velocities out
	float *dbgData = new float[256 * 256 * 2];
	cl::Buffer dbgBuf = tiny.Buffer(CL::MEM::WRITE_ONLY, 256 * 256 * 2 * sizeof(float));
	//Setup our GL objects vector
	std::vector<cl::Memory> glObjs;
	glObjs.push_back(imgA);
	glObjs.push_back(imgB);
	//The time step will be constant and velocity won't change each step, so set'em now
	float dt = 1.0f / FPS;
	kernel.setArg(0, sizeof(float), &dt);
	kernel.setArg(1, velBuf);
	kernel.setArg(4, dbgBuf);
	//Query the preferred work group size
	int workSize = tiny.PreferredWorkSize(kernel);
	//fixed for now
	int imgSize = 256;
	cl::NDRange local(workSize, workSize);
	cl::NDRange global(imgSize, imgSize);
	//Track the run number so we know which texture to set as in/out and which to draw
	int run = 0;

	//Our event structure
	SDL_Event e;
	//Limit framerate with a timer
	Timer delta;
	//For tracking if we want to quit
	bool quit = false, paused = false, printDbg = true;
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
					//So we can change velocity
					case SDLK_w:
						velocity[1] = speed;
						tiny.WriteData(velBuf, 2 * sizeof(float), velocity);
						break;
					case SDLK_s:
						velocity[1] = -speed;
						tiny.WriteData(velBuf, 2 * sizeof(float), velocity);
						break;
					case SDLK_a:
						velocity[0] = -speed;
						tiny.WriteData(velBuf, 2 * sizeof(float), velocity);
						break;
					case SDLK_d:
						velocity[0] = speed;
						tiny.WriteData(velBuf, 2 * sizeof(float), velocity);
						break;
					case SDLK_r:
						velocity[0] = 0.0f;
						velocity[1] = 0.0f;
						tiny.WriteData(velBuf, 2 * sizeof(float), velocity);
						break;
					//Toggle pause
					case SDLK_SPACE:
						paused = !paused;
						break;
					//For quitting, escape key
					case SDLK_ESCAPE:
						quit = true;
						break;
					default:
						break;
				}
			}
		}
		//When we pause read out and print the velocity information
		if (paused && printDbg){
			printDbg = false;
			tiny.ReadData(dbgBuf, 256 * 256 * 2, dbgData);
			//Print only the bottom and top rows, note that 0, 0 denotes the
			//bottom left corner on the image
			std::cout << "Bottom row:\n";
			for (int i = 0; i < 1 * 2; ++i){
				if (i % 2 == 0){
					std::cout << "\n" << "idx: " << i 
						<< " x,y: " << (i / 2) % 256 << ", "
						<< (i / 2 - (i / 2) % 256) / 256 << " val: ";
				}
				std::cout << dbgData[i] << ", ";
			}
			//std::cout << "\n\nLowrow:\n";
			//Why do I get nothing at row 64 and up?
			//int row = 64;
			//for (int i = row * 256 * 2; i < row * 256 * 2 + 256 * 2; ++i){
			//	if (i % 2 == 0){
			//		std::cout << "\n" << "idx: " << i 
			//			<< " x,y: " << (i / 2) % 256 << ", "
			//			<< (i / 2 - (i / 2) % 256) / 256 << " val: ";
			//	}
			//	std::cout << dbgData[i] << ", ";
			//}
			std::cout << std::endl;
		}
		//Run the kernel, setting the in/out textures properly. On even runs the output will be
		//in texB, on odd runs output will be in texA
		if (!paused){
			printDbg = true;
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
		}
		//RENDERING
		window.Clear();
		window.DrawElementsTextured(vao, prog, active, GL_TRIANGLES, vao.NumElements("elem"));
		window.Present();

		//Cap fps
		if (delta.Ticks() < 1000 / FPS)
			SDL_Delay(1000 / FPS - delta.Ticks());
	}
	delete dbgData;
	Window::Quit();
}
void bigDot(){
	CL::TinyCL tiny(CL::DEVICE::GPU);
	cl::Program prog = tiny.LoadProgram("../res/bigDot.cl");
	cl::Kernel kernel = tiny.LoadKernel(prog, "bigDot");

	//Setup the input data
	const int nElem = 4;
	float vecA[nElem] = {0};
	float vecB[nElem] = {0};
	for (int i = 0; i < nElem; ++i){
		vecA[i] = i;
		vecB[i] = nElem - i;
	}
	//Print the vectors
	std::cout << "vect a: ";
	for (int i = 0; i < nElem; ++i)
		std::cout << vecA[i] << ", ";
	std::cout << "\nvect b: ";
	for (int i = 0; i < nElem; ++i)
		std::cout << vecB[i] << ", ";

	//Setup cl buffers
	cl::Buffer bufVecA = tiny.Buffer(CL::MEM::READ_ONLY, nElem * sizeof(float), vecA);
	cl::Buffer bufVecB = tiny.Buffer(CL::MEM::READ_ONLY, nElem * sizeof(float), vecB);
	//Our out buffer is (nElem / 4) because we process in chunks of 4
	cl::Buffer bOut = tiny.Buffer(CL::MEM::WRITE_ONLY, (nElem / 4) * sizeof(float));

	kernel.setArg(0, bufVecA);
	kernel.setArg(1, bufVecB);
	kernel.setArg(2, bOut);
	kernel.setArg(3, sizeof(int), (void*)&nElem);

	//We pass nullrange to the local argument to let OpenCL decide how to split things up
	cl::NDRange global(nElem / 4);
	tiny.RunKernel(kernel, cl::NullRange, global);

	//Read results
	float result[nElem / 4] = {0};
	tiny.ReadData(bOut, (nElem / 4) * sizeof(float), result);
	tiny.mQueue.finish();

	//Sum to get final result
	float sum = 0.0f;
	for (int i = 0; i < nElem / 4; ++i)
		sum += result[i];
	std::cout << "\nDot result: " << sum << std::endl;
}
void openglCompute(){
	Window::Init();
	Window window("OpenGL Compute - Nothing will be drawn");
	GLenum err = glewInit();
	if (err != GLEW_OK){
		std::cout << "Glew error: " << glewGetErrorString(err) << std::endl;
		return;
	}

	//This shader declares local group size to be 16x16 then does nothing
	const char *doNothingSrc = 
		"#version 430 core \n \
		//Input layout qualifier declaring a 16x16 local workgroup size \n \
		//It looks like a big bonus for Opencl is being able to set this at runtime \
		//Whereas in OpenGL compute shader it must be in the shader, although I suppose we could \
		//change that somehow? hm \
		layout (local_size_x = 16, local_size_y = 1) in; \n \
		void main() { }";

	std::string src = Util::ReadFile("../res/helloCompute.glsl");

	const char *shaderSrc = src.c_str();

	GLuint shader = glCreateShader(GL_COMPUTE_SHADER);
	glShaderSource(shader, 1, &shaderSrc, NULL);
	glCompileShader(shader);
	GLint status;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
	if (status != GL_TRUE){
		std::cout << "Compile failed" << std::endl;
		//Get the log length and then get the log
        GLint logLength;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength);
        std::vector<char> log(logLength);
        glGetShaderInfoLog(shader, logLength, NULL, &log[0]);

        //Construct and return log message
        std::string errorMsg(log.begin(), log.end());
        std::cout << errorMsg << std::endl;
	}

	GLuint program = glCreateProgram();
	glAttachShader(program, shader);
	glLinkProgram(program);
	glGetProgramiv(program, GL_LINK_STATUS, &status);
	if (status != GL_TRUE)
		std::cout << "Link failed" << std::endl;

	glUseProgram(program);
	
	//Setup the data buffer
	GLuint dataBuf;
	glGenBuffers(1, &dataBuf);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, dataBuf);
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(GLuint) * 16, NULL, GL_DYNAMIC_COPY);
	//Bind it as the 0th shader storage buffer binding pt
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, dataBuf);

	GLuint globalDim[] = { 16, 1, 1 };
	GLuint dispatchBuf;
	glGenBuffers(1, &dispatchBuf);
	glBindBuffer(GL_DISPATCH_INDIRECT_BUFFER, dispatchBuf);
	glBufferData(GL_DISPATCH_INDIRECT_BUFFER, sizeof(globalDim), globalDim, GL_STATIC_DRAW);
	glDispatchComputeIndirect(0);

	std::cout << "Error? " << std::hex << glGetError() << std::endl;

	//Now read out the results, interesting that they print as [0-9] then [a-f]
	int *outData = (int*)glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY);
	std::cout << "Data read: ";
	for (int i = 0; i < 16; ++i)
		std::cout << outData[i] << ", ";
	std::cout << std::endl;
	glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
	outData = nullptr;
}
cl::Buffer householderBuf(std::array<float, 4> vect, CL::TinyCL &tiny){
	cl::Program prog = tiny.LoadProgram("../res/householder.cl");
	cl::Kernel kernel = tiny.LoadKernel(prog, "householder");

	cl::Buffer res = tiny.Buffer(CL::MEM::WRITE_ONLY, sizeof(float) * 16);

	kernel.setArg(0, sizeof(float) * 4, &vect[0]);
	kernel.setArg(1, res);

	tiny.RunKernel(kernel, cl::NullRange, cl::NDRange(1));
	return res;
}
std::array<float, 16> householder(std::array<float, 4> vect, CL::TinyCL &tiny){
	cl::Buffer res = householderBuf(vect, tiny);
	std::array<float, 16> mat;
	tiny.ReadData(res, sizeof(float) * 16, &mat[0]);
	return mat;
}
cl::Buffer reflectBuf(std::array<float, 4> v, std::array<float, 4> u, CL::TinyCL &tiny){
	cl::Program prog = tiny.LoadProgram("../res/matvecmult.cl");
	cl::Kernel kernel = tiny.LoadKernel(prog, "matVecMult");

	cl::Buffer hMat = householderBuf(u, tiny);
	cl::Buffer res = tiny.Buffer(CL::MEM::WRITE_ONLY, sizeof(float) * 4);

	kernel.setArg(0, hMat);
	kernel.setArg(1, sizeof(float) * 4, &v[0]);
	kernel.setArg(2, res);

	tiny.RunKernel(kernel, cl::NullRange, cl::NDRange(1));
	return res;
}
std::array<float, 4> reflect(std::array<float, 4> v, std::array<float, 4> u, CL::TinyCL &tiny){
	cl::Buffer res = reflectBuf(v, u, tiny);
	std::array<float, 4> vect;
	tiny.ReadData(res, sizeof(float) * 4, &vect[0]);
	return vect;
}
void qrDecomp(std::array<float, 16> &a, std::array<float, 16> &q, 
	std::array<float, 16> &r, CL::TinyCL &tiny)
{

}
