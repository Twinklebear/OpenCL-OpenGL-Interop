#define __CL_ENABLE_EXCEPTIONS

#include <stdexcept>
#include <string>
#include <iostream>
#include <iomanip>
#include <GL/glew.h>
#include <SDL.h>
#include <CL/cl.hpp>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <window.h>
#include <util.h>
#include <glshader.h>
#include <glprogram.h>
#include <glvertexbuffer.h>
#include <glvertexarray.h>
#include <timer.h>
#include "tinycl.h"
#include "sparsematrix.h"
#include "demos.h"

const std::array<glm::vec3, 8> quad = {
	//Vertex positions
	glm::vec3(-1.0, -1.0, 0.0),
	glm::vec3(1.0, -1.0, 0.0),
	glm::vec3(-1.0, 1.0, 0.0),
	glm::vec3(1.0, 1.0, 0.0),
	//UV coords
	glm::vec3(0.0, 0.0, 0.0),
	glm::vec3(1.0, 0.0, 0.0),
	glm::vec3(0.0, 1.0, 1.0),
	glm::vec3(1.0, 1.0, 0.0)
};
const std::array<unsigned short, 6> quadElems = {
	0, 1, 2,
	1, 3, 2
};
void liveAdvectTexture(){
	Window::Init();
	Window window("Realtime Texture Advection");
	//Set an fps cap
	const float FPS = 60.0f;

	//Setup a quad to draw too
	GL::VertexArray vao;
	vao.elementBuffer(quadElems);
	GL::VertexBuffer vbo(quad, GL::USAGE::STATIC_DRAW);	

	//Setup program
	GL::Program prog("../res/shader.v.glsl", "../res/shader.f.glsl");
	
	//Setup the attributes
	vao.setAttribPointer(vbo, prog.getAttribute("position"), 3, GL_FLOAT, GL_FALSE);
	vao.setAttribPointer(vbo, prog.getAttribute("texIn"), 3, GL_FLOAT, GL_FALSE, 0, (void*)(sizeof(glm::vec3) * 4));
	
	glm::mat4 view = glm::lookAt<float>(glm::vec3(0, 0, 1), glm::vec3(0, 0, -1), glm::vec3(0, 1, 0));
	glm::mat4 proj = glm::perspective(60.0f, (float)(window.Box().w) /  (float)(window.Box().h), 0.1f, 100.0f);
	//Curious why translating back -1.0f in z causes the texture to flicker. could it be a mip-maps thing?
	glm::mat4 model = glm::scale(0.55f, 0.55f, 1.0f);
	glm::mat4 mvp = proj * view * model;
	prog.uniformMat4x4("mvp", mvp);

	/*
	* I don't think OpenCL or OpenGL provide a simple method for copying images/textures so 
	* instead we'll flip the in/out image each step and draw the out image by setting active = out
	*/
	//Make textures to work with
	GL::Texture texA("../res/map.png");
	GL::Texture texB("../res/blank.png");
	//Active is the actual texture we will draw
	GL::Texture active = texB;

	//Setup our OpenCL context + program and kernel
	CL::TinyCL tiny(CL::DEVICE::GPU, true);
	cl::Program program = tiny.loadProgram("../res/simpleAdvect.cl");
	cl::Kernel kernel = tiny.loadKernel(program, "simpleAdvect");

	//Setup our OpenCL data
#ifdef CL_VERSION_1_2
	cl::ImageGL imgA = tiny.imageFromTexture(CL::MEM::READ_WRITE, texA);
	cl::ImageGL imgB = tiny.imageFromTexture(CL::MEM::READ_WRITE, texB);
#else
	cl::Image2DGL imgA = tiny.imageFromTexture(CL::MEM::READ_WRITE, texA);
	cl::Image2DGL imgB = tiny.imageFromTexture(CL::MEM::READ_WRITE, texB);
#endif
	const float speed = 0.2f;
	float velocity[2] = { 0.0f, 0.0f };
	cl::Buffer velBuf = tiny.buffer(CL::MEM::READ_ONLY, 2 * sizeof(float), velocity);

	//Setup our GL objects vector
	std::vector<cl::Memory> glObjs;
	glObjs.push_back(imgA);
	glObjs.push_back(imgB);

	//The time step will be constant and velocity won't change each step, so set'em now
	float dt = 1.0f / FPS;
	kernel.setArg(0, sizeof(float), &dt);
	kernel.setArg(1, velBuf);
	
	//Query the preferred work group size
	int workSize = tiny.preferredWorkSize(kernel);
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
	bool quit = false, paused = false;
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
						tiny.writeData(velBuf, 2 * sizeof(float), velocity);
						break;
					case SDLK_s:
						velocity[1] = -speed;
						tiny.writeData(velBuf, 2 * sizeof(float), velocity);
						break;
					case SDLK_a:
						velocity[0] = -speed;
						tiny.writeData(velBuf, 2 * sizeof(float), velocity);
						break;
					case SDLK_d:
						velocity[0] = speed;
						tiny.writeData(velBuf, 2 * sizeof(float), velocity);
						break;
					case SDLK_r:
						velocity[0] = 0.0f;
						velocity[1] = 0.0f;
						tiny.writeData(velBuf, 2 * sizeof(float), velocity);
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
		//Run the kernel, setting the in/out textures properly. On even runs the output will be
		//in texB, on odd runs output will be in texA
		if (!paused){
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

				tiny.runKernel(kernel, local, global);
			
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

		prog.use();
		glBindVertexArray(vao);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texA);
		glDrawElements(GL_TRIANGLES, vao.numElements(), GL_UNSIGNED_SHORT, NULL);

		window.Present();

		//Cap fps
		if (delta.Ticks() < 1000 / FPS)
			SDL_Delay(1000 / FPS - delta.Ticks());
	}
	Window::Quit();
}
void openglCompute(){
	//Note: Compute shaders are only OpenGL 4.3+ so my laptop can't run this, since
	//it's on 4.0
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

	std::string src = Util::readFile("../res/helloCompute.glsl");

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

	glDeleteShader(shader);
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

	//cleanup
	glDeleteBuffers(1, &dataBuf);
	glDeleteBuffers(1, &dispatchBuf);
	glDeleteProgram(program);
}
cl::Buffer householderBuf(std::array<float, 4> vect, CL::TinyCL &tiny){
	cl::Program prog = tiny.loadProgram("../res/householder.cl");
	cl::Kernel kernel = tiny.loadKernel(prog, "householder");

	cl::Buffer res = tiny.buffer(CL::MEM::WRITE_ONLY, sizeof(float) * 16);

	kernel.setArg(0, sizeof(float) * 4, &vect[0]);
	kernel.setArg(1, res);

	tiny.runKernel(kernel, cl::NullRange, cl::NDRange(1));
	return res;
}
std::array<float, 16> householder(std::array<float, 4> vect, CL::TinyCL &tiny){
	cl::Buffer res = householderBuf(vect, tiny);
	std::array<float, 16> mat;
	tiny.readData(res, sizeof(float) * 16, &mat[0]);
	return mat;
}
cl::Buffer reflectBuf(std::array<float, 4> v, std::array<float, 4> u, CL::TinyCL &tiny){
	cl::Program prog = tiny.loadProgram("../res/matvecmult.cl");
	cl::Kernel kernel = tiny.loadKernel(prog, "matVecMult");

	cl::Buffer hMat = householderBuf(u, tiny);
	cl::Buffer res = tiny.buffer(CL::MEM::WRITE_ONLY, sizeof(float) * 4);

	kernel.setArg(0, hMat);
	kernel.setArg(1, sizeof(float) * 4, &v[0]);
	kernel.setArg(2, res);

	tiny.runKernel(kernel, cl::NullRange, cl::NDRange(1));
	return res;
}
std::array<float, 4> reflect(std::array<float, 4> v, std::array<float, 4> u, CL::TinyCL &tiny){
	cl::Buffer res = reflectBuf(v, u, tiny);
	std::array<float, 4> vect;
	tiny.readData(res, sizeof(float) * 4, &vect[0]);
	return vect;
}
std::vector<float> conjGradSolve(const SparseMatrix &matrix, std::vector<float> bVec, CL::TinyCL &tiny){
	if (bVec.size() != matrix.dim){
		std::cout << "b vector does not match A dim" << std::endl;
		return std::vector<float>();
	}
	cl::Program prog = tiny.loadProgram("../res/conjGrad.cl");
	cl::Kernel kernel = tiny.loadKernel(prog, "conjGrad");

	std::cout << "conj grad max work group size: " << tiny.maxWorkGroupSize(kernel) << std::endl;
	
	//Get the raw data from the matrix
	int dim = matrix.dim, nElems = matrix.elements.size();
	int *rows = new int[nElems];
	int *cols = new int[nElems];
	float *vals = new float[nElems];
	matrix.getRaw(rows, cols, vals);

	//Setup cl buffers
	cl::Buffer rowBuf = tiny.buffer(CL::MEM::READ_ONLY, nElems * sizeof(int), rows);
	cl::Buffer colBuf = tiny.buffer(CL::MEM::READ_ONLY, nElems * sizeof(int), cols);
	cl::Buffer valBuf = tiny.buffer(CL::MEM::READ_ONLY, nElems * sizeof(float), vals);
	cl::Buffer bBuf = tiny.buffer(CL::MEM::READ_ONLY, bVec.size() * sizeof(float), &bVec[0]);
	cl::Buffer resBuf = tiny.buffer(CL::MEM::WRITE_ONLY, (2 + dim) * sizeof(float));

	kernel.setArg(0, sizeof(dim), &dim);
	kernel.setArg(1, sizeof(nElems), &nElems);
	kernel.setArg(2, dim * sizeof(float), NULL);
	kernel.setArg(3, dim * sizeof(float), NULL);
	kernel.setArg(4, dim * sizeof(float), NULL);
	kernel.setArg(5, dim * sizeof(float), NULL);
	kernel.setArg(6, rowBuf);
	kernel.setArg(7, colBuf);
	kernel.setArg(8, valBuf);
	kernel.setArg(9, bBuf);
	kernel.setArg(10, resBuf);

	tiny.runKernel(kernel, dim, dim);

	//Read results
	float info[2];
	tiny.readData(resBuf, 2 * sizeof(float), info);
	std::cout << "After: " << info[0] << " iterations, the residual length is: " << info[1] << std::endl;

	//Read the solved x vector
	std::vector<float> x;
	x.reserve(dim);
	tiny.readData(resBuf, dim * sizeof(float), &x[0], 2 * sizeof(float));

	delete[] rows;
	delete[] cols;
	delete[] vals;
	
	return x;
}
