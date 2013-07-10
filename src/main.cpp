#include <window.h>
#include "simplefluid.h"

int main(int argc, char** argv){
	Window::Init();
	Window window("SimpleFluid tests");

	SimpleFluid fluid(32);
	fluid.tests();

	window.Close();
	Window::Quit();

	return 0;
}
