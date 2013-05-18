#include <string>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <iostream>
#include "sparsematrix.h"

Element Element::diagonal(){
	Element e;
	e.row = col;
	e.col = row;
	e.val = val;
	return e;
}
//True if lhs row is lower than rhs
bool lowerRow(const Element &lhs, const Element &rhs){
	return lhs.row < rhs.row;
}
//True if this col is lower than rhs
bool lowerCol(const Element &lhs, const Element &rhs){
	return lhs.col < rhs.col;
}
/**
* Load the matrix from a matrix market file, rowMaj true if we want to 
* sort by row ascending, ie. row major, currently only support
* loading from coordinate real symmetric matrices
*/
SparseMatrix::SparseMatrix(const std::string &file, bool rowMaj){
	loadMatrix(file, rowMaj);
}
//Get the underlying row, column and value arrays for use in passing to OpenCL
void SparseMatrix::getRaw(int *row, int *col, float *val){

}
//Parse and load a matrix from a matrix market file
void SparseMatrix::loadMatrix(const std::string &file, bool rowMaj){
	if (file.substr(file.size() - 3, 3) != "mtx"){
		std::cout << "Error: Not a Matrix Market file: " << file << std::endl;
		return;
	}

	std::ifstream matFile(file.c_str());
	//The first line after end of comments is the M N L information
	bool readComment;
	std::string line;
	while (std::getline(matFile, line)){
		if (line.at(0) == '%'){
			readComment = true;
			//2 %% indicates header information
			if (line.at(1) == '%'){
				if (line.find("coordinate") == -1){
					std::cout << "Error: non-coordinate matrix is unsupported" << std::endl;
					return;
				}
				if (line.find("symmetric") == -1){
					std::cout << "Error: non-symmetric matrix is unsupported" << std::endl;
					return;
				}
				symmetric = true;
			}
		}
		//Non-comments will either be M N L info or matrix elements
		if (line.at(0) != '%'){
			//If this is the first line after comments it's the M N L info
			if (readComment){
				std::stringstream ss(line);
				float m, n, l;
				ss >> m >> n >> l;
				std::cout << "M N L: " << m << " " << n << " " << l << std::endl;
				//l is the number of entries in the file, so there will be at least that many
				//elements
				elements.reserve(l);
			}
			//Otherwise it's element data in the form: row col value
			else {
				Element elem;
				std::stringstream ss(line);
				ss >> elem.row >> elem.col >> elem.val;
				elements.push_back(elem);
				//If the row is symmetric we'll only be given the diagonal and lower-triangular implying that
				//if we're given an off-diagonal: i j v then a corresponding element: j i v should also be inserted
				if (symmetric && elem.row != elem.col)
					elements.push_back(elem.diagonal());
			}
			readComment = false;
		}
	}
	//Select the appropriate sorting for the way we want to treat the matrix, row-maj or col-maj
	//If row major we'll sort by row, if column sort by col, both in ascending order
	if (rowMaj)
		std::sort(elements.begin(), elements.end(), lowerRow);
	else
		std::sort(elements.begin(), elements.end(), lowerCol);

	std::cout << "Matrix size: " << elements.size() << std::endl;
	std::cout << "Matrix:" << std::endl;
	for (std::vector<Element>::const_iterator it = elements.begin(); it != elements.end(); ++it)
		std::cout << it->row << " " << it->col << " " << it->val << "\n";
	std::cout << std::endl;
}
