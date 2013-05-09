#include <array>
#include <iostream>
#include <iomanip>
#include "tinycl.h"

/*
* Various demo/idea test functions live here.
* Each one is stand-alone and may be simply called from main
*/
/*
* This demo will move a texture with some velocity and draw the 
* the texture as it moves in real time
*/
void liveAdvectTexture();
/*
* Perform a dot product between arbitrary sized vectors
* This is not an interop program, just a standard math operation
*/
void bigDot();
/*
* This performs a matrix transpose operation on matrices of 4n x 4n size
*/
void transpose();
//Helper function, print an MxN matrix
void logMatrix(float *mat, size_t m, size_t n);
template<size_t N>
void logMatrix(const std::array<float, N> &mat){
	std::cout << std::setprecision(4) << '\n';
	for (size_t i = 0; i < N; ++i){
		if (i % static_cast<int>(std::sqrt(N)) == 0 && i != 0)
			std::cout << '\n';
		std::cout << std::setw(6) << mat[i] << " ";
	}
	std::cout << "\n";
}
/*
* Transpose the passed in matrix using the tinycl context provided and return
* the transposed matrix
* Note: the matrix must be of size 4nx4n 
*/
template<size_t N>
std::array<float, N> transpose(std::array<float, N> &matrix, CL::TinyCL &tiny){
	cl::Program prog = tiny.LoadProgram("../res/transpose.cl");
	cl::Kernel kernel = tiny.LoadKernel(prog, "transpose");

	size_t matDim = static_cast<size_t>(std::sqrt(N));
	//Setup matrix buffer
	cl::Buffer bufMat = tiny.Buffer(CL::MEM::READ_WRITE, sizeof(float) * N, &matrix[0]);
	//Setup local mem params and the size param
	size_t localMem = tiny.mDevices.at(0).getInfo<CL_DEVICE_LOCAL_MEM_SIZE>();
	cl_uint nBlocks = matDim / 4;

	//Pass kernel arguments
	kernel.setArg(0, bufMat);
	kernel.setArg(1, localMem , NULL);
	kernel.setArg(2, sizeof(nBlocks), &nBlocks);

	//Figure out local and global size
	size_t globalSize = (matDim / 4 * (matDim / 4 + 1)) / 2;
	cl::NDRange global(globalSize);

	tiny.RunKernel(kernel, cl::NullRange, global);

	//Read the transposed matrix result
	std::array<float, N> res;
	tiny.ReadData(bufMat, sizeof(float) * N, &res[0]);
	return res;
}
/*
* Multiply matrix a by matrix b and get back the result, c
* c = a * b
* using the OpenCL context passed
*/
template<size_t N>
std::array<float, N> matrixMult(std::array<float, N> &a, std::array<float, N> &b, CL::TinyCL &tiny){
	//First we need the transpose of b to multiply against, b/c the kernel does the multiplication
	//in blocks summing up dot products of float4 vectors making up each row/col
	std::array<float, N> bTrans = transpose(b, tiny);

	cl::Program prog = tiny.LoadProgram("../res/matrixmult.cl");
	cl::Kernel kernel = tiny.LoadKernel(prog, "matrixMult");

	size_t nRows = static_cast<size_t>(std::sqrt(N));
	//Setup a, b, c matrix buffers
	cl::Buffer aBuf = tiny.Buffer(CL::MEM::READ_ONLY, sizeof(float) * N, &a[0]);
	cl::Buffer bBuf = tiny.Buffer(CL::MEM::READ_ONLY, sizeof(float) * N, &bTrans[0]);
	cl::Buffer cBuf = tiny.Buffer(CL::MEM::WRITE_ONLY, sizeof(float) * N);

	kernel.setArg(0, aBuf);
	kernel.setArg(1, bBuf);
	kernel.setArg(2, cBuf);

	tiny.RunKernel(kernel, cl::NullRange, cl::NDRange(nRows));

	//Read the multiplication result
	std::array<float, N> res;
	tiny.ReadData(cBuf, sizeof(float) * N, &res[0]);
	return res;
}
