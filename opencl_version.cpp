#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <math.h>
#include <time.h>
#include <chrono>

#include <CL/cl.h>

const unsigned int depth  =     3; //Different dimensions on each axis makes the indices esier to identify.
const unsigned int row  =     16; //8192
const unsigned int column =    16; //8192

cl_float input1[depth][row][column];
cl_float input2[depth][row][column];

cl_float output1[depth][row][column];
cl_float output2[depth][row][column];
cl_float output3[depth][row][column];


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

int main(int argc, char** argv){
  cl_int error_no;
  cl_uint num_of_platforms;
  cl_uint num_of_devices;
  cl_platform_id * platform_ids;
  cl_device_id * device_ids;
  cl_context context = NULL;
  cl_command_queue command_queue;
  cl_program program;
  cl_kernel kernel;
  cl_mem inputCLBuffer1;
  cl_mem inputCLBuffer2;
  cl_mem outputCLBuffer1;
  cl_mem outputCLBuffer2;
  cl_mem outputCLBuffer3;
  char buffer[10240];
  
  for (unsigned int i = 0; i < depth; ++i){
    for (unsigned int j = 0; j < row; ++j){
      for (unsigned int k = 0; k < column; ++k){
	      input1[i][j][k] = 100.0f; //Some values very different from the indices
	      input2[i][j][k] = 101.0f;
      }
    }
  }


  clock_t s_t1 = clock();
  //==========load kernel ===========================================================================
  // Load the kernel source code into the array kernel_source
  std::ifstream kernel_file("kernel.cl");
  std::string k_src(std::istreambuf_iterator<char>(kernel_file), (std::istreambuf_iterator<char>()));
  const char* kernel_source = k_src.data();	
  //==========load kernel ===========================================================================
  
  
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
    device_ids = (cl_device_id *)alloca(sizeof(cl_device_id) * num_of_devices);
    error_no = clGetDeviceIDs( platform_ids[i], CL_DEVICE_TYPE_GPU, num_of_devices, &device_ids[0], NULL);
    errorCheck(error_no, "clGetDeviceIDs");
  }
  
  if (device_ids == NULL) {
    std::cout << "No CPU device found" << std::endl;
    exit(-1);
  }
  
  cl_context_properties context_props[] = { CL_CONTEXT_PLATFORM, (cl_context_properties) platform_ids[i], 0 };
  context = clCreateContext(context_props, num_of_devices, device_ids, &contextCallbackFnct, NULL, &error_no);
  errorCheck(error_no, "clCreateContext");
  
  size_t kernel_source_length = std::string(kernel_source).length();
  program = clCreateProgramWithSource( context, 1, &kernel_source, &kernel_source_length, &error_no);
  errorCheck(error_no, "clCreateProgramWithSource");

  error_no = clBuildProgram(program, num_of_devices, device_ids, NULL, NULL, NULL);
  errorCheck(error_no, "clBuildProgram");
  clGetProgramBuildInfo(program, *device_ids, CL_PROGRAM_BUILD_LOG, sizeof(buffer), buffer, NULL);
  fprintf(stderr, "CL Kernel Compilation:\n %s \n", buffer);
  
  kernel = clCreateKernel(program, "ThreeDimArray", &error_no);
  errorCheck(error_no, "clCreateKernel");
    
  inputCLBuffer1 = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, 
				sizeof(cl_float) * depth * row * column,
				static_cast<void *>(input1), &error_no);
  errorCheck(error_no, "clCreateBuffer(input1)");
  
  inputCLBuffer2 = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
				sizeof(cl_float) * depth * row * column,
				static_cast<void *>(input2), &error_no);
  errorCheck(error_no, "clCreateBuffer(input2)");
  
  outputCLBuffer1 = clCreateBuffer(context, CL_MEM_WRITE_ONLY,
				 sizeof(cl_float) * depth * row * column,
				 NULL, &error_no);
  errorCheck(error_no, "clCreateBuffer(output1)");
  
  outputCLBuffer2 = clCreateBuffer(context, CL_MEM_WRITE_ONLY,
				 sizeof(cl_float) * depth * row * column,
				 NULL, &error_no);
  errorCheck(error_no, "clCreateBuffer(output2)");
  
  outputCLBuffer3 = clCreateBuffer(context, CL_MEM_WRITE_ONLY,
				 sizeof(cl_float) * depth * row * column,
				 NULL, &error_no);
  errorCheck(error_no, "clCreateBuffer(output3)");
  
  command_queue = clCreateCommandQueue(context, device_ids[0], 0, &error_no);
  errorCheck(error_no, "clCreateCommandCommand_Queue");
  
  error_no  = clSetKernelArg(kernel, 0, sizeof(cl_mem), &inputCLBuffer1);
  error_no |= clSetKernelArg(kernel, 1, sizeof(cl_mem), &inputCLBuffer2);
  error_no |= clSetKernelArg(kernel, 2, sizeof(cl_mem), &outputCLBuffer1);
  error_no |= clSetKernelArg(kernel, 3, sizeof(cl_mem), &outputCLBuffer2);
  error_no |= clSetKernelArg(kernel, 4, sizeof(cl_mem), &outputCLBuffer3);
  errorCheck(error_no, "clSetKernelArg");
  
  const size_t globalWorkSize[3] = {depth, row, column};
  const size_t localWorkSize[3] = {1, 1, 1};
  
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
				output1, 0, NULL, NULL);
  errorCheck(error_no, "clEnqueueReadBuffer - 1");
  error_no = clEnqueueReadBuffer( command_queue, outputCLBuffer2, CL_TRUE, 0,
				sizeof(cl_float) * depth * row * column,
				output2, 0, NULL, NULL);
  errorCheck(error_no, "clEnqueueReadBuffer - 2");
  error_no = clEnqueueReadBuffer( command_queue, outputCLBuffer3, CL_TRUE, 0,
				sizeof(cl_float) * depth * row * column,
				output3, 0, NULL, NULL);
  errorCheck(error_no, "clEnqueueReadBuffer - 3");

  clock_t s_t2 = clock();
  // showing total time taken to sequential executation
	printf("OpenCL Code: \n");
	showTimeDifference(s_t1, s_t2);


  std::ofstream fout("output.txt");
  for (int x = 0; x < depth; x++){
    for (int y = 0; y < row; y++){
      for (int z = 0; z < column; z++){
		fout << output1[x][y][z] << " ";
      }	
      fout << std::endl;
    }
    fout << std::endl;
  }
  fout <<  "***********************************\n" << std::endl;
  // for (int x = 0; x < depth; x++){
  //   for (int y = 0; y < row; y++){
  //     for (int z = 0; z < column; z++){
	// fout << output2[x][y][z] << " ";
  //     }	
  //     fout << std::endl;
  //   }
  //   fout << std::endl;
  // }
  // fout <<  "***********************************\n" << std::endl;
  // for (int x = 0; x < depth; x++){
  //   for (int y = 0; y < row; y++){
  //     for (int z = 0; z < column; z++){
	// fout << output3[x][y][z] << " ";
  //     }	
  //     fout << std::endl;
  //   }
  //   fout << std::endl;
  // }
  fout.close();
  return(0);
}