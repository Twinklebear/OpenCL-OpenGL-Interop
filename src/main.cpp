#include <array>
#include "demos.h"

int main(int argc, char** argv){
	const size_t matSize = 4 * 4;
	std::array<float, matSize> mat;
	for (int i = 0; i < matSize; ++i)
		mat[i] = i;

	CL::TinyCL tiny(CL::DEVICE::GPU);
	std::array<float, matSize> res = transpose(mat, tiny);
	std::cout << "Initial matrix:\n";
	logMatrix(mat);
	std::cout << "Transposed matrix:\n";
	logMatrix(res);

	std::array<float, matSize> multRes = matrixMult(mat, res, tiny);
	std::cout << "Multiplication result:\n";
	logMatrix(multRes);

	std::cout << "Computing householder matrix for [0, 5, 0, 0]\n";
	std::array<float, 4> v = { 0, 5, 0, 0 };
	std::array<float, 16> hMat = householder(v, tiny);
	std::cout << "Householder matrix:\n";
	logMatrix(hMat);

	return 0;
}
