/*
* Multiply a vector by a sparse matrix and get the resultant
* vector.
* The global # of work items will be equal to the number rows in the
* sparse matrix, which is assumed to be square (what would happen if it wasn't?)
*/
__kernel void sparseMatVec(int nVals, __global int *rows, __global int *cols, 
	__global float *vals, __global float *vec, __global float *res)
{
	int id = get_global_id(0);
	//We use these to select the range of i to read our row's values from (row # == id)
	int startIdx = -1;
	int endIdx = -1;
	//Determine the indices containing the values for the row this kernel will be working on
	for (int i = id; i < nVals; ++i){
		if (rows[i] == id && startIdx == -1)
			startIdx = i;
		if (rows[i] == id + 1 && startIdx != -1 && endIdx == -1){
			endIdx = i - 1;
			break;
		}
		if (i == nVals - 1 && startIdx != -1 && endIdx == -1)
			endIdx = i;
	}
	float sum = 0.0f;
	for (int i = startIdx; i <= endIdx; ++i)
			sum += vals[i] * vec[cols[i]];

	res[id] = sum;
}