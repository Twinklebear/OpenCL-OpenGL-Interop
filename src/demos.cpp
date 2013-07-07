#define __CL_ENABLE_EXCEPTIONS

#include <stdexcept>
#include <string>
#include <iostream>
#include <iomanip>
#include <chrono>
#include <GL/glew.h>
#include <SDL.h>
#include <CL/cl.hpp>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <window.h>
#include <util.h>
#include <glshader.h>
#include <glprogram.h>
#include <glbuffer.h>
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
	glm::mat4 model = glm::scale(0.55f, 0.55f, 1.0f);
	glm::mat4 mvp = proj * view * model;
	prog.uniformMat4x4("mvp", mvp);

	/*
	* I don't think OpenCL or OpenGL provide a simple method for copying images/textures so 
	* instead we'll flip the in/out image each step and draw the out image by setting active = out
	*/
	//Make textures to work with
	GL::Texture texA("../res/map.png", true, SOIL_FLAG_INVERT_Y | SOIL_FLAG_NTSC_SAFE_RGB);
	GL::Texture texB("../res/blank.png", true, SOIL_FLAG_INVERT_Y | SOIL_FLAG_NTSC_SAFE_RGB);
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

	Util::checkError("Dispatched compute");

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
	tiny.readData(res, sizeof(float) * 16, &mat[0], NULL, true);
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
	tiny.readData(res, sizeof(float) * 4, &vect[0], NULL, true);
	return vect;
}
std::vector<float> localConjGradSolve(const SparseMatrix &matrix, std::vector<float> &bVec, CL::TinyCL &tiny){
	if (bVec.size() != matrix.dim){
		std::cout << "b vector does not match A dim" << std::endl;
		return std::vector<float>();
	}
	cl::Program prog = tiny.loadProgram("../res/conjGrad.cl");
	cl::Kernel kernel = tiny.loadKernel(prog, "conjGrad");
	
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
	tiny.readData(resBuf, 2 * sizeof(float), info, NULL, true);
	std::cout << "After: " << info[0] << " iterations, the residual length is: " << info[1] << std::endl;

	//Read the solved x vector
	std::vector<float> x;
	x.resize(dim);
	tiny.readData(resBuf, dim * sizeof(float), &x[0], 2 * sizeof(float), true);

	delete[] rows;
	delete[] cols;
	delete[] vals;
	
	return x;
}
//The event callback I'm using for CG profiling, the data will be the kernel name
void CL_CALLBACK cgEventCallback(cl_event evt, cl_int status, void *data){
	cl_ulong startns = 0, endns = 0;
	int err = 0;
	err = clGetEventProfilingInfo(evt, CL_PROFILING_COMMAND_START, sizeof(cl_ulong), &startns, NULL);
	if (err != 0)
		std::cout << "error: " << err << std::endl;
	err = clGetEventProfilingInfo(evt, CL_PROFILING_COMMAND_END, sizeof(cl_ulong), &endns, NULL);
	if (err != 0)
		std::cout << "error: " << err << std::endl;
	cl_ulong nanosec = endns - startns;
	std::cout << *static_cast<std::string*>(data)
		<< " took: " << nanosec * 1e-6 << "ms"
		<< std::endl;
}
std::vector<float> conjugateGradient(const SparseMatrix &matrix, std::vector<float> &b, CL::TinyCL &tiny){
	if (b.size() != matrix.dim){
		std::cout << "Error: matrix dimensions doesn't match b dimensions" << std::endl;
		return std::vector<float>();
	}
	cl::Program program = tiny.loadProgram("../res/conjugateGradient.cl");
	//These kernels could probably be shoved in a vector but for clarity of what's going on
	//I'll leave them with seperate names for now
	cl::Kernel sparseMatVec = tiny.loadKernel(program, "sparseMatVec");
	cl::Kernel bigDot = tiny.loadKernel(program, "bigDot");
	cl::Kernel initVects = tiny.loadKernel(program, "initVects");
	cl::Kernel updateXR = tiny.loadKernel(program, "updateXR");
	cl::Kernel updateDir = tiny.loadKernel(program, "updateDir");
	cl::Kernel updateAlpha = tiny.loadKernel(program, "updateAlpha");

	//Get raw values from the matrix to send to OpenCL
	std::vector<int> rows, cols;
	rows.resize(matrix.elements.size());
	cols.resize(matrix.elements.size());
	std::vector<float> vals;
	vals.resize(matrix.elements.size());
	matrix.getRaw(&rows[0], &cols[0], &vals[0]);
	
	//Setup all our buffers
	//The sparse matrix will be stored in an array of 3 buffers, 0 - rowBuf, 1 - colBuf, 2 - valBuf
	std::array<cl::Buffer, 3> matrixBuf;
	matrixBuf[0] = tiny.buffer(CL::MEM::READ_ONLY, rows.size() * sizeof(int), &rows[0]);
	matrixBuf[1] = tiny.buffer(CL::MEM::READ_ONLY, cols.size() * sizeof(int), &cols[0]);
	matrixBuf[2] = tiny.buffer(CL::MEM::READ_ONLY, vals.size() * sizeof(float), &vals[0]);
	//The vector buffers needed, x, r, p, b, and aTimesP
	//Probably shove these in a vector later too, keep split up for clarity atm
	cl::Buffer x = tiny.buffer(CL::MEM::READ_WRITE, matrix.dim * sizeof(float));
	cl::Buffer r = tiny.buffer(CL::MEM::READ_WRITE, matrix.dim * sizeof(float));
	cl::Buffer p = tiny.buffer(CL::MEM::READ_WRITE, matrix.dim * sizeof(float));
	cl::Buffer bBuf = tiny.buffer(CL::MEM::READ_ONLY, b.size() * sizeof(float), &b[0]);
	cl::Buffer aTimesP = tiny.buffer(CL::MEM::READ_WRITE, matrix.dim * sizeof(float));
	cl::Buffer apDotpBuf = tiny.buffer(CL::MEM::READ_WRITE, matrix.dim * sizeof(float));
	cl::Buffer alpha = tiny.buffer(CL::MEM::READ_WRITE, sizeof(float));
	//the r dot r buffer holds old r dot r [0] and new r dot r [1]
	std::array<cl::Buffer, 2> rDotrBuf;
	for (int i = 0; i < rDotrBuf.size(); ++i)
		rDotrBuf[i] = tiny.buffer(CL::MEM::READ_WRITE, matrix.dim * sizeof(float));

	//Setup unchanging buffer arguments for the kernels
	initVects.setArg(0, x);
	initVects.setArg(1, r);
	initVects.setArg(2, p);
	initVects.setArg(3, bBuf);
	//initvects can run while we setup more stuff so start it up
	tiny.runKernel(initVects, cl::NullRange, cl::NDRange(matrix.dim));
	//after r is setup we can also run r dot r so queue it up as well
	bigDot.setArg(0, r);
	bigDot.setArg(1, r);
	bigDot.setArg(2, rDotrBuf[0]);
	tiny.runKernel(bigDot, cl::NullRange, cl::NDRange(matrix.dim / 2));
	//sparseMatVec for aTimesP = matrix * p
	int nVals = matrix.elements.size();
	sparseMatVec.setArg(0, sizeof(int), &nVals);
	for (int i = 1; i < 4; ++i)
		sparseMatVec.setArg(i, matrixBuf[i - 1]);
	sparseMatVec.setArg(4, p);
	sparseMatVec.setArg(5, aTimesP);

	updateXR.setArg(0, alpha);
	updateXR.setArg(1, p);
	updateXR.setArg(2, aTimesP);
	updateXR.setArg(3, x);
	updateXR.setArg(4, r);

	updateDir.setArg(0, rDotrBuf[1]);
	updateDir.setArg(1, rDotrBuf[0]);
	updateDir.setArg(2, r);
	updateDir.setArg(3, p);

	updateAlpha.setArg(0, rDotrBuf[0]);
	updateAlpha.setArg(1, apDotpBuf);
	updateAlpha.setArg(2, alpha);

	//TODO: Speed improvements

	//Various values we need to track to use in calculations in various kernels
	float rLength = 100;
	//We want to go for 1000 iterations or until the solution is close enough
	//Make i available outside the loop for now so we can print the iteration count
	int i = 0;
	for (; i < 1000 && rLength >= 0.01f; ++i){
		//compute aTimesP = Ap
		tiny.runKernel(sparseMatVec, cl::NullRange, cl::NDRange(matrix.dim), cl::NullRange);
		
		//now find apDotP = p dot aTimesp (ie. p dot Ap)
		bigDot.setArg(0, aTimesP);
		bigDot.setArg(1, p);
		bigDot.setArg(2, apDotpBuf);
		tiny.runKernel(bigDot, cl::NullRange, cl::NDRange(matrix.dim / 2), cl::NullRange);
		
		//Update alpha
		tiny.runKernel(updateAlpha, cl::NullRange, cl::NDRange(1), cl::NullRange);

		//update x & r, x += alpha * p & r -= alpha * atimesP
		tiny.runKernel(updateXR, cl::NullRange, cl::NDRange(matrix.dim), cl::NullRange);
		
		//Compute new value for r dot r
		bigDot.setArg(0, r);
		bigDot.setArg(1, r);
		bigDot.setArg(2, rDotrBuf[1]);
		tiny.runKernel(bigDot, cl::NullRange, cl::NDRange(matrix.dim / 2), cl::NullRange);

		//Update the direction
		tiny.runKernel(updateDir, cl::NullRange, cl::NDRange(matrix.dim), cl::NullRange);
		
		//Update oldRdotR and rLength
		tiny.mQueue.enqueueCopyBuffer(rDotrBuf[1], rDotrBuf[0], 0, 0, sizeof(float));
		float rdotr = 0;
		tiny.readData(rDotrBuf[1], sizeof(float), &rdotr, NULL, true);
		rLength = std::sqrtf(rdotr);
	}
	std::cout << "Solution took: " << i << " iterations, final residual length: " << rLength << std::endl;

	//Read the solution vector, x
	std::vector<float> solution;
	solution.resize(matrix.dim);
	tiny.readData(x, matrix.dim * sizeof(float), &solution[0], NULL, true);
	return solution;
}
std::vector<float> sparseVecMult(const SparseMatrix &matrix, std::vector<float> vec, CL::TinyCL &tiny){
	if (vec.size() != matrix.dim){
		std::cout << "Vector size doesn't match matrix dimensions" << std::endl;
		return std::vector<float>();
	}

	cl::Program program = tiny.loadProgram("../res/sparseMatVec.cl");
	cl::Kernel kernel = tiny.loadKernel(program, "sparseMatVec");

	int nVals = matrix.elements.size();
	int *rows = new int[nVals];
	int *cols = new int[nVals];
	float *vals = new float[nVals];
	matrix.getRaw(rows, cols, vals);

	cl::Buffer rowBuf = tiny.buffer(CL::MEM::READ_ONLY, nVals * sizeof(float), rows);
	cl::Buffer colBuf = tiny.buffer(CL::MEM::READ_ONLY, nVals * sizeof(float), cols);
	cl::Buffer valBuf = tiny.buffer(CL::MEM::READ_ONLY, nVals * sizeof(float), vals);
	cl::Buffer vecBuf = tiny.buffer(CL::MEM::READ_ONLY, vec.size() * sizeof(float), &vec[0]);
	cl::Buffer resBuf = tiny.buffer(CL::MEM::WRITE_ONLY, vec.size() * sizeof(float));

	kernel.setArg(0, sizeof(int), &nVals);
	kernel.setArg(1, rowBuf);
	kernel.setArg(2, colBuf);
	kernel.setArg(3, valBuf);
	kernel.setArg(4, vecBuf);
	kernel.setArg(5, resBuf);

	tiny.runKernel(kernel, cl::NullRange, matrix.dim);

	std::vector<float> result;
	result.resize(matrix.dim);
	tiny.readData(resBuf, matrix.dim * sizeof(float), &result[0], NULL, true);

	delete[] rows;
	delete[] cols;
	delete[] vals;

	return result;
}
void cgComparisonTest(){
	CL::TinyCL tiny(CL::DEVICE::GPU);
	SparseMatrix sMat("../res/bcsstk05.mtx");
	std::cout << "Computing CG on matrix of dim: " << sMat.dim << std::endl;
	std::vector<float> b;
	for (int i = 0; i < sMat.dim; ++i)
		b.push_back(i);

	//Compare my kernel with the book kernel to make sure it's correct
	std::vector<float> localRes, myRes;

	//Measure elapsed time for my kernel and book kernel
	std::cout << "Book CG:\n";
	std::chrono::high_resolution_clock::time_point start = std::chrono::high_resolution_clock::now();
	localRes = localConjGradSolve(sMat, b, tiny);
	std::chrono::high_resolution_clock::duration bookTime = std::chrono::high_resolution_clock::now() - start;

	std::cout << "------\nMy CG:\n";
	start = std::chrono::high_resolution_clock::now();
	myRes = conjugateGradient(sMat, b, tiny);
	std::chrono::high_resolution_clock::duration myTime = std::chrono::high_resolution_clock::now() - start;

	std::cout << "-----\nBook solve time: "
		<< std::chrono::duration_cast<std::chrono::milliseconds>(bookTime).count() << "ms\n"
		<< "My solve time: "
		<< std::chrono::duration_cast<std::chrono::milliseconds>(myTime).count() << "ms\n"
		<< "Time difference, mine - book: "
		<< std::chrono::duration_cast<std::chrono::milliseconds>(myTime - bookTime).count()
		<< "ms" << std::endl;

	//If the results are differ at a digit higher than the some minimal
	//error then my implementation is wrong
	float avgDiff = 0, maxDif = 1e-6;
	int nDifferent = 0;
	for (int i = 0; i < localRes.size(); ++i){
		float diff = std::abs(localRes.at(i) - myRes.at(i));
		if (diff > maxDif){
			avgDiff += diff;
			++nDifferent;
		}
	}
	if (nDifferent != 0)
	avgDiff /= nDifferent;

	std::cout << "# of values differing by more than " << std::scientific << maxDif
		<< " : " << nDifferent << " of " << myRes.size()
		<< "\nAverage difference between values: "
		<< avgDiff << std::endl;
}
