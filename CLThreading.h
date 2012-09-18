#ifndef CL_THREADING_H
#define CL_THREADING_H

#include "CLGeneral.h"

typedef struct CLThreadAsyncState
{
	pthread_t* handles;
	unsigned int count;
} CLThreadAsyncState;

cl_int 
	clRunAndWaitThread(
		unsigned int count,
		void** data,
		void *(*start_routine) (void*));

cl_int 
	clRunThread(
		unsigned int count,
		void** data,
		void *(*start_routine) (void*),
		CLThreadAsyncState* async_state);

cl_int 
	clWaitThread(
		CLThreadAsyncState* async_state);
	
cl_int 
	clRunSingleThread(
        void* data,
		void *(*start_routine) (void*),
		CLThreadAsyncState* async_state);

#endif