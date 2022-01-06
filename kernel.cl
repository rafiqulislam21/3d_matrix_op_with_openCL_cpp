__kernel void ThreeDimArray(__global float *const A) {
  const int k = get_global_id(0);
  const int i = get_global_id(1);
  const int j = get_global_id(2);
  //const int depth  = get_global_size(0); // unused here
  const int row    = get_global_size(1);
  const int column = get_global_size(2);
  
  const int idx = k * row * column + i * column + j; // linear index to access matrix A in 1D form

  if(k == 0) {
      A[idx] = (float)i / ((float)j + 1.00f);
  } else if(k == 1) {
      A[idx] = 1.00;
  } else {
      A[idx] = (float)j / ((float)i + 1.00f);
  }
};


// // OpenCL kernel. Each work item takes care of one element of c
// #pragma OPENCL EXTENSION cl_khr_fp64 : enable
// __kernel void vecAdd(__global double *a, __global double *b, __global double *c,
//                      const unsigned int n) {
//   // Get our global thread ID
//   int id = get_global_id(0);

//   // Make sure we do not go out of bounds
//   if (id < n)
//     c[id] = a[id] + b[id];
// }


// __kernel void matrix(__global double *c, const unsigned int dim1,
//                      const unsigned int dim2) {
//   // Get our global thread ID
//   int i = get_global_id(0);
//   int j = get_global_id(1);
//   int k = get_global_id(2);

//   // Make sure we do not go out of bounds
//   if (id < n)
//     int idx = i + dim1 * j + dim1 * dim2 * k;
//     c[idx] = idx;
// }


// __kernel void ThreeDimArray(__global float *const output1) {
//   const int x = get_global_id(0);
//   const int y = get_global_id(1);
//   const int z = get_global_id(2);
//   const int max_x = get_global_size(0);
//   const int max_y = get_global_size(1);
//   const int max_z = get_global_size(2);
  
//   const int idx = x * max_y * max_z + y * max_z + z;

//   output1[idx] = 1.00;
// };