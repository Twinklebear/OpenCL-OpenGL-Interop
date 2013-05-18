#include <array>
#include "sparsematrix.h"
#include "demos.h"

int main(int argc, char** argv){
	SparseMatrix matrix("../res/bcsstk01.mtx");

	//Now try getting the raw values
	int *row = new int[matrix.elements.size()];
	int *col = new int[matrix.elements.size()];
	float *val = new float[matrix.elements.size()];
	matrix.getRaw(row, col, val);

	delete[] row;
	delete[] col;
	delete[] val;

	return 0;
}
