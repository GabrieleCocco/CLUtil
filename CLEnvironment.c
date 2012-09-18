#include "CLEnvironment.h"
#include <stdio.h>
void clLoadFile(
	const char* file,
	size_t* size,
	const char* * output) {
    // locals 
    FILE* pFileStream = NULL;
    size_t szSourceLength;

	if((pFileStream = fopen(file, "r")) != 0)
		*output = NULL;

    // get the length of the source code
    fseek(pFileStream, 0, SEEK_END); 
    szSourceLength = ftell(pFileStream);
    fseek(pFileStream, 0, SEEK_SET); 

    // allocate a buffer for the source code string and read it in
    char* cSourceString = (char*)malloc(szSourceLength + 1); 
    if (fread(cSourceString, szSourceLength, 1, pFileStream) != 1)
    {
        fclose(pFileStream);
        free(cSourceString);
    }

    // close the file and return the total length of the combined (preamble + source) string
    fclose(pFileStream);
    cSourceString[szSourceLength] = '\0';
	*size = szSourceLength;
    *output = cSourceString;
}

void clListDevices(
	unsigned int* count,
	CLDeviceInfo** output) {

	cl_device_id device_ids[100];
	CLDeviceInfo* devices;
	cl_platform_id platform;
	clGetPlatformIDs(1, &platform, NULL);
	clGetDeviceIDs(platform, CL_DEVICE_TYPE_ALL, 100, device_ids, count);
	
	devices = (CLDeviceInfo*)malloc(*count * sizeof(CLDeviceInfo));
	for(unsigned int i = 0; i < *count; i++) {
		devices[i].id = device_ids[i];
		devices[i].name = (char*)malloc(256 * sizeof(char));
		cl_device_type type;
		clGetDeviceInfo(devices[i].id, CL_DEVICE_NAME, 256, devices[i].name, NULL);		
		clGetDeviceInfo(devices[i].id, CL_DEVICE_TYPE, sizeof(type), &type, NULL);
		devices[i].type = type;
 	}
	*output = devices;
}

