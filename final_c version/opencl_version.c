
#ifdef __APPLE__
#include <OpenCL/opencl.h>
#else
#include <CL/cl.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#define MAX_SOURCE_SIZE (0x100000)

//for using sequential part
const int depth = 3;
const int row = 8192; //8192
const int column = 8192;

float A_seq[depth][row][column];

//for openCL part
int SIZE = 8192; //8192

//======================================================================================================================
//----------------------------------Helper functions--------------------------------------------------------------------
//======================================================================================================================
double showTimeDifference(clock_t t1, clock_t t2)
{
	double timeDifference = double(t2 - t1) / CLOCKS_PER_SEC;
	;
	printf("========================= Time taken %.4f s ==========================\n\n", timeDifference);

	return timeDifference;
}

void displayMatrix(float (*MAT)[row][column])
{
	//Display matrix ------------------------------------------------------
	for (int k = 0; k < depth; k++)
	{
		printf("Baseline matrix k = %d (i=0..10, j =0..10):\n", k);
		//each row
		for (int i = 0; i < row; i++)
		{
			//each column
			for (int j = 0; j < column; j++)
			{
				printf("%f \t", MAT[k][i][j]);
			}
			printf("\n");
		}
		printf("\n");
	}
}

void checkValidity(float (*MAT_A)[row][column], float (*MAT_B)[row][column])
{
	//check matrix validity------------------------------------------------------
	bool isSame = true;
	for (int k = 0; k < depth; k++)
	{
		//each row
		for (int i = 0; i < row; i++)
		{
			//each column
			for (int j = 0; j < column; j++)
			{
				if (round(MAT_A[k][i][j]) != round(MAT_B[k][i][j]))
				{
					printf("mat seq = %f\n", MAT_A[k][i][j]);
					printf("mat opem = %f\n", MAT_B[k][i][j]);
					isSame = false;
				}
			}
		}
	}

	if (isSame == true)
	{
		printf("Matrix validition Successful!\n\n");
	}
	else
	{
		printf("Matrix validition Failed!\n\n");
	}
}

//======================================================================================================================
//----------------------------------Helper functions--------------------------------------------------------------------
//======================================================================================================================

//======================================================================================================================
//----------------------------------Sequential code---------------------------------------------------------------------
//======================================================================================================================
void sequential_code()
{
	//loop to fill in every row
	for (int k = 0; k < depth; k++)
	{
		//loop to fill in every column
		for (int i = 0; i < row; i++)
		{
			//loop to fill in each 2D matrix
			for (int j = 0; j < column; j++)
			{
				//for matrix k=0
				if (k == 0)
				{
					A_seq[k][i][j] = (float)i / ((float)j + 1.00);
				}
				//for matrix k=1
				else if (k == 1)
				{
					A_seq[k][i][j] = 1.00;
				}
				//for matrix k=2
				else
				{
					A_seq[k][i][j] = (float)j / ((float)i + 1.00);
				}
			}
		}
	}

	//Iteration count
	for (int t = 0; t < 24; t++)
	{
		// printf("Iteration = %d :\n", t);
		//each row - beware first row and last row not to be updated therefore from 1...8190
		for (int i = 1; i < row - 1; i++)
		{
			//each column
			for (int j = 0; j < column; j++)
			{
				//only matrix k=1 is updated
				A_seq[1][i][j] = A_seq[1][i][j] + (1 / (sqrt(A_seq[0][i + 1][j] + A_seq[2][i - 1][j])));
				// printf("%.2f \t", A[1][i][j]);
			}
			// printf("\n");
		}
		// printf("\n");
	}
}
//======================================================================================================================
//----------------------------------Sequential code---------------------------------------------------------------------
//======================================================================================================================

