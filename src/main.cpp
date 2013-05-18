#include <cmath>
#include <vector>
#include "sparsematrix.h"
#include "demos.h"

int main(int argc, char** argv){
	SparseMatrix matrix("../res/bcsstk01.mtx");
	
	std::srand(NULL);
	std::vector<float> b;
	b.reserve(matrix.dim);
	for (int i = 0; i < matrix.dim; ++i)
		b.push_back(std::rand() - RAND_MAX / 2.0f);

	CL::TinyCL tiny(CL::DEVICE::GPU);
	std::vector<float> x = conjGradSolve(matrix, b, tiny);
	std::cout << "Solved x vector:\n";
	for (int i = 0; i < matrix.dim; ++i)
		std::cout << x[i] << "\n";
	std::cout << std::endl;

	return 0;
}