void clCreateDeviceEnvironment(
	CLDeviceInfo* devices,
	unsigned int count,
	const char* kernel_path,
	const char** kernel_functions,
	unsigned int kernel_functions_count,
	const char* build_options,
	unsigned int enable_gpu_profiling,
	unsigned int shared,
	CLDeviceEnvironment* output) {
		
	cl_platform_id platform_id;
	//printf("\n  %-35s", "Getting platform id...");
	clGetPlatformIDs(1, &platform_id, NULL);
	//printf("DONE!\n");

	if(!shared) {
		for(unsigned int i = 0; i < count; i++) {
			output[i].platform = platform_id;

			//printf("  %-35s (device id %d)", "Creating context for device...", devices[i].id);
			output[i].info.id = devices[i].id;
			output[i].info.type = devices[i].type;
			output[i].info.name = (char*)malloc(strlen(devices[i].name) + 1);
			memset(output[i].info.name, 0, strlen(devices[i].name) + 1);
			strncpy(output[i].info.name, devices[i].name, strlen(devices[i].name));
			cl_int err;
			output[i].context = clCreateContext(0, 1, &devices[i].id, NULL, NULL, &err);
            clCheckOrExit(err);
			//printf("DONE! (%d)\n", err);

			//printf("  %-35s", "Creating command queues...");	
			if(enable_gpu_profiling)
				output[i].queue = clCreateCommandQueue(output[i].context, output[i].info.id, CL_QUEUE_PROFILING_ENABLE, &err);
			else
				output[i].queue = clCreateCommandQueue(output[i].context, output[i].info.id, 0, &err);
            clCheckOrExit(err);
			//printf("DONE! (%d)\n", err);

			//printf("  %-35s", "Loading kernel file...");
			const char* kernel_src;
			const size_t kernel_size;
			clLoadFile(kernel_path, &kernel_size, &kernel_src);
			//printf("DONE!\n");

			//printf("  %-35s", "Creating program with source...");
			output[i].program = clCreateProgramWithSource(output[i].context, 1, &kernel_src, &kernel_size, &err);
            clCheckOrExit(err);
			//printf("DONE!\n");

			//printf("  %-35s", "Building program...");
			cl_int status = clBuildProgram(output[i].program, 0, NULL, build_options, NULL, NULL);
			if(status != CL_SUCCESS) {
				char build_log[2048];
				memset(build_log, 0, 2048);
				clGetProgramBuildInfo(output[i].program, devices[i].id, CL_PROGRAM_BUILD_LOG, 2048, build_log, NULL);
				printf("\n\nBUILD LOG: \n %s\n\n", build_log);
			}
			//printf("DONE!\n");

			output[i].kernels_count = kernel_functions_count;
			output[i].kernels = (cl_kernel*)malloc(kernel_functions_count * sizeof(cl_kernel));
			for(unsigned int j = 0; j < kernel_functions_count; j++) {		
				//std::ostringstream o;
				//o << "  Creating kernel " << kernel_functions[j];

				//printf("  %-35s", o.str().c_str());
				output[i].kernels[j] = clCreateKernel(output[i].program, kernel_functions[j], &err);
				if(err != CL_SUCCESS)
					printf("FAIL CREATING KERNEL %s! (%d)\n", kernel_functions[j], err);
                clCheckOrExit(err);
			}

			free((char*)kernel_src);
		}
	}
	else {
		cl_context context;
		cl_program program;
		cl_kernel* kernels;
	
		printf("  %-35s", "Creating context...");
		cl_device_id* ids = (cl_device_id*)malloc(count * sizeof(cl_device_id));
		for(unsigned int i = 0; i < count; i++) 
			ids[i] = devices[i].id;
		context = clCreateContext(0, count, ids, NULL, NULL, NULL);
		free(ids);
		printf("DONE!\n");
		
		printf("  %-35s", "Creating command queues...");	
		for(unsigned int i = 0; i < count; i++) {
			if(enable_gpu_profiling)
				output[i].queue = clCreateCommandQueue(context, output[i].info.id, CL_QUEUE_PROFILING_ENABLE, NULL);
			else
				output[i].queue = clCreateCommandQueue(context, output[i].info.id, CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE, NULL);
		}
		printf("DONE!\n");

		printf("  %-35s", "Loading kernel file...");
		const char* kernel_src;
		const size_t kernel_size;
		clLoadFile(kernel_path, &kernel_size, &kernel_src);
		printf("DONE!\n");

		printf("  %-35s", "Creating program with source...");
		program = clCreateProgramWithSource(context, 1, &kernel_src, &kernel_size, NULL);		
		printf("DONE!\n");

		printf("  %-35s", "Building program...");
		cl_int status = clBuildProgram(program, 0, NULL, build_options, NULL, NULL);
		if(status != CL_SUCCESS) {
			char build_log[2048];
			memset(build_log, 0, 2048);
			clGetProgramBuildInfo(program, devices[0].id, CL_PROGRAM_BUILD_LOG, 2048, build_log, NULL);
			printf("\n\nBUILD LOG: \n %s\n\n", build_log);
		}		
		printf("DONE!\n");

		kernels = (cl_kernel*)malloc(kernel_functions_count * sizeof(cl_kernel));
		for(unsigned int j = 0; j < kernel_functions_count; j++) {
			cl_int err;
			printf("  Creating kernel %s", kernel_functions[j]);
			kernels[j] = clCreateKernel(program, kernel_functions[j], &err);
			if(err == 0)
				printf("DONE!\n");
			else
				printf("FAIL! (%d)\n", err);
		}
		free((char*)kernel_src);

		for(unsigned int i = 0; i < count; i++) {
			output[i].platform = platform_id;
			output[i].context = context;
			output[i].program = program;
			output[i].kernels_count = kernel_functions_count;
			output[i].kernels = (cl_kernel*)malloc(kernel_functions_count * sizeof(cl_kernel));
			for(unsigned int j = 0; j < kernel_functions_count; j++) 
				output[i].kernels[j] = kernels[j];
			output[i].info.id = devices[i].id;
			output[i].info.type = devices[i].type;
			output[i].info.name = (char*)malloc(strlen(devices[i].name) + 1);
			memset(output[i].info.name, 0, strlen(devices[i].name) + 1);
			strncpy(output[i].info.name, devices[i].name, strlen(devices[i].name));
		}		
		free(kernels);
	}
}

