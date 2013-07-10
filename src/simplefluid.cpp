#include <iostream>
#include <cmath>
#include <ctime>
#include <chrono>
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

SimpleFluid::SimpleFluid(int dim)
	//Should be interop context, but false for now since I'm just testing setting up the interaction matrix
	//and solving it
	: tiny(CL::DEVICE::GPU, true), dim(dim), interactionMat(generateMatrix())
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
