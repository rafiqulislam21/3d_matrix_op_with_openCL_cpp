#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <math.h>
#include <time.h>

#include <CL/cl.h>

const unsigned int depth = 3;     //Different dimensions on each axis makes the indices esier to identify.
const unsigned int row = 8;       //8192
const unsigned int column = 8;    //8192

float A_seq[depth][row][column];  // sequential output matrix A
cl_float A[depth][row][column];   // OpenCL Host output matrix A

//==========================================================================================================================
//----------------------------------Helper functions------------------------------------------------------------------------
//==========================================================================================================================
double showTimeDifference(clock_t t1, clock_t t2){
    double timeDifference = double(t2 - t1)/CLOCKS_PER_SEC;;
	  printf("========================= Time taken %.4f s ==========================\n\n", timeDifference);

    return timeDifference;
}

void displayMatrix(float (*MAT)[row][column]){
  //Display matrix ------------------------------------------------------
	for (int k = 0; k < depth; k++){
		printf("Baseline matrix k = %d (i=0..10, j =0..10):\n", k);
		//each row
    for (int i = 0; i < row; i++){
			//each column
			for (int j = 0; j < column; j++){
				printf("%f \t", MAT[k][i][j]);
			}
			printf("\n");
		}
		printf("\n");
	}
}

