#include <iostream>
#include <cmath>
#include <chrono>
#include "sparsematrix.h"
#include "simplefluid.h"
#include "demos.h"

void testSimpleFluidMatrix(){
	//TODO: too high a grid dimension (128) crashed my video driver, what exactly was the
	//cause of the issue? Too much memory usage maybe?
	//Otherwise the fluid interaction matrix seems to solve really fast (<20 iterations) even with
	//my slower method, which is very good news
	SimpleFluid test(32);
	//Try out CG solve on the matrix with a random b vector
	std::srand(std::time(NULL));
	std::vector<float> b;
	for (int i = 0; i < test.interactionMat.dim; ++i)
		b.push_back(rand());

	std::chrono::high_resolution_clock::time_point start = std::chrono::high_resolution_clock::now();
	std::vector<float> x = conjugateGradient(test.interactionMat, b, test.tiny);
	std::chrono::high_resolution_clock::time_point end = std::chrono::high_resolution_clock::now();

	std::cout << "Solving took: " 
		<< std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count()
		<< "ms" << std::endl;
}

int main(int argc, char** argv){
	testSimpleFluidMatrix();
	return 0;
}
