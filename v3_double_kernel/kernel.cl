__kernel void ThreeDimArray(__global float *const A) {
  const int k = get_global_id(0);
  const int i = get_global_id(1);
  const int j = get_global_id(2);
  // const int depth  = get_global_size(0); // unused here
  const int row = get_global_size(1);
  const int column = get_global_size(2);
  const int idx = k * row * column + i * column + j; // linear index to

  // matrix initialization part
  // access matrix A in 1D form
  //------------optimization test faster---------------------------------------
 
  if (k == 0) {
    A[idx] = (float)i / ((float)j + 1.00);
  } else if (k == 1) {
    A[idx] = 1.00;
  } else {
    A[idx] = (float)j / ((float)i + 1.00);
  }

  // barrier(CLK_LOCAL_MEM_FENCE | CLK_GLOBAL_MEM_FENCE);

  // // iteration part
  // if (i >= 1 && i <= row - 1) {
  //   const int idk0i1 = 0 * row * column + (i + 1) * column + j; // indexes for k=0 and i+1
  //   const int idk2i1 = 2 * row * column + (i - 1) * column + j; // indexes for k=2 and i-1
  //   if (k == 1) {
  //       // A_seq[1][i][j] = A_seq[1][i][j] + (1 / (sqrt(A_seq[0][i + 1][j] + A_seq[2][i - 1][j])));
  //       A[idx] = A[idx] + (float)(1 / (native_sqrt(A[idk0i1] + A[idk2i1])));
  //   }
  // }
  //------------optimization test faster---------------------------------------

  //------------accurate but slower---------------------------------------
  // const int idk0 = 0 * row * column + i * column + j; // indexes for k=0
  // const int idk1 = 1 * row * column + i * column + j; // indexes for k=1
  // const int idk2 = 2 * row * column + i * column + j; // indexes for k=2

  // A[idk0] = (float)i / ((float)j + 1.00);
  // A[idk1] = 1.00;
  // A[idk2] = (float)j / ((float)i + 1.00);

  // barrier(CLK_GLOBAL_MEM_FENCE);

  // // iteration part
  // if(i >= 1 && i <= row-1){
  //   const int idk0i1 = 0 * row * column + (i+1) * column + j; // indexes for k=0 and i+1 
  //   const int idk2i1 = 2 * row * column + (i-1) * column + j; //indexes for k=2 and i-1 
  //   for (int t = 0; t < 24; t++) {
  //     // A_seq[1][i][j] = A_seq[1][i][j] + (1 / (sqrt(A_seq[0][i + 1][j] +A_seq[2][i - 1][j]))); 
  //     A[idk1] = A[idk1] + (float)(1 / (native_sqrt(A[idk0i1] + A[idk2i1])));
  //   }
  // }
  //------------accurate but slower---------------------------------------
};


__kernel void iterations(__global float *const A) {
  const int k = get_global_id(0);
  const int i = get_global_id(1);
  const int j = get_global_id(2);
  // const int depth  = get_global_size(0); // unused here
  const int row = get_global_size(1);
  const int column = get_global_size(2);
  // const int idx = k * row * column + i * column + j; // linear index to

  //------------accurate but slower---------------------------------------
  // const int idk0 = 0 * row * column + i * column + j; // indexes for k=0
  const int idk1 = 1 * row * column + i * column + j; // indexes for k=1
  // const int idk2 = 2 * row * column + i * column + j; // indexes for k=2
  // iteration part
  if(i >= 1 && i <= row-1){
    const int idk0i1 = 0 * row * column + (i+1) * column + j; // indexes for k=0 and i+1 
    const int idk2i1 = 2 * row * column + (i-1) * column + j; //indexes for k=2 and i-1 
      // A_seq[1][i][j] = A_seq[1][i][j] + (1 / (sqrt(A_seq[0][i + 1][j] +A_seq[2][i - 1][j]))); 
      A[idk1] = A[idk1] + (float)(1 / (native_sqrt(A[idk0i1] + A[idk2i1])));
  }
  //------------accurate but slower---------------------------------------
};









// try 1d version to optimized
// __local float *const A_local = A;

// optimized kernel
// __kernel void ThreeDimArray(__global float *A, const int depth, const int
// row,
//                             const int column) {
//   const int idx = get_global_id(0); // 1D kernel range is depth*row*column
//   const int t = n % (column * row), j = t % column, i = t / column,
//             k = n / (column * row);
//   A[idx] = k == 0   ? (float)i / ((float)j + 1.0f)
//            : k == 1 ? 1.0f
//                     : (float)j / ((float)i + 1.0f);
// };





// __kernel void vecKernel(__global float *const A) {
//   const int k = get_global_id(0);
//   const int i = get_global_id(1);
//   const int j = get_global_id(2);
//   // const int depth  = get_global_size(0);
//   const int row = get_global_size(1);
//   const int column = get_global_size(2);
//   const int idx = k * row * column + i * column + j; // linear index to

//   // matrix initialization part
//   if (k == 0) {
//     A[idx] = (float)i / ((float)j + 1.00);
//   } else if (k == 1) {
//     A[idx] = 1.00;
//   } else {
//     A[idx] = (float)j / ((float)i + 1.00);
//   }

//   barrier(CLK_LOCAL_MEM_FENCE | CLK_GLOBAL_MEM_FENCE);

//   // iteration part
//   if (i >= 1 && i <= row - 1) {
//     const int idk0i1 = 0 * row * column + (i + 1) * column + j; // indexes for k=0 and i+1
//     const int idk2i1 = 2 * row * column + (i - 1) * column + j; // indexes for k=2 and i-1
//     if (k == 1) {
//       //no loops here only cell update
//       // You write e for loop in the C file that wraps clEnqueue NDRange kernel (and/or writing/reading from gpu).
//       // In the kernel part you write only the code from the formula whixh updates matrix cells.

//       for (int t = 0; t < 24; t++) {
//         // A_seq[1][i][j] = A_seq[1][i][j] + (1 / (sqrt(A_seq[0][i + 1][j] + A_seq[2][i - 1][j])));
//         A[idx] = A[idx] + (float)(1 / (native_sqrt(A[idk0i1] + A[idk2i1])));
//       }
//     }
//   }
// };