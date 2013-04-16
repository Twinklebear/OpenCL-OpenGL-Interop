//This is a Runge-Kutta 4 implementation in OpenCL
//for use in the fluid solver

//The advection solver, currently just solving for position
//x is the array of values vs time, u is the velocity field, for now I'll leave it constant
//and only along x axis
//dt is the time step, n is the total number of steps to run
__kernel void solveRK(__global float *x, __global float *u, 
	__global float *dt, __global int *n)
{
	//assume that x[1] has been set already
	//For now just track position vs time in this velocity field
	//unsigned int i;
	for (int i = 1; i < (*n); ++i){
		//Becuase u is constant everywhere we don't lookup the value of u at q
		float xA = x[i - 1] - 0.5f * (*dt) * (*u);
		x[i] = x[i - 1] - (*dt) * (*u); 
	}
	//second order
	//q_n+1/2 = q_n + 0.5 * dt * fcn(q_n);
	//q_n+1 = q_n + dt * fcn(q_n+1/2);
	//but what is the function?
	//Dq/Dt = 0 is the equation
	//The equation in book for a non-vector quantity:
	//dq/dt + grad q dot u = 0
	//so
	//dq/dt = -grad q dot u
	//and we solve this differential equation with RK?

	//for vector quantities it's similar
	//Dq/Dt = dq/dt + u dot grad q = 0?
	//But who's the function? to use in the RK solution, fcn(q_n)
	//Book mentions for tracing an imaginary particle 
	//dx/dt = u(x)
	//and because we're tracing back we use -
	//x_n+1/2 = x_n - 0.5 * dt * u(x)
	//x_n+1 = x_n - dt * u(x_n+1/2)
	//but what's this u(x) function?
	//Is it just meaning the value of velocity field at point x?
}
