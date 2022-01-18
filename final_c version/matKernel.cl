
__kernel void matrixOperations(__global const float *a, __global float *b,
                               __global const float *c, int size) {

  int n = get_global_id(0);
  int i = n / size;
  int j = n % size;
  if (i > 0 && i < size - 1) {
    b[(size * i) + j] +=
        (1 / (native_sqrt(a[size * (i + 1) + j] + c[size * (i - 1) + j])));
  }
}