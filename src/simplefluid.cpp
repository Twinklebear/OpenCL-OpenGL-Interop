#include "tinycl.h"
#include "sparsematrix.h"
#include "simplefluid.h"

SimpleFluid::SimpleFluid(int dim)
	//Should be interop context, but false for now since I'm just testing setting up the interaction matrix
	//and solving it
	: tiny(CL::DEVICE::GPU, false), dim(dim), interactionMat(generateMatrix())
{
}
SparseMatrix SimpleFluid::generateMatrix(){
	std::vector<Element> elements;
	int cells = std::pow(dim, 2);
	//All diagonal entries of the matrix are 4
	for (int i = 0; i < cells; ++i){
		elements.push_back(Element(i, i, 4));
		//Set the cell-cell interaction values for the cells up/down/left/right of our cell
		//note that this simulation is currently using wrapping boundaries so we just wrap edges
		//in cellNumber
		int x, y;
		cellPos(i, x, y);
		elements.push_back(Element(i, cellNumber(x - 1, y), -1));
		elements.push_back(Element(i, cellNumber(x + 1, y), -1));
		elements.push_back(Element(i, cellNumber(x, y - 1), -1));
		elements.push_back(Element(i, cellNumber(x, y + 1), -1));
	}
	return SparseMatrix(elements, dim, true);
}
int SimpleFluid::cellNumber(int x, int y) const {
	//Wrap coordinates if necessary
	if (x < 0)
		x += dim;
	else if (x >= dim)
		x %= dim;
	if (y < 0)
		y += dim;
	else if (y >= dim)
		y %= dim;

	return (x + y * dim);
}
void SimpleFluid::cellPos(int n, int &x, int &y) const {
	x = n % dim;
	y = (n - x) / dim;
}