void checkValidity(float (*MAT_A)[row][column], float (*MAT_B)[row][column]){
  //check matrix validity------------------------------------------------------
  bool isSame = true;
	for (int k = 0; k < depth; k++){
		//each row
    for (int i = 0; i < row; i++){
			//each column
			for (int j = 0; j < column; j++){
				if(round(MAT_A[k][i][j]) != round(MAT_B[k][i][j])){
          printf("mat seq = %f\n", MAT_A[k][i][j]);
          printf("mat opem = %f\n", MAT_B[k][i][j]);
          isSame = false;
        }
			}
		}
	}

  if(isSame == true){
    printf("Matrix validition Successful!\n\n");
  }else{
    printf("Matrix validition Failed!\n\n");
  }
  
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
void Sequential_code(){
  printf("Sequential code start executing......\n");
  
  //Filling matrix with values-------------------------------------------
	//loop to fill in each 2D matrix
	for (int k = 0; k < depth; k++){
		//loop to fill in every row
		for (int i = 0; i < row; i++){
			//loop to fill in every column
			for (int j = 0; j < column; j++){
				//for matrix k=0
				if (k == 0){
					A_seq[k][i][j] = (float)i / ((float)j + 1.00);
				}
				//for matrix k=1
				else if (k == 1){
					A_seq[k][i][j] = 1.00;
				}
				//for matrix k=2
				else{
					A_seq[k][i][j] = (float)j / ((float)i + 1.00);
				}
			}
		}
	}

	//Iteration count------------------------------------------------------
	for (int t = 0; t < 24; t++){
		// printf("Iteration = %d :\n", t);
		//each row - beware first row and last row not to be updated therefore from 1...8190
		for (int i = 1; i < row; i++){
			//each column
			for (int j = 0; j < column; j++){
				//only matrix k=1 is updated
				A_seq[1][i][j] = A_seq[1][i][j] + (1 / (sqrt(A_seq[0][i + 1][j] + A_seq[2][i - 1][j])));
				// printf("%.2f \t", A[1][i][j]);
			}
			// printf("\n");
		}
		// printf("\n");
	}

  printf("Sequential code execution finished!\n");
}

//==========================================================================================================================
//----------------------------------Sequential code-------------------------------------------------------------------------
//==========================================================================================================================




//==========================================================================================================================
//----------------------------------OpenCL code-----------------------------------------------------------------------------
//==========================================================================================================================
void OpenCL_code(){
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

  // Device output buffer
  cl_mem memBufferA;
  
  char buffer[10240];

  
  // Load the kernel source code into the array kernel_source
  std::ifstream kernel_file("kernel.cl");
  std::string k_src(std::istreambuf_iterator<char>(kernel_file), (std::istreambuf_iterator<char>()));
  const char* kernel_source = k_src.data();	
  
  // Bind to platform
  error_no = clGetPlatformIDs(0, NULL, &num_of_platforms);
  errorCheck((error_no != CL_SUCCESS) ? error_no : (num_of_platforms <= 0 ? -1 : CL_SUCCESS), "clGetPlatformIDs");
  printf("** %d OpenCL platform(s) found: \n", num_of_platforms);
  
  platform_ids = (cl_platform_id *)alloca(sizeof(cl_platform_id) * num_of_platforms);
  error_no = clGetPlatformIDs(num_of_platforms, platform_ids, NULL);
  
  errorCheck((error_no != CL_SUCCESS) ? error_no : (num_of_platforms <= 0 ? -1 : CL_SUCCESS), "clGetPlatformIDs");
  
  device_ids = NULL;
  cl_uint i = 0;
  clGetPlatformInfo(platform_ids[i], CL_PLATFORM_NAME, 10240, buffer, NULL);
  printf("\tNAME = %s\n", buffer);
  error_no = clGetDeviceIDs(platform_ids[i], CL_DEVICE_TYPE_GPU, 1, NULL, &num_of_devices);
  printf("** %d OpenCL devices: \n", num_of_devices);
    
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
  fprintf(stderr, "\tCL Kernel Compilation:\n %s \n", buffer);
  
  // Create the compute kernel in the program we wish to run
  kernel = clCreateKernel(program, "ThreeDimArray", &error_no);
  errorCheck(error_no, "clCreateKernel");
    
  // Create the input and output arrays in device memory for our calculation
  memBufferA = clCreateBuffer(context, CL_MEM_WRITE_ONLY,
				 sizeof(cl_float) * depth * row * column,
				 NULL, &error_no);
  errorCheck(error_no, "clCreateBuffer(A)");
  
  // Create a command queue
  command_queue = clCreateCommandQueue(context, device_ids[0], 0, &error_no);
  errorCheck(error_no, "clCreateCommandCommand_Queue");
  

  // Set the arguments to our compute kernel
  error_no  = clSetKernelArg(kernel, 0, sizeof(cl_mem), &memBufferA);
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

  // Wait for the command queue to get serviced before reading back results
	clFinish(command_queue);

  // Read the results from the device  
  error_no = clEnqueueReadBuffer( command_queue, memBufferA, CL_TRUE, 0,
				sizeof(cl_float) * depth * row * column,
				A, 0, NULL, NULL);
  errorCheck(error_no, "clEnqueueReadBuffer - 1");

  // release OpenCL resources
	clReleaseMemObject(memBufferA);
	clReleaseProgram(program);
	clReleaseKernel(kernel);
	clReleaseCommandQueue(command_queue);
	clReleaseContext(context);
	//release host memory
	// free(A);

  printf("OpenCL code execution finished!\n");
}

//==========================================================================================================================
//----------------------------------OpenCL code-----------------------------------------------------------------------------
//==========================================================================================================================



int main(int argc, char** argv){
  
  //sequential code------------------------------------------------------
  clock_t seq_t1 = clock();           //taking start time of execution
  Sequential_code();
  clock_t seq_t2 = clock();           //taking end time of execution
	// printf("Sequential Code: \n");
	double seq_time = showTimeDifference(seq_t1, seq_t2); //calculate time difference for sequential executation
  //Display seq matrix 
  displayMatrix(A_seq);

  
  //OpenCL code----------------------------------------------------------
  clock_t opn_t1 = clock();           //taking start time of OpenCL execution
  OpenCL_code();
  clock_t opn_t2 = clock();           //taking end time of OpenCL execution
	// printf("OpenCL Code: \n");
	double openCl_time = showTimeDifference(opn_t1, opn_t2); //calculate time difference for OpenCl parallel executation
  //Display matrix 
  displayMatrix(A);

  //matrix validation----------------------------------------------------
  checkValidity(A_seq, A);

  //performance calculation----------------------------------------------
  double speedUp = seq_time/openCl_time;
  printf("Speed up: %.2f \n", speedUp);

  return(0);
}