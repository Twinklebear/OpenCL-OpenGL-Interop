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

	return 0;
}
