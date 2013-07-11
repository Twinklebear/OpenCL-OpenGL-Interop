#include <window.h>
#include "simplefluid.h"

int main(int argc, char** argv){
	Window::init();
	Window window("SimpleFluid tests");

	SimpleFluid fluid(32);
	fluid.tests();

	window.close();
	Window::quit();

	return 0;
}
