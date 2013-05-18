#include <array>
#include <iostream>
#include <iomanip>
#include <memory>
#include "sparsematrix.h"
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
//Perform a basic computation using an OpenGL Compute shader instead of OpenCL
void openglCompute();
//Log a matrix to console
template<size_t N>
void logMatrix(const std::array<float, N> &mat){
	std::cout << std::setprecision(4) << '\n';
	for (size_t i = 0; i < N; ++i){
		if (i % static_cast<int>(std::sqrt(N)) == 0 && i != 0)
			std::cout << '\n';
		std::cout << std::setw(8) << mat[i] << " ";
	}
	std::cout << "\n";
}
/*
* Transpose the passed in matrix using the tinycl context and return the
* cl::Buffer containing the result. This should be used if the result 
* of the operation is needed for future OpenCL kernels
* Note: the matrix must be of size 4nx4n 
*/
template<size_t N>
cl::Buffer transposeBuf(std::array<float, N> &matrix, CL::TinyCL &tiny){
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
	return bufMat;
}
/*
* Transpose the passed in matrix using the tinycl context provided and return
* the transposed matrix
* Note: the matrix must be of size 4nx4n 
*/
template<size_t N>
std::array<float, N> transpose(std::array<float, N> &matrix, CL::TinyCL &tiny){
	cl::Buffer bufMat = transposeBuf(matrix, tiny);
	//Read and return
	std::array<float, N> res;
	tiny.ReadData(bufMat, sizeof(float) * N, &res[0]);
	return res;
}
/*
* Multiply matrix a by matrix b and get back the result, c
* c = a * b
* using the OpenCL context passed and return the cl::Buffer containing the result
* this should be used if the result of the operation is needed for future OpenCL use
*/
template<size_t N>
cl::Buffer matrixMultBuf(std::array<float, N> &a, std::array<float, N> &b, CL::TinyCL &tiny){
	cl::Program prog = tiny.LoadProgram("../res/matrixmult.cl");
	cl::Kernel kernel = tiny.LoadKernel(prog, "matrixMult");

	size_t nRows = static_cast<size_t>(std::sqrt(N));
	//Setup a, b, c matrix buffers
	cl::Buffer aBuf = tiny.Buffer(CL::MEM::READ_ONLY, sizeof(float) * N, &a[0]);
	cl::Buffer bBuf = transposeBuf(b, tiny);
	cl::Buffer cBuf = tiny.Buffer(CL::MEM::WRITE_ONLY, sizeof(float) * N);

	kernel.setArg(0, aBuf);
	kernel.setArg(1, bBuf);
	kernel.setArg(2, cBuf);

	tiny.RunKernel(kernel, cl::NullRange, cl::NDRange(nRows));
	return cBuf;
}
/*
* Multiply matrix a by matrix b and get back the result, c
* c = a * b
* using the OpenCL context passed
*/
template<size_t N>
std::array<float, N> matrixMult(std::array<float, N> &a, std::array<float, N> &b, CL::TinyCL &tiny){
	cl::Buffer bufMat = matrixMultBuf(a, b, tiny);
	//Read and return
	std::array<float, N> res;
	tiny.ReadData(bufMat, sizeof(float) * N, &res[0]);
	return res;
}
/*
* Compute Householder matrix for vector4 but return the cl::Buffer
* this should be used if the result of the computation is needed in future
* kernels to avoid reading/writing back and forth to the GPU
*/
cl::Buffer householderBuf(std::array<float, 4> vect, CL::TinyCL &tiny);
/*
* Compute the Householder matrix for a vector4
*/
std::array<float, 16> householder(std::array<float, 4> vect, CL::TinyCL &tiny);
/*
* Reflect a vector v using the Householder matrix of u and get back the buffer
* containing the result
*/
cl::Buffer reflectBuf(std::array<float, 4> v, std::array<float, 4> u, CL::TinyCL &tiny);
/*
* Reflect a vector v using the Householder matrix of u and get back the result
*/
std::array<float, 4> reflect(std::array<float, 4> v, std::array<float, 4> u, CL::TinyCL &tiny);
/*
* Solve a linear system Ax = b (where A is sparse) using the Conjugate Gradient method
* and return the solved x vector. x could be really big so we return a shared_ptr
*/
std::shared_ptr<std::vector<float>>
conjGradSolve(const SparseMatrix &matrix, std::vector<float> bVec, CL::TinyCL &tiny);
