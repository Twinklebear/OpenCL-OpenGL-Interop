#ifndef SIMPLEFLUID_H
#define SIMPLEFLUID_H

#include "tinycl.h"
#include "sparsematrix.h"

/**
* A simple 2d fluid simulation using a 2d MAC grid
*/
class SimpleFluid {
public:
	/**
	* Create the simulation, specifying the dimensions for the simulation grid
	* the class will instantiate its own CL GPU context
	* @param dim dimensions of the square grid to simulate on (dimXdim)
	*/
	SimpleFluid(int dim);
	/**
	* Function to run various tests on the simple fluid instance
	* this is really just while I'm working out the kinks and testing things individually
	*/
	void tests();

private:
	/**
	* Generate the sparse matrix describing the cell-cell interactions for a 2d fluid simulation
	* The matrix is cells^2 X cells^2 with all diagonal elements 4 and elements representing 
	* interacting cells set to -1
	*/
	SparseMatrix generateMatrix();
	/**
	* Compute the cell number of the cell at the given coordinates where
	* x and y are the cell x,y indices and the number is given treating the grid
	* in row major order. ie. cell 0,0 is #0, cell 1,0 is #1
	* the x/y will also be wrapped appropriately along the row/column to simulate
	* wrapping boundary conditions, ie. on a 2x2 grid reading cell 2,0 will wrap to 0,0 and
	* return that index
	*/
	int cellNumber(int x, int y) const;
	/**
	* Compute the x & y values of some cell number taking the grid in row-major order
	* @param n The cell number
	* @param x Output of the x value
	* @param y Output of the y value
	*/
	void cellPos(int n, int &x, int &y) const;

private:
	CL::TinyCL tiny;
	int dim;
	SparseMatrix interactionMat;
};

#endif