int main(int argc, char **argv)
{

	//sequential code-------------------------------------------------------------------------------------------
	printf("Sequential code start executing......\n");
	clock_t seq_t1 = clock(); //taking start time of execution
	sequential_code();
	clock_t seq_t2 = clock(); //taking end time of execution
	// printf("Sequential Code: \n");
	double seq_time = showTimeDifference(seq_t1, seq_t2); //calculate time difference for sequential executation
	//Display seq matrix
	// displayMatrix(A_seq);
	printf("Sequential code execution finished!\n");
	//sequential code-------------------------------------------------------------------------------------------



	//OpenCL code-----------------------------------------------------------------------------------------------
	printf("Opencl code start executing......\n");
	
	// Allocate memories for input arrays and output array.
	float *A = (float *)malloc(sizeof(float) * SIZE * SIZE); //for k=0 read only
	float *B = (float *)malloc(sizeof(float) * SIZE * SIZE); //for k=1 read and write
	float *C = (float *)malloc(sizeof(float) * SIZE * SIZE); //for k=2 ready only

	// Initialize values for array members.
	for (int k = 0; k < depth; ++k)
	{
		for (int i = 0; i < SIZE; ++i)
		{
			for (int j = 0; j < SIZE; ++j)
			{
				//for matrix k=0
				if (k == 0)
				{
					A[i * SIZE + j] = (float)i / ((float)j + 1.00); //A_seq[0][i][j]
				}
				//for matrix k=1
				else if (k == 1)
				{
					B[i * SIZE + j] = 1.00;	//A_seq[1][i][j]
				}
				//for matrix k=2
				else
				{
					C[i * SIZE + j] = (float)j / ((float)i + 1.00);	//A_seq[2][i][j]
				}
			}
		}
	}

	// Load kernel from file matKernel.cl
	FILE *kernelFile;
	char *kernelSource;
	size_t kernelSize;

	kernelFile = fopen("matKernel.cl", "r");

	if (!kernelFile)
	{
		fprintf(stderr, "No file named matKernel.cl was found\n");
		exit(-1);
	}
	kernelSource = (char *)malloc(MAX_SOURCE_SIZE);
	kernelSize = fread(kernelSource, 1, MAX_SOURCE_SIZE, kernelFile);
	fclose(kernelFile);

	// Getting platform and device information
	cl_platform_id platformId = NULL;
	cl_device_id deviceID = NULL;
	cl_uint retNumDevices;
	cl_uint retNumPlatforms;
	cl_int ret = clGetPlatformIDs(1, &platformId, &retNumPlatforms);
	ret = clGetDeviceIDs(platformId, CL_DEVICE_TYPE_DEFAULT, 1, &deviceID, &retNumDevices);

	// Creating context.
	cl_context context = clCreateContext(NULL, 1, &deviceID, NULL, NULL, &ret);

	// Creating command queue
	cl_command_queue commandQueue = clCreateCommandQueue(context, deviceID, 0, &ret);

	// Memory buffers for each array
	cl_mem aMemObj = clCreateBuffer(context, CL_MEM_READ_ONLY, SIZE * SIZE * sizeof(float), NULL, &ret);
	cl_mem bMemObj = clCreateBuffer(context, CL_MEM_READ_WRITE, SIZE * SIZE * sizeof(float), NULL, &ret);
	cl_mem cMemObj = clCreateBuffer(context, CL_MEM_READ_ONLY, SIZE * SIZE * sizeof(float), NULL, &ret);



	clock_t opn_t1 = clock();           //taking start time of OpenCL execution
	// Copy lists to memory buffers
	ret = clEnqueueWriteBuffer(commandQueue, aMemObj, CL_TRUE, 0, SIZE * SIZE * sizeof(float), A, 0, NULL, NULL);
	ret = clEnqueueWriteBuffer(commandQueue, bMemObj, CL_TRUE, 0, SIZE * SIZE * sizeof(float), B, 0, NULL, NULL);
	ret = clEnqueueWriteBuffer(commandQueue, cMemObj, CL_TRUE, 0, SIZE * SIZE * sizeof(float), C, 0, NULL, NULL);

	// Create program from kernel source
	cl_program program = clCreateProgramWithSource(context, 1, (const char **)&kernelSource, (const size_t *)&kernelSize, &ret);

	// Build program
	ret = clBuildProgram(program, 1, &deviceID, NULL, NULL, NULL);

	// Create kernel
	cl_kernel kernel = clCreateKernel(program, "matrixOperations", &ret);

	// Set arguments for kernel
	ret = clSetKernelArg(kernel, 0, sizeof(cl_mem), (void *)&aMemObj);
	ret = clSetKernelArg(kernel, 1, sizeof(cl_mem), (void *)&bMemObj);
	ret = clSetKernelArg(kernel, 2, sizeof(cl_mem), (void *)&cMemObj);
	ret = clSetKernelArg(kernel, 3, sizeof(cl_int), &SIZE);

	// Execute the kernel
	size_t globalItemSize = SIZE * SIZE;
	size_t localItemSize = 64; // globalItemSize has to be a multiple of localItemSize ex: 1024/64 = 16
	for (int i = 0; i < 24; ++i)
	{
		ret = clEnqueueNDRangeKernel(commandQueue, kernel, 1, NULL, &globalItemSize, &localItemSize, 0, NULL, NULL);
	}

	// Read from device back to host.
	ret = clEnqueueReadBuffer(commandQueue, bMemObj, CL_TRUE, 0, SIZE * SIZE * sizeof(float), B, 0, NULL, NULL);
	
	clock_t opn_t2 = clock();           //taking end time of OpenCL execution
	printf("OpenCL code execution finished!\n");
	
	// printf("OpenCL Code: \n");
	double openCl_time = showTimeDifference(opn_t1, opn_t2); //calculate time difference for OpenCl parallel executation
  	
	//Display matrix 
  	// displayMatrix(A);

  	//matrix validation----------------------------------------------------
  	// checkValidity(A_seq, A);

  	//performance calculation----------------------------------------------
  	double speedUp = seq_time/openCl_time;
  	printf("Speed up: %.2f \n", speedUp);


	// Write result
	// printf("\n");
	// for (int i = 0; i < SIZE; ++i)
	// {
	// 	for (int j = 0; j < SIZE; ++j)
	// 	{
	// 		printf("%.2f \t", B[i * SIZE + j]);
	// 	}
	// 	printf("\n");
	// }


	// Clean up, release memory.
	ret = clFlush(commandQueue);
	ret = clFinish(commandQueue);
	ret = clReleaseCommandQueue(commandQueue);
	ret = clReleaseKernel(kernel);
	ret = clReleaseProgram(program);
	ret = clReleaseMemObject(aMemObj);
	ret = clReleaseMemObject(bMemObj);
	ret = clReleaseMemObject(cMemObj);
	ret = clReleaseContext(context);
	free(A);
	free(B);
	free(C);

	return 0;
}