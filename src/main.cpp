#include <cmath>
#include <vector>
#include "sparsematrix.h"
#include "demos.h"

int main(int argc, char** argv){
	CL::TinyCL tiny(CL::DEVICE::GPU);
	
	const size_t nRow = 4;
	std::array<float, nRow * nRow> mat;
	for (int i = 0; i < mat.size(); ++i)
		mat[i] = i;
	std::cout << "Matrix:";
	logMatrix(mat);

	std::array<float, nRow> vec;
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

	int rows[8] = { 0, 1, 2, 3, 4, 5, 6, 7 };
	int cols[8] = { 0, 1, 2, 3, 4, 5, 6, 7 };
	float vals[8] = { 1, 1, 1, 1, 1, 1, 1, 1 };
	SparseMatrix sMat(rows, cols, vals, 8);

	return 0;
}