void clFreeDeviceEnvironments(
	CLDeviceEnvironment* environments,
	unsigned int count,
	unsigned int shared) {
	if(environments) {
		if(!shared) {
			for(unsigned int i = 0 ; i < count; i++) {
				if(environments[i].context)
					clReleaseContext(environments[i].context);
				if(environments[i].program)
					clReleaseProgram(environments[i].program);
				if(environments[i].kernels) {
					for(unsigned int j = 0; j < environments[i].kernels_count; j++) {
						if(environments[i].kernels[j])
							clReleaseKernel(environments[i].kernels[j]);  
					}
					free(environments[i].kernels);
				}
				if(environments[i].queue) 
					clReleaseCommandQueue(environments[i].queue);
				clFreeDeviceInfo(&environments[i].info);
			}
		}
		else {
			clReleaseContext(environments[0].context);
			clReleaseProgram(environments[0].program);
			for(unsigned int j = 0; j < environments[0].kernels_count; j++) {
				if(environments[0].kernels[j])
					clReleaseKernel(environments[0].kernels[j]);  
			}
			clReleaseCommandQueue(environments[0].queue);
		
			for(unsigned int i = 0 ; i < count; i++) {
				free(environments[i].kernels);		
				clFreeDeviceInfo(&environments[i].info);
			}
		}
	}
}

void clErrorToString(
	cl_int error,
	char* output) {
    static const char* errorString[] = {
        "CL_SUCCESS",
        "CL_DEVICE_NOT_FOUND",
        "CL_DEVICE_NOT_AVAILABLE",
        "CL_COMPILER_NOT_AVAILABLE",
        "CL_MEM_OBJECT_ALLOCATION_FAILURE",
        "CL_OUT_OF_RESOURCES",
        "CL_OUT_OF_HOST_MEMORY",
        "CL_PROFILING_INFO_NOT_AVAILABLE",
        "CL_MEM_COPY_OVERLAP",
        "CL_IMAGE_FORMAT_MISMATCH",
        "CL_IMAGE_FORMAT_NOT_SUPPORTED",
        "CL_BUILD_PROGRAM_FAILURE",
        "CL_MAP_FAILURE",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "CL_INVALID_VALUE",
        "CL_INVALID_DEVICE_TYPE",
        "CL_INVALID_PLATFORM",
        "CL_INVALID_DEVICE",
        "CL_INVALID_CONTEXT",
        "CL_INVALID_QUEUE_PROPERTIES",
        "CL_INVALID_COMMAND_QUEUE",
        "CL_INVALID_HOST_PTR",
        "CL_INVALID_MEM_OBJECT",
        "CL_INVALID_IMAGE_FORMAT_DESCRIPTOR",
        "CL_INVALID_IMAGE_SIZE",
        "CL_INVALID_SAMPLER",
        "CL_INVALID_BINARY",
        "CL_INVALID_BUILD_OPTIONS",
        "CL_INVALID_PROGRAM",
        "CL_INVALID_PROGRAM_EXECUTABLE",
        "CL_INVALID_KERNEL_NAME",
        "CL_INVALID_KERNEL_DEFINITION",
        "CL_INVALID_KERNEL",
        "CL_INVALID_ARG_INDEX",
        "CL_INVALID_ARG_VALUE",
        "CL_INVALID_ARG_SIZE",
        "CL_INVALID_KERNEL_ARGS",
        "CL_INVALID_WORK_DIMENSION",
        "CL_INVALID_WORK_GROUP_SIZE",
        "CL_INVALID_WORK_ITEM_SIZE",
        "CL_INVALID_GLOBAL_OFFSET",
        "CL_INVALID_EVENT_WAIT_LIST",
        "CL_INVALID_EVENT",
        "CL_INVALID_OPERATION",
        "CL_INVALID_GL_OBJECT",
        "CL_INVALID_BUFFER_SIZE",
        "CL_INVALID_MIP_LEVEL",
        "CL_INVALID_GLOBAL_WORK_SIZE",
    };

    const int errorCount = sizeof(errorString)/sizeof(errorString[0]);
	const int index = -error;
	if (index >= 0 && index < errorCount) {
		output = (char*)malloc(strlen(errorString[index]) + 1);
		strncpy(output, errorString[index], strlen(errorString[index]));
		output[strlen(errorString[index])] = 0;
	}
	else {
		output = (char*)malloc(strlen("Unspecified Error") + 1);
		strncpy(output, "Unspecified Error", strlen("Unspecified Error"));
		output[strlen("Unspecified Error")] = 0;
	}
}
