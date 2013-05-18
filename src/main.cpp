#include <array>
#include "sparsematrix.h"
#include "demos.h"

int main(int argc, char** argv){
	SparseMatrix matrix("../res/bcsstk05.mtx", false);

	//Now try getting the raw values
	int *row = new int[matrix.elements.size()];
	int *col = new int[matrix.elements.size()];
	float *val = new float[matrix.elements.size()];
	matrix.getRaw(row, col, val);

	for (size_t i = 0; i < matrix.elements.size(); ++i)
		std::cout << row[i] << " " << col[i] << " " << val[i] << "\n";
	std::cout << std::endl;

	return 0;
}
