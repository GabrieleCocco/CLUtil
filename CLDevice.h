#ifndef CL_DEVICE_H
#define CL_DEVICE_H

#include "CLGeneral.h"

typedef struct CLDeviceCombination {
	cl_device_id* ids;
	unsigned int count;
} CLDeviceCombination;

typedef struct CLDeviceInfo {
	cl_device_id id;
	char* name;
	cl_device_type type;
} CLDeviceInfo;

cl_int
clGetBestWorkGroupSize(CLDeviceInfo* device,
                    cl_uint* global_sizes,
                    cl_uint dimension_count,
                    cl_uint* local_sizes);
CLDeviceInfo* 
	clGetDeviceList(
		unsigned int* count);

void 
	clPrintDeviceList(
		CLDeviceInfo* devices,
		unsigned int count,
		const char* before_item);

void 
	clPrintDeviceSpecification(
		cl_device_id device);

CLDeviceInfo* 
	clGetDeviceById(
		cl_device_id id);

char* 
	clGetDeviceName(
		cl_device_id device);

cl_device_type 
	clGetDeviceType(
		cl_device_id device);

CLDeviceCombination*
	clGetDeviceCombinations(
		cl_device_type device_type, 
		unsigned int* combination_count);

void 
	clDeviceTypeToString(
		cl_device_type type, 
		char** output);

void clFreeDeviceInfo(
	CLDeviceInfo* device);

#endif