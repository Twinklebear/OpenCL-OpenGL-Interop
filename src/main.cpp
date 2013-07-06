#include <iostream>
#include "sparsematrix.h"
#include "simplefluid.h"
#include "demos.h"

void testSimpleFluidMatrix(){
	SimpleFluid test(3);
	std::cout << test.interactionMat.print() << std::endl;
}

int main(int argc, char** argv){
	testSimpleFluidMatrix();
	return 0;
}
