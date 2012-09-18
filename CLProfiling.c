#include "CLProfiling.h"

double 
	clToMillis(struct timeval time) 
{	
	double millis = 0;
	millis += time.tv_sec * 1000;
	millis += ((double)time.tv_usec / 1000.0);

	return millis;
}

void 
	clZeroTime(struct timeval* time) 
{
    time->tv_sec = 0;
    time->tv_usec = 0;
}

double
	clGetEventTime(
		cl_device_id device,
		cl_event ev) 
{
	cl_int err = CL_SUCCESS;
	cl_ulong start_time, end_time;
	err = clGetEventProfilingInfo(ev, CL_PROFILING_COMMAND_START, sizeof(cl_ulong), &start_time, NULL);
	err = clGetEventProfilingInfo(ev, CL_PROFILING_COMMAND_END, sizeof(cl_ulong), &end_time, NULL);

	return ((double)(end_time - start_time)) / (double) (1000000);
}

double
	clGetEventTime2(
		cl_device_id device,
		cl_event ev1,
		cl_event ev2) 
{
	cl_int err = CL_SUCCESS;
	cl_ulong start_time, end_time;
	err = clGetEventProfilingInfo(ev1, CL_PROFILING_COMMAND_START, sizeof(cl_ulong), &start_time, NULL);
	err = clGetEventProfilingInfo(ev2, CL_PROFILING_COMMAND_END, sizeof(cl_ulong), &end_time, NULL);

	return ((double)(end_time - start_time)) / (double) (1000000);
}

int 
	clComputeSamples(
		CLInitFunction init,
		CLRunFunction run,
		CLReleaseFunction release,
		void* data,
		unsigned int profiling_duration,
		CLSetSamplesFunction set_samples)
{	
	int samples = 10;
	if(profiling_duration != 0) {
		while(true) {
			CLCommandQueueInfo* queues;
			unsigned int queue_count;
		
			double completion_time;
			CLProfilingResult* results;
		
			set_samples(data, samples);

			init(data, &queues, &queue_count);
			run(&completion_time, &results);
			release();
			free(results);
			
			for(cl_uint i = 0; i < queue_count; i++)
				clFreeCommandQueueInfo(&queues[i]);
			free(queues);
		
			printf("Taken %f ms for %d samples\n", completion_time, samples);
			if(completion_time == 0) {
				samples *= 10;
				if(samples >= 100000)
					return samples;
			}
			else {
				samples = (unsigned int)ceil(((double)profiling_duration * (double)samples) / (double)completion_time);
				return (int)fmin(samples, 100000);
			}
		}
	}
	return 1;
}

CLProfilingResult*
	clAverageResults(
		CLProfilingResult** temp_results,
		unsigned int device_count,
		unsigned int sample_count)
{
	CLProfilingResult* results = (CLProfilingResult*)malloc(device_count * sizeof(CLProfilingResult));
	for(unsigned int dev_index = 0; dev_index < device_count; dev_index++) {
		results[dev_index].exit_code = 0;
		results[dev_index].alloc_time = 0;
		results[dev_index].init_time = 0;
		results[dev_index].exec_time = 0;
		results[dev_index].read_time = 0;
		results[dev_index].kernel_time = 0;
		for(unsigned int i = 0; i < sample_count; i++) {
			results[dev_index].exit_code = results[dev_index].exit_code == 0 ? temp_results[i][dev_index].exit_code : results[dev_index].exit_code;
			results[dev_index].alloc_time += temp_results[i][dev_index].alloc_time;
			results[dev_index].init_time += temp_results[i][dev_index].init_time;
			results[dev_index].exec_time += temp_results[i][dev_index].exec_time;
			results[dev_index].read_time += temp_results[i][dev_index].read_time;
			results[dev_index].kernel_time += temp_results[i][dev_index].kernel_time;
		}
		results[dev_index].alloc_time /= sample_count;
		results[dev_index].init_time /= sample_count;
		results[dev_index].exec_time /= sample_count;
		results[dev_index].read_time /= sample_count;
		results[dev_index].kernel_time /= sample_count;
	}
	return results;
}

void 
	clPrintProfilingResult(
		CLProfilingResult* result,
		double total_time,
		char* name,
		int padding) 
{
	for(int i = 0; i < padding; i++)
		printf(" ");

	for(int i = 0; i < 5; i++)
		printf("=");
	printf(" Profiling result %-10s ", name);
	for(int i = 0; i < 5; i++)
		printf("=");
	printf("\n");
	
	for(int i = 0; i < padding; i++)
		printf(" ");
	printf("%-27s %12d\n", "Exit code:", result->exit_code);

	for(int i = 0; i < padding; i++)
		printf(" ");
	printf("%-27s %8.3f ms\n", "Alloc time:", result->alloc_time);
	
	for(int i = 0; i < padding; i++)
		printf(" ");
	printf("%-27s %8.3f ms\n", "Init time:",  result->init_time);
	
	for(int i = 0; i < padding; i++)
		printf(" ");
	printf("%-27s %8.3f ms\n", "Exec time:", result->exec_time);
	
	for(int i = 0; i < padding; i++)
		printf(" ");
	printf("%-27s %8.3f ms\n", "Read time:", result->read_time);
	
	for(int i = 0; i < padding; i++)
		printf(" ");
	printf("%-27s %8.3f ms\n", "Total time:", total_time);
	
	for(int i = 0; i < padding; i++)
		printf(" ");
	for(int i = 0; i < 39; i++)
		printf("=");
	printf("\n");
}