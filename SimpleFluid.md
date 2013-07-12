SimpleFluid Sim Documentation
=

Some documentation on work and functions of the SimpleFluid simulation

CG Solver
-
The Conjugate Gradient solver is split into a kernel for each operation group. These kernels are then
run and the length of the residual read off the device to determine if another iteration should be done.
The kernel code is located in `res/conjugateGradient.cl`. The solver is being tested on the SimpleFluid
cell interaction matrix in the function `SimpleFluid::tests()`.

**TODO:** Speed!

Advection in Velocity Field
-
A rough version of this is shown in the advectImageField kernel, in `res/simplefluid/advectImageField.cl`.
This reads the velocities from a velocity field image where RG corresponds to xy velocities with zero taken
as 127/255. The value relative to this is scaled up some to give us more range. When reading the x and y 
velocities I think the appropriate offsets are made, at least for accounting for the MAC grid structure
but I'm not quite sure about taking into account the filler 34th row. The 34th column should never be read 
since the global work size is based on the grid size, not the velocity image size. However it's necessary to
have a 34x34 image instead of 33x33 so that our texture is a power of 2. The 33x33 requirement comes from
their being 1 extra entry of x/y than the number of grid cells.

This part is still being worked on, but it's kind of functional. I just don't think it's quite right yet.
It's being tested in the function `SimpleFluid::testVelocityField()`.

Applying the Pressure to the Velocity
-
Yet to be done/thought about.
