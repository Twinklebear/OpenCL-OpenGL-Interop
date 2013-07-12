#include <window.h>
#include "demos.h"
#include "simplefluid.h"

int main(int argc, char** argv){
	Window::init();
	Window window("SimpleFluid tests");

	SimpleFluid fluid(32, window);
	fluid.testVelocityField();

	window.close();
	Window::quit();

	return 0;
}
