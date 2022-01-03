#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <CL/opencl.h>

#define MAX_SOURCE_SIZE (0x100000)

int main(int argc, char *argv[])
{
    // Length of vectors
    unsigned int n = 100000;

    // Host input vectors
    double * A;
    double * B;
    // Host output vector
    double * C;

    // Device input buffers
    cl_mem  mem_a;
    cl_mem  mem_b;
    // Device output buffer
    cl_mem  mem_c;

    cl_platform_id cpPlatform; // OpenCL platform
    cl_device_id device_id;    // device ID
    cl_context context;        // context
    cl_command_queue queue;    // command queue
    cl_program program;        // program
    cl_kernel kernel;          // kernel

    // Size, in bytes, of each vector
    size_t bytes = n * sizeof(double);

    // Allocate memory for each vector on host
     A = (double *)malloc(bytes);
     B = (double *)malloc(bytes);
     C = (double *)malloc(bytes);

    // Initialize vectors on host
    int i;
    for (i = 0; i < n; i++)
    {
         A[i] = i;
         B[i] = i;
    }

    //kernel----------------------------------------------
    // Load the kernel source code into the array kernelSource
    FILE *fp;
    char *kernelSource;
    size_t source_size;

    fp = fopen("kernel.cl", "r");
    if (!fp)
    {
        fprintf(stderr, "Failed to load kernel.\n");
        exit(1);
    }
    kernelSource = (char *)malloc(MAX_SOURCE_SIZE);
    source_size = fread(kernelSource, 1, MAX_SOURCE_SIZE, fp);
    fclose(fp);
    //kernel----------------------------------------------

    size_t globalSize, localSize;
    cl_int err;

    // Number of work items in each local work group
    localSize = 64;

    // Number of total work items - localSize must be devisor
    globalSize = ceil(n / (float)localSize) * localSize;

    // Bind to platform
    err = clGetPlatformIDs(1, &cpPlatform, NULL);

    // Get ID for the device
    err = clGetDeviceIDs(cpPlatform, CL_DEVICE_TYPE_GPU, 1, &device_id, NULL);

    // Create a context
    context = clCreateContext(0, 1, &device_id, NULL, NULL, &err);

    // Create a command queue
    queue = clCreateCommandQueue(context, device_id, 0, &err);

    // Create the compute program from the source buffer
    program = clCreateProgramWithSource(context, 1,
                                        (const char **)&kernelSource, NULL, &err);

    // Build the program executable
    clBuildProgram(program, 0, NULL, NULL, NULL, NULL);

    // Create the compute kernel in the program we wish to run
    kernel = clCreateKernel(program, "vecAdd", &err);

    // Create the input and output arrays in device memory for our calculation
     mem_a = clCreateBuffer(context, CL_MEM_READ_ONLY, bytes, NULL, NULL);
     mem_b = clCreateBuffer(context, CL_MEM_READ_ONLY, bytes, NULL, NULL);
     mem_c = clCreateBuffer(context, CL_MEM_WRITE_ONLY, bytes, NULL, NULL);

    // Write our data set into the input array in device memory
    err = clEnqueueWriteBuffer(queue,  mem_a, CL_TRUE, 0,
                               bytes,  A, 0, NULL, NULL);
    err |= clEnqueueWriteBuffer(queue,  mem_b, CL_TRUE, 0,
                                bytes,  B, 0, NULL, NULL);

    // Set the arguments to our compute kernel
    err = clSetKernelArg(kernel, 0, sizeof(cl_mem), & mem_a);
    err |= clSetKernelArg(kernel, 1, sizeof(cl_mem), & mem_b);
    err |= clSetKernelArg(kernel, 2, sizeof(cl_mem), & mem_c);
    err |= clSetKernelArg(kernel, 3, sizeof(unsigned int), &n);

    // Execute the kernel over the entire range of the data set
    err = clEnqueueNDRangeKernel(queue, kernel, 1, NULL, &globalSize, &localSize,
                                 0, NULL, NULL);

    // Wait for the command queue to get serviced before reading back results
    clFinish(queue);

    // Read the results from the device
    clEnqueueReadBuffer(queue,  mem_c, CL_TRUE, 0,
                        bytes,  C, 0, NULL, NULL);

    //Sum up vector c and print result divided by n, this should equal 1 within error
    double sum = 0;
    for (i = 0; i < n; i++)
    {
        printf("%f + %f = %f\n",  A[i],  B[i],  C[i]);
        sum +=  C[i];
    }
    printf("final result: %f\n", sum / n);

    // release OpenCL resources
    clReleaseMemObject( mem_a);
    clReleaseMemObject( mem_b);
    clReleaseMemObject( mem_c);
    clReleaseProgram(program);
    clReleaseKernel(kernel);
    clReleaseCommandQueue(queue);
    clReleaseContext(context);

    //release host memory
    free(A);
    free(B);
    free( C);

    return 0;
}