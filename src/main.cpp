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

	std::array<float, 4 * 3> vec2;
	std::cout << "Vector being dotted with self: ";
	for (int i = 0; i < vec2.size(); ++i){
		vec2[i] = i;
		std::cout << i << ", ";
	}
	std::cout << std::endl;

	std::cout << "Result: " << dot(vec2, vec2, tiny) << std::endl;

	//Try out the sparse matrix mult kernel
	int row[10], col[10];
	float vals[10];
	std::vector<float> vecS;
	for (int i = 0; i < 10; ++i){
		row[i] = i;
		col[i] = i;
		vals[i] = 2;
		vecS.push_back(i);
	}
	std::cout << "Initial vector: ";
	for (auto i : vecS)
		std::cout << i << ", ";
	std::cout << std::endl;

	SparseMatrix simple(row, col, vals, 10);
	std::cout << "SparseMat:\n" << simple << std::endl;

	vecS = sparseVecMult(simple, vecS, tiny);

	std::cout << "Result: ";
	for (auto i : vecS)
		std::cout << i << ", ";
	std::cout << std::endl;


	return 0;
}
