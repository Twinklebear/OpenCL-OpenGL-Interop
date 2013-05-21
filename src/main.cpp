#include <cmath>
#include <vector>
#include "sparsematrix.h"
#include "demos.h"

int main(int argc, char** argv){
	CL::TinyCL tiny(CL::DEVICE::GPU);
	
	std::array<float, 8 * 8> mat;
	for (int i = 0; i < mat.size(); ++i)
		mat[i] = i;
	std::cout << "Matrix:";
	logMatrix(mat);

	std::array<float, 8> vec;
	for (int i = 0; i < vec.size(); ++i)
		vec[i] = i;

	std::cout << "Vector: ";
	for (auto i : vec)
		std::cout << i << " ";
	std::cout << std::endl;

	auto res = matrixVecMult(mat, vec, tiny);

	std::cout << "Result: ";
	for (auto i : res)
		std::cout << i << " ";
	std::cout << std::endl;

	return 0;
}
