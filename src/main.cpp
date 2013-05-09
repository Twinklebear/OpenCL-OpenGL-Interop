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

	std::cout << "Computing householder matrix for [-8, 4, 16, 0]\n";
	std::array<float, 4> v = { -8, 4, -16, 0 };
	std::array<float, 16> hMat = householder(v, tiny);
	std::cout << "Householder matrix:\n";
	logMatrix(hMat);

	std::array<float, 4> col = { -17, 18, -8, 0 };
	std::array<float, 4> c = reflect(col, v, tiny);
	std::cout << "New column:\n";
	for (int i = 0; i < 4; ++i)
		std::cout << std::setprecision(4) << std::setw(8) << c[i];
	std::cout << std::endl;

	col[0] = -10;
	col[1] = -32;
	col[2] = -24;
	col[3] = 0;
	c = reflect(col, v, tiny);
	std::cout << "New column:\n";
	for (int i = 0; i < 4; ++i)
		std::cout << std::setprecision(4) << std::setw(8) << c[i];
	std::cout << std::endl;

	return 0;
}
