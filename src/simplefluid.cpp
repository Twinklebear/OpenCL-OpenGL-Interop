#include <iostream>
#include <cmath>
#include <ctime>
#include <chrono>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glbuffer.h>
#include <glvertexarray.h>
#include <glprogram.h>
#include <window.h>
#include <input.h>
#include "tinycl.h"
#include "sparsematrix.h"
#include "demos.h"
#include "simplefluid.h"

const std::array<glm::vec3, 8> SimpleFluid::quad = {
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
const std::array<unsigned short, 6> SimpleFluid::quadElems = {
		0, 1, 2,
		1, 3, 2
};

SimpleFluid::SimpleFluid(int dim, Window &window)
	//Should be interop context, but false for now since I'm just testing setting up the interaction matrix
	//and solving it
	: tiny(CL::DEVICE::GPU, true), dim(dim), interactionMat(generateMatrix()), window(window)
{
}
SparseMatrix SimpleFluid::generateMatrix(){
	std::vector<Element> elements;
	int cells = std::pow(dim, 2);
	//All diagonal entries of the matrix are 4
	for (int i = 0; i < cells; ++i){
		elements.push_back(Element(i, i, 4));
		//Set the cell-cell interaction values for the cells up/down/left/right of our cell
		//note that this simulation is currently using wrapping boundaries so we just wrap edges
		//in cellNumber
		int x, y;
		cellPos(i, x, y);
		elements.push_back(Element(i, cellNumber(x - 1, y), -1));
		elements.push_back(Element(i, cellNumber(x + 1, y), -1));
		elements.push_back(Element(i, cellNumber(x, y - 1), -1));
		elements.push_back(Element(i, cellNumber(x, y + 1), -1));
	}
	return SparseMatrix(elements, dim, true);
}
void SimpleFluid::tests(){
	//TODO: too high a grid dimension (128) crashed my video driver, what exactly was the
	//cause of the issue? Too much memory usage maybe?
	//Otherwise the fluid interaction matrix seems to solve really fast (<20 iterations) even with
	//my slower method, which is very good news
	//TODO: I've noticed that on occasion I the solution fails for some reason and I get back
	//1.#QNAN as the residual length so something bugs out somewhere, but I'm not sure where
	std::srand(std::time(NULL));
	std::vector<float> b;
	for (int i = 0; i < interactionMat.dim; ++i)
		b.push_back(rand());

	std::chrono::high_resolution_clock::time_point start = std::chrono::high_resolution_clock::now();
	std::vector<float> x = conjugateGradient(interactionMat, b, tiny);
	std::chrono::high_resolution_clock::time_point end = std::chrono::high_resolution_clock::now();

	std::cout << "Solving took: " 
		<< std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count()
		<< "ms" << std::endl;
}
void SimpleFluid::testVelocityField(){
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
	//I suppose later I should pass in the window w/h for setting this properly
	glm::mat4 proj = glm::perspective(60.0f, 
		static_cast<float>(window.box().w) /  static_cast<float>(window.box().h), 0.1f, 100.0f);
	glm::mat4 model = glm::scale(0.35f, 0.35f, 1.0f);
	glm::mat4 mvp = proj * view * model;
	prog.uniformMat4x4("mvp", mvp);

	//Make textures to work with
	GL::Texture fieldA("../res/simplefluid/fluid32.png", true, SOIL_FLAG_INVERT_Y);
	GL::Texture fieldB("../res/simplefluid/fluid32.png", true, SOIL_FLAG_INVERT_Y);
	GL::Texture velocity("../res/simplefluid/right_velocity_32.png", true, SOIL_FLAG_INVERT_Y);
	//Output is the advection output texture, which will flip each run
	GL::Texture output = fieldB;

	//Setup our OpenCL data
#ifdef CL_VERSION_1_2
	cl::ImageGL imgA = tiny.imageFromTexture(CL::MEM::READ_WRITE, fieldA);
	cl::ImageGL imgB = tiny.imageFromTexture(CL::MEM::READ_WRITE, fieldB);
	cl::ImageGL imgVel = tiny.imageFromTexture(CL::MEM::READ_ONLY, velocity);
#else
	cl::Image2DGL imgA = tiny.imageFromTexture(CL::MEM::READ_WRITE, fieldA);
	cl::Image2DGL imgB = tiny.imageFromTexture(CL::MEM::READ_WRITE, fieldB);
	cl::Image2DGL imgVel = tiny.imageFromTexture(CL::MEM::READ_ONLY, velocity);
#endif
	std::vector<cl::Memory> glObjs;
	glObjs.push_back(imgA);
	glObjs.push_back(imgB);
	glObjs.push_back(imgVel);

	cl::Program advectProgram = tiny.loadProgram("../res/simplefluid/advectImageField.cl");
	cl::Kernel advect = tiny.loadKernel(advectProgram, "advectImageField");

	//We'll pick an arbitray time step for now
	advect.setArg(0, 1.f / 30.f);
	advect.setArg(1, imgVel);
	advect.setArg(2, imgA);
	advect.setArg(3, imgB);

	int workSize = tiny.preferredWorkSize(advect);
	cl::NDRange local(workSize, workSize);
	cl::NDRange global(dim, dim);

	//We use the run number to decide which field image should be input and which should be output
	//on even runs A is in, B is out and odd runs we flip
	int run = 0;

	Input::Init();
	while (!Input::Quit()){
		Input::PollEvents();
		if (Input::KeyDown(SDL_SCANCODE_ESCAPE))
			Input::Quit(true);

		//Advect the field
		if (run % 2 == 0 || run == 0){
			advect.setArg(2, imgA);
			advect.setArg(3, imgB);
			output = fieldB;
		}
		else {
			advect.setArg(2, imgB);
			advect.setArg(3, imgA);
			output = fieldA;
		}
		glFinish();
		tiny.mQueue.enqueueAcquireGLObjects(&glObjs);
		tiny.runKernel(advect, local, global);
		tiny.mQueue.enqueueReleaseGLObjects(&glObjs);
		tiny.mQueue.finish();
		++run;

		//RENDERING
		window.clear();

		prog.use();
		glBindVertexArray(vao);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, output);
		glDrawElements(GL_TRIANGLES, vao.numElements(), GL_UNSIGNED_SHORT, NULL);

		window.present();
	}
}
int SimpleFluid::cellNumber(int x, int y) const {
	//Wrap coordinates if necessary
	if (x < 0)
		x += dim;
	else if (x >= dim)
		x %= dim;
	if (y < 0)
		y += dim;
	else if (y >= dim)
		y %= dim;

	return (x + y * dim);
}
void SimpleFluid::cellPos(int n, int &x, int &y) const {
	x = n % dim;
	y = (n - x) / dim;
}
