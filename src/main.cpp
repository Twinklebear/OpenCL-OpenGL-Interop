#include <vector>
#include <chrono>
#include "sparsematrix.h"
#include "demos.h"

int main(int argc, char** argv){
	CL::TinyCL tiny(CL::DEVICE::GPU);
	
	SparseMatrix sMat("../res/bcsstk05.mtx");
	std::vector<float> b;
	for (int i = 0; i < sMat.dim; ++i)
		b.push_back(i);

	//Compare my kernel with the book kernel to make sure it's correct
	std::vector<float> localRes, myRes;

	//Measure elapsed time for my kernel and book kernel
	std::cout << "Book CG:\n";
	std::chrono::high_resolution_clock::time_point start = std::chrono::high_resolution_clock::now();
	localRes = localConjGradSolve(sMat, b, tiny);
	std::chrono::high_resolution_clock::duration bookTime = std::chrono::high_resolution_clock::now() - start;

	std::cout << "------\nMy CG:\n";
	start = std::chrono::high_resolution_clock::now();
	myRes = conjugateGradient(sMat, b, tiny);
	std::chrono::high_resolution_clock::duration myTime = std::chrono::high_resolution_clock::now() - start;

	std::cout << "-----\nTime difference, mine - book: " 
		<< std::chrono::duration_cast<std::chrono::milliseconds>(myTime - bookTime).count()
		<< "ms" << std::endl;
	
	//If the results are differ at a digit higher than the some minimal
	//error then my implementation is wrong
	float avgDiff = 0, maxDif = 1e-6;
	int nDifferent = 0;
	for (int i = 0; i < localRes.size(); ++i){
		float diff = std::abs(localRes.at(i) - myRes.at(i));
		if (diff > maxDif){
			avgDiff += diff;
			++nDifferent;
		}
	}
	if (nDifferent != 0)
		avgDiff /= nDifferent;

	std::cout << "# of values differing by more than " << std::scientific << maxDif
		<< " : " << nDifferent << " of " << myRes.size()
		<< "\nAverage difference between values: " 
		<< avgDiff << std::endl;

	return 0;
}
