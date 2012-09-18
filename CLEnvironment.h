#ifndef CL_ENVIRONMENT_H
#define CL_ENVIRONMENT_H

#include "CLDevice.h"

/* Struct representing the execution environment for one device */
typedef struct CLDeviceEnvironment {
	cl_context context;
	cl_platform_id platform;
	CLDeviceInfo info;
	cl_command_queue queue;
	cl_program program;
	cl_kernel* kernels;
	unsigned int kernels_count;
} CLDeviceEnvironment;

/* CL environment functions */
void clCreateDeviceEnvironment(
	CLDeviceInfo* devices,
	unsigned int device_count,
	const char* kernel_path,
	const char** kernel_functions,
	unsigned int kernel_functions_count,
	const char* build_options,
	unsigned int enable_gpu_profiling,
	unsigned int shared,
	CLDeviceEnvironment* output);

void clFreeDeviceEnvironments(
	CLDeviceEnvironment* environments,
	unsigned int device_count,
	unsigned int shared);


void clErrorToString(
	cl_int error,
	char* output);

/* Private functions */
void clLoadFile(
	const char* file,
	size_t* size,
	const char* * output);
/* End */

#endif
