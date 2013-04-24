/*
* Various demo/idea test functions live here.
* Each one is stand-alone and may be simply called from main
*/
/*
* This demo will move a texture with some velocity and draw the 
* the texture as it moves in real time
*/
void liveAdvectTexture();
/*
* Perform a dot product between arbitrary sized vectors
* This is not an interop program, just a standard math operation
*/
void bigDot();
/*
* This performs a matrix transpose operation on matrices of 4n x 4n size
*/
void transpose();
//Helper function, print an MxN matrix
void logMatrix(float *mat, size_t m, size_t n);