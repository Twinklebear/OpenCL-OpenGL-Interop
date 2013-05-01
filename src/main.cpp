#include <array>
#include "demos.h"

int main(int argc, char** argv){
	std::array<float, 4*4> mat = {
		1, 2, 3, 4,
		5, 6, 7, 8,
		9, 10, 11, 12,
		13, 14, 15, 16
	};
	CL::TinyCL tiny(CL::DEVICE::GPU);
	transpose(mat, tiny);

	std::array<float, 8*8> mat2;
	for (int i = 0; i < mat2.size(); ++i)
		mat2[i] = i;
	transpose(mat2, tiny);

	return 0;
}
