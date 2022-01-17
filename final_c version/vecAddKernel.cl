__kernel void addVectors(__global const float *a, 
	__global const float *b,
	__global float *c) {
		
		int gid = get_global_id(0);
		c[gid] = c[gid] + (float)(1 / (native_sqrt(a[gid] + b[gid])));
	}