#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <math.h>
#include <time.h>

#include <CL/cl.h>

const unsigned int depth  =     3; //Different dimensions on each axis makes the indices esier to identify.
const unsigned int row  =     8192; //8192
const unsigned int column =    8192; //8192

// cl_float input1[depth][row][column];
// cl_float input2[depth][row][column];

// Host output matrix
cl_float A[depth][row][column];
// cl_float output2[depth][row][column];
// cl_float output3[depth][row][column];

//==========================================================================================================================
//----------------------------------Helper functions------------------------------------------------------------------------
//==========================================================================================================================
void showTimeDifference(clock_t t1, clock_t t2){
    double timeTaken = double(t2 - t1)/CLOCKS_PER_SEC;;
	  printf("========================= Time taken = %.3f s ==========================\n", timeTaken);
}

inline void errorCheck(cl_int err, const char * name){
  if(err != CL_SUCCESS){
    std::cerr << "ERROR: " << name << " Error Code: " << err << std::endl;
    exit(EXIT_FAILURE);
  }
}

void CL_CALLBACK contextCallbackFnct(const char * err_info, const void * private_information, size_t tt, void * user_data){
  std::cout << "Error in context:"<< err_info << std::endl;
  exit(EXIT_FAILURE);
}

//==========================================================================================================================
//----------------------------------Helper functions------------------------------------------------------------------------
//==========================================================================================================================




//==========================================================================================================================
//----------------------------------Sequential code-------------------------------------------------------------------------
//==========================================================================================================================
void SequentialCode(){
  printf("Sequential code start executing......\n");
  clock_t s_t1 = clock();
	//loop to fill in every row
	for (int k = 0; k < depth; k++){
		//loop to fill in every column
		for (int i = 0; i < row; i++){
			//loop to fill in each 2D matrix
			for (int j = 0; j < column; j++){
				//for matrix k=0
				if (k == 0){
					A[k][i][j] = (float)i / ((float)j + 1.00);
				}
				//for matrix k=1
				else if (k == 1){
					A[k][i][j] = 1.00;
				}
				//for matrix k=2
				else{
					A[k][i][j] = (float)j / ((float)i + 1.00);
				}
			}
		}
	}

	//Display array====================================================================================
	// for (int k = 0; k < depth; k++){
	// 	printf("Baseline matrix k = %d (i=0..10, j =0..10):\n", k);
	// 	for (int i = 0; i < row; i++){
	// 		//each column
	// 		for (int j = 0; j < column; j++){
	// 			//only matrix k=1 is updated
	// 			printf("%.2f \t", A[k][i][j]);
	// 		}
	// 		printf("\n");
	// 	}
	// 	printf("\n");
	// }
	//Display array=======================================================================================

	//Iteration count
	for (int t = 0; t < 24; t++){
		// printf("Iteration = %d :\n", t);
		//each row - beware first row and last row not to be updated therefore from 1...8190
		for (int i = 1; i < row; i++){
			//each column
			for (int j = 0; j < column; j++){
				//only matrix k=1 is updated
				A[1][i][j] = A[1][i][j] + (1 / (sqrt(A[0][i + 1][j] + A[2][i - 1][j])));
				// printf("%.2f \t", A[1][i][j]);
			}
			// printf("\n");
		}
		// printf("\n");
	}
	clock_t s_t2 = clock();
	// showing total time taken to sequential executation
	printf("Sequential Code: \n");
	showTimeDifference(s_t1, s_t2);
  printf("Sequential code execution finished!\n");
}

//==========================================================================================================================
//----------------------------------Sequential code-------------------------------------------------------------------------
//==========================================================================================================================



int main(int argc, char** argv){
  //calling sequential code-------------------
  SequentialCode();

  //OpenCL code start-------------------------
  printf("Opencl code start executing......\n");
  cl_int error_no;
  cl_uint num_of_platforms;
  cl_uint num_of_devices;
  cl_platform_id * platform_ids;    // OpenCL platform
  cl_device_id * device_ids;        // device ID
  cl_context context = NULL;        // context
  cl_command_queue command_queue;   // command queue
  cl_program program;               // program
  cl_kernel kernel;                 // kernel
  // cl_mem inputCLBuffer1;
  // cl_mem inputCLBuffer2;

  // Device output buffer
  cl_mem outputCLBuffer1;
  
  // cl_mem outputCLBuffer2;
  // cl_mem outputCLBuffer3;
  char buffer[10240];
  
  // for (unsigned int i = 0; i < depth; ++i){
  //   for (unsigned int j = 0; j < row; ++j){
  //     for (unsigned int k = 0; k < column; ++k){
	//       input1[i][j][k] = 100.0f; //Some values very different from the indices
	//       input2[i][j][k] = 101.0f;
  //     }
  //   }
  // }


  clock_t s_t1 = clock();
  // Load the kernel source code into the array kernel_source
  std::ifstream kernel_file("kernel.cl");
  std::string k_src(std::istreambuf_iterator<char>(kernel_file), (std::istreambuf_iterator<char>()));
  const char* kernel_source = k_src.data();	
  
  // Bind to platform
  error_no = clGetPlatformIDs(0, NULL, &num_of_platforms);
  errorCheck((error_no != CL_SUCCESS) ? error_no : (num_of_platforms <= 0 ? -1 : CL_SUCCESS), "clGetPlatformIDs");
  printf("=== %d OpenCL platform(s) found: ===\n", num_of_platforms);
  
  platform_ids = (cl_platform_id *)alloca(sizeof(cl_platform_id) * num_of_platforms);
  error_no = clGetPlatformIDs(num_of_platforms, platform_ids, NULL);
  
  errorCheck((error_no != CL_SUCCESS) ? error_no : (num_of_platforms <= 0 ? -1 : CL_SUCCESS), "clGetPlatformIDs");
  
  device_ids = NULL;
  cl_uint i = 0;
  clGetPlatformInfo(platform_ids[i], CL_PLATFORM_NAME, 10240, buffer, NULL);
  printf("  NAME = %s\n", buffer);
  error_no = clGetDeviceIDs(platform_ids[i], CL_DEVICE_TYPE_GPU, 1, NULL, &num_of_devices);
  printf("=== %d OpenCL devices: ===\n", num_of_devices);
    
  if (error_no != CL_SUCCESS && error_no != CL_DEVICE_NOT_FOUND){
    errorCheck(error_no, "clGetDeviceIDs");
  }
  else if (num_of_devices > 0){
    // Get ID for the device
    device_ids = (cl_device_id *)alloca(sizeof(cl_device_id) * num_of_devices);
    error_no = clGetDeviceIDs( platform_ids[i], CL_DEVICE_TYPE_GPU, num_of_devices, &device_ids[0], NULL);
    errorCheck(error_no, "clGetDeviceIDs");
  }
  
  if (device_ids == NULL) {
    std::cout << "No CPU device found" << std::endl;
    exit(-1);
  }
  
  // Create a context
  cl_context_properties context_props[] = { CL_CONTEXT_PLATFORM, (cl_context_properties) platform_ids[i], 0 };
  context = clCreateContext(context_props, num_of_devices, device_ids, &contextCallbackFnct, NULL, &error_no);
  errorCheck(error_no, "clCreateContext");
  
  // Create the compute program from the source buffer
  size_t kernel_source_length = std::string(kernel_source).length();
  program = clCreateProgramWithSource( context, 1, &kernel_source, &kernel_source_length, &error_no);
  errorCheck(error_no, "clCreateProgramWithSource");

  // Build the program executable
  error_no = clBuildProgram(program, num_of_devices, device_ids, NULL, NULL, NULL);
  errorCheck(error_no, "clBuildProgram");
  clGetProgramBuildInfo(program, *device_ids, CL_PROGRAM_BUILD_LOG, sizeof(buffer), buffer, NULL);
  fprintf(stderr, "CL Kernel Compilation:\n %s \n", buffer);
  
  // Create the compute kernel in the program we wish to run
  kernel = clCreateKernel(program, "ThreeDimArray", &error_no);
  errorCheck(error_no, "clCreateKernel");
    
  // inputCLBuffer1 = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, 
	// 			sizeof(cl_float) * depth * row * column,
	// 			static_cast<void *>(input1), &error_no);
  // errorCheck(error_no, "clCreateBuffer(input1)");
  
  // inputCLBuffer2 = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
	// 			sizeof(cl_float) * depth * row * column,
	// 			static_cast<void *>(input2), &error_no);
  // errorCheck(error_no, "clCreateBuffer(input2)");
  
  // Create the input and output arrays in device memory for our calculation
  outputCLBuffer1 = clCreateBuffer(context, CL_MEM_WRITE_ONLY,
				 sizeof(cl_float) * depth * row * column,
				 NULL, &error_no);
  errorCheck(error_no, "clCreateBuffer(A)");
  
  // outputCLBuffer2 = clCreateBuffer(context, CL_MEM_WRITE_ONLY,
	// 			 sizeof(cl_float) * depth * row * column,
	// 			 NULL, &error_no);
  // errorCheck(error_no, "clCreateBuffer(output2)");
  
  // outputCLBuffer3 = clCreateBuffer(context, CL_MEM_WRITE_ONLY,
	// 			 sizeof(cl_float) * depth * row * column,
	// 			 NULL, &error_no);
  // errorCheck(error_no, "clCreateBuffer(output3)");
  
  // Create a command queue
  command_queue = clCreateCommandQueue(context, device_ids[0], 0, &error_no);
  errorCheck(error_no, "clCreateCommandCommand_Queue");
  

  // Set the arguments to our compute kernel
  // error_no  = clSetKernelArg(kernel, 0, sizeof(cl_mem), &inputCLBuffer1);
  // error_no |= clSetKernelArg(kernel, 1, sizeof(cl_mem), &inputCLBuffer2);
  error_no  = clSetKernelArg(kernel, 0, sizeof(cl_mem), &outputCLBuffer1);
  // error_no |= clSetKernelArg(kernel, 1, sizeof(cl_mem), &outputCLBuffer2);
  // error_no |= clSetKernelArg(kernel, 2, sizeof(cl_mem), &outputCLBuffer3);
  errorCheck(error_no, "clSetKernelArg");
  
  const size_t globalWorkSize[3] = {depth, row, column};
  const size_t localWorkSize[3] = {1, 1, 1};
  
  // Execute the kernel over the entire range of the data set
  error_no = clEnqueueNDRangeKernel(
				  command_queue,
				  kernel,
				  3,
				  NULL,
				  globalWorkSize,
				  localWorkSize,
				  0,
				  NULL,
				  NULL);
  errorCheck(error_no, "clEnqueueNDRangeKernel");
    
  error_no = clEnqueueReadBuffer( command_queue, outputCLBuffer1, CL_TRUE, 0,
				sizeof(cl_float) * depth * row * column,
				A, 0, NULL, NULL);
  errorCheck(error_no, "clEnqueueReadBuffer - 1");
  // error_no = clEnqueueReadBuffer( command_queue, outputCLBuffer2, CL_TRUE, 0,
	// 			sizeof(cl_float) * depth * row * column,
	// 			output2, 0, NULL, NULL);
  // errorCheck(error_no, "clEnqueueReadBuffer - 2");
  // error_no = clEnqueueReadBuffer( command_queue, outputCLBuffer3, CL_TRUE, 0,
	// 			sizeof(cl_float) * depth * row * column,
	// 			output3, 0, NULL, NULL);
  // errorCheck(error_no, "clEnqueueReadBuffer - 3");


  //todo-----------------------
  // // release OpenCL resources
	// clReleaseMemObject(mem_a);
	// clReleaseProgram(program);
	// clReleaseKernel(kernel);
	// clReleaseCommandQueue(queue);
	// clReleaseContext(context);

	// //release host memory
	// free(A);

  clock_t s_t2 = clock();
  // showing total time taken to sequential executation
	printf("OpenCL Code: \n");
	showTimeDifference(s_t1, s_t2);


  std::ofstream fout("output.txt");
  for (int x = 0; x < depth; x++){
    for (int y = 0; y < row; y++){
      for (int z = 0; z < column; z++){
		fout << A[x][y][z] << " ";
      }	
      fout << std::endl;
    }
    fout << std::endl;
  }
  fout <<  "***********************************\n" << std::endl;
  fout.close();

  printf("OpenCL code execution finished!\n");

  return(0);
}