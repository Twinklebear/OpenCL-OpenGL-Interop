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

	std::cout << "b sizE: " << b.size() << std::endl;

	CL::TinyCL tiny(CL::DEVICE::GPU);
	conjGradSolve(matrix, b, tiny);

	return 0;
}
