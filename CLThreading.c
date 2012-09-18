#include "CLThreading.h"

cl_int 
	clRunAndWaitThread(
		unsigned int count,
		void** data,
		void *(*start_routine) (void*))
{
	pthread_t* thread_handle = (pthread_t*)malloc(count * sizeof(pthread_t));
	cl_int thread_result = CL_SUCCESS;
	
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
    
	for(unsigned int i = 0; i < count; i++) {        
        int result = pthread_create(&thread_handle[i], &attr, start_routine, data[i]);			
		if(result) 
			return (cl_int)result;
	}
    
    pthread_attr_destroy(&attr);
    
	for(unsigned int i = 0; i < count; i++) {
        void *status;
        int result = pthread_join(thread_handle[i], &status);
		if(result != CL_SUCCESS)
			thread_result = result;
	}
	free(thread_handle);

	return thread_result;
}
	
cl_int 
	clRunThread(
		unsigned int count,
		void** data,
		void *(*start_routine) (void*),
		CLThreadAsyncState* state) 
{
	state->count = 0;
	state->handles = (pthread_t*)malloc(count * sizeof(pthread_t));
    
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
    
	for(unsigned int i = 0; i < count; i++) {        
        int result = pthread_create(&state->handles[i], &attr, start_routine, data[i]);			
		if(result) 
			return (cl_int)result;
        
		state->count++;
	}
    
    pthread_attr_destroy(&attr);
	return 0;
}
	
cl_int 
	clWaitThread(
		CLThreadAsyncState* async_state)
{			
	cl_int thread_result = CL_SUCCESS;
	for(unsigned int i = 0; i < async_state->count; i++) {		
        void *status;
        int result = pthread_join(async_state->handles[i], &status);
		if(result != CL_SUCCESS)
			thread_result = result;
	}
	free(async_state->handles);
	return thread_result;
}

cl_int 
	clRunSingleThread(
	void* data,
	void *(*start_routine) (void*),
	CLThreadAsyncState* async_state) 
{
	return clRunThread(1, &data, start_routine, async_state);
}