#ifndef CL_PROFILING_H
#define CL_PROFILING_H

#include "CLCommandQueueInfo.h"
#include "CLTimer.h"

typedef struct CLProfilingResult 
{
	cl_int exit_code;
	double alloc_time;
	double init_time;
	double exec_time;
	double read_time;
	double kernel_time;
} CLProfilingResult;

typedef void (*CLRunFunction) (double* completion_time, CLProfilingResult** result);
typedef void (*CLInitFunction) (void* arguments, CLCommandQueueInfo** queues, unsigned int* queues_count);
typedef void (*CLReleaseFunction) ();
typedef void (*CLSetSamplesFunction) (void*, unsigned int);

double 
	clToMillis(struct timeval time);
void 
	clZeroTime(struct timeval* time);

double
	clGetEventTime(
		cl_device_id device,
		cl_event ev);

double
	clGetEventTime2(
		cl_device_id device,
		cl_event ev1,
		cl_event ev2); 

int 
	clComputeSamples(
		CLInitFunction init,
		CLRunFunction run,
		CLReleaseFunction release,
		void* data,
		unsigned int profiling_duration,
		CLSetSamplesFunction set_samples);

CLProfilingResult*
	clAverageResults(
		CLProfilingResult** temp_results,
		unsigned int device_count,
		unsigned int sample_count);

void 
	clPrintProfilingResult(
		CLProfilingResult* result,
		double total_time,
		char* name,
		int padding);
#endif