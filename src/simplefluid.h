#ifndef SIMPLEFLUID_H
#define SIMPLEFLUID_H

#include <glm/glm.hpp>
#include <window.h>
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
	* @param dim Dimensions of the square grid to simulate on (dimXdim)
	* @param window Window to draw the simulation too
	*/
	SimpleFluid(int dim, Window &window);
	/**
	* Function to run various tests on the simple fluid instance
	* this is really just while I'm working out the kinks and testing things individually
	*/
	void tests();
	//Function to test the velocity field advection
	void testVelocityField();

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
	//The window being drawn too
	Window &window;
	//The quad to be used for drawing the fluid texture and velocity textures on to
	const static std::array<glm::vec3, 8> quad;
	//The element buffer to draw the quad
	const static std::array<unsigned short, 6> quadElems;
};

#endif
