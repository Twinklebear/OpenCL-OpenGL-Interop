/*
* Solve a symmetric sparse matrix system using the Conjugate Gradient method
* Each kernel works on a single row of values in the matrix @ row = id
* and updates the values x[id], r[id] and p[id]
* @param dim Dimension of the matrix we're trying to solve (square only)
* @param nVals Number of non-zero values in the sparse matrix
* @param r The residual vector. Each kernel only updates r[id] but makes use
* 	of everyone's results to compute r dot r needed for finding p[id]
* @param x The result vector, each kernel works on x[id] and sums in the
*	results of the iterations, each loop: x[id] += alpha * p[id]
* @param Atimsp The result of Ap, each kernel only computes the value at its
*	id, but uses the result of everyones work to find p dot Ap
* @param p The current search direction, again each kernel updates the value @ id
*	but uses the result of all kernels updates each step for the direction
* @param rows The row indices of the non-zero matrix elements
* @param cols The col indices of the non-zero matrix elements
* @param A The sparse matrix values. The matrix is read like so:
*	non-zero entry 2: row[2] = 1, col[2] = 5, val[2] = 15.4
* @param b The right side of the system Ax = b
* @param result The final iteration count, residual lenght and x vector are
*	written to this buffer so that the host can read back information about the solution
*	along with the solution itself
*/
__kernel void conjGrad(int dim, int nVals, __local float *r, __local float *x,
	__local float *Atimesp, __local float *p, __global int *rows, __global int *cols,
	__global float *A, __global float *b, __global float *result)
{
	//Todo: the book uses local id, and runs everything as one work group
	//which is ok, but what if the matrix is bigger dimensions than max work group size?
	//what if I want to use the preferred work group size?
	int id = get_local_id(0);
	//We use these to select the range of i to read row/col from
	int startIdx = -1;
	int endIdx = -1;
	//Determine the indices containing the values for the row this kernel will be working on
	for (int i = id; i < nVals; ++i){
		if (rows[i] == id && startIdx == -1)
			startIdx = i;
		else if (rows[i] == id + 1 && endIdx == -1){
			endIdx = i - 1;
			break;
		}
		else if (i == nVals - 1 && endIdx == -1)
			endIdx = i;
	}

	//Set initial guess, residual and dir
	x[id] = 0.0f;
	r[id] = b[id];
	p[id] = b[id];
	//Wait for everyone to get setup
	barrier(CLK_LOCAL_MEM_FENCE);

	//What if instead of only having the lead kernel do this
	//we used a seperate kernel that would more evenly divide the work?
	//Compute the initial dot product and length of r in the lead kernel
	local float oldRdotR, rLen;
	if (id == 0){
		oldRdotR = 0.0f;
		for (int i = 0; i < dim; ++i)
			oldRdotR += r[i] * r[i];

		rLen = sqrt(oldRdotR);
	}
	barrier(CLK_LOCAL_MEM_FENCE);

	//Do the CG iterations
	local float alpha, newRdotR, ApDotp;
	local int iteration;
	iteration = 0;
	while (iteration < 1000 && rLen >= 0.01f){
		Atimesp[id] = 0.0f;
		for (int i = startIdx; i <= endIdx; ++i)
			Atimesp[id] += A[i] * p[cols[i]];

		barrier(CLK_LOCAL_MEM_FENCE);

		//Lead kernel computes alpha
		if (id == 0){
			ApDotp = 0.0f;
			for (int i = 0; i < dim; ++i)
				ApDotp += Atimesp[i] * p[i];

			alpha = oldRdotR / ApDotp;
		}
		barrier(CLK_LOCAL_MEM_FENCE);

		//Determine next guess & residual this work item will investigate
		x[id] += alpha * p[id];
		r[id] -= alpha * Atimesp[id];
		barrier(CLK_LOCAL_MEM_FENCE);

		//Lead kernel computes r dot r, again could this benefit
		//from being split into its own kernel for even work distribution?
		//Or would the overhead of running another kernel outweight benefit?
		if (id == 0){
			newRdotR = 0.0f;
			for (int i = 0; i < dim; ++i)
				newRdotR += r[i] * r[i];

			rLen = sqrt(newRdotR);
		}
		barrier(CLK_LOCAL_MEM_FENCE);

		//Update dir
		p[id] = r[id] + (newRdotR / oldRdotR) * p[id];
		barrier(CLK_LOCAL_MEM_FENCE);

		oldRdotR = newRdotR;

		//Lead kernel incremenents the iteration count and we wait for everyone to finish
		//their iteration
		if (id == 0)
			++iteration;

		barrier(CLK_LOCAL_MEM_FENCE);
	}
	result[0] = iteration * 1.0f;
	result[1] = rLen;
	//We also want access to the result so write that out as well
	//for the final kernel I want to use in the sim I'll probably skip 
	//outputting the iteration count and rLen
	result[id + 2] = x[id];
}
