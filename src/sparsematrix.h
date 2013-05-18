#ifndef SPARSEMATRIX_H
#define SPARSEMATRIX_H

#include <string>
#include <vector>

/**
* Defines an entry in the sparse matrix, containing
* information about the row, column and value
* also provides comparison functions for comparing row/col
*/
struct Element {
	int row, col;
	float val;

	//Get a diagonal version of the element, ie. with row and col switched
	Element diagonal();
};
//True if lhs row is lower than rhs
bool lowerRow(const Element &lhs, const Element &rhs);
//True if this col is lower than rhs
bool lowerCol(const Element &lhs, const Element &rhs);

/**
* A sparse matrix, can be loaded directly from a matrix market
* matrix file
*/
class SparseMatrix {
public:
	/**
	* Load the matrix from a matrix market file, rowMaj true if we want to 
	* sort by row ascending, ie. row major, currently only support
	* loading from coordinate real symmetric matrices
	*/
	SparseMatrix(const std::string &file, bool rowMaj = true);
	//Get the underlying row, column and value arrays for use in passing to OpenCL
	void getRaw(int *row, int *col, float *val);

private:
	//Parse and load a matrix from a matrix market file
	void loadMatrix(const std::string &file, bool rowMaj = true);

public:
	std::vector<Element> elements;
	bool symmetric;
};

#endif
