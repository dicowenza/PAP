
__kernel void addmat(__global float *A,
		     __global float *B,
		     __global float *C)
{
	// TODO
	int x =get_global_id(0);
	int y =get_global_id(1);

	C[x*SIZE+y]=A[x*SIZE+y]+B[x*SIZE+y];
	
}
