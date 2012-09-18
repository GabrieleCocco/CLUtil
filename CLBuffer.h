#ifndef CL_THREADING_H
#define cL_THREADING_H

#include "CLThreading.h"

typedef void (*CLBufferAsyncCallback) (void*);

typedef struct CLReadBufferData 
{
	cl_command_queue queue;
	cl_mem buffer;
	cl_bool blocking;
	size_t offset;
	size_t size;
	void* ptr;
	cl_uint event_count;
	cl_event* wait_event;
	cl_event* gen_event;
} CLReadBufferData;

typedef struct CLWriteBufferData 
{
	cl_command_queue queue;
	cl_mem buffer;
	cl_bool blocking;
	size_t offset;
	size_t size;
	const void* ptr;
	cl_uint event_count;
	cl_event* wait_event;
	cl_event* gen_event;
} CLWriteBufferData;

typedef struct CLMemcpyData 
{
	void* src;
	void* dst;
	size_t size;
	size_t offset;
} CLMemcpyData;

typedef struct CLBufferAsyncState
{
	CLThreadAsyncState state;
	void** data;
	CLBufferAsyncCallback callback;
} CLBufferAsyncState;

void*
	clEnqueueReadBufferMTTask(
		void* pdata);

void*
	clEnqueueWriteBufferMTTask(
		void* pdata);

void*
	clMemcpyMTTask(
		void* pdata);

cl_int
	clEnqueueReadBufferMT(
		cl_uint thread_count,
		cl_command_queue queue,
		cl_mem buffer,
		size_t offset,
		size_t size,
		void* ptr,
		cl_uint event_count,
		cl_event* wait_event,
		cl_event** gen_event);

cl_int
	clEnqueueReadufferMTAsync(
		cl_uint thread_count,
		cl_command_queue queue,
		cl_mem buffer,
		size_t offset,
		size_t size,
		void* ptr,
		cl_uint event_count,
		cl_event* wait_event,
		cl_event** gen_event,
		CLBufferAsyncState* state);

cl_int
	clEnqueueWriteBufferMT(
		cl_uint thread_count,
		cl_command_queue queue,
		cl_mem buffer,
		size_t offset,
		size_t size,
		const void* ptr,
		cl_uint event_count,
		cl_event* wait_event,
		cl_event** gen_event);

cl_int
	clEnqueueWriteBufferMTAsync(
		cl_uint thread_count,
		cl_command_queue queue,
		cl_mem buffer,
		size_t offset,
		size_t size,
		const void* ptr,
		cl_uint event_count,
		cl_event* wait_event,
		cl_event** gen_event,
		CLBufferAsyncState* state);

cl_int
	clMemcpyMT(
		cl_uint thread_count,
		size_t size,
		void* src,
		void* dst);

cl_int
	clMemcpyMTAsync(
		cl_uint thread_count,
		size_t size,
		void* src,
		void* dst,
		CLBufferAsyncState* state);

cl_int
	clWaitBuffer(
		CLBufferAsyncState* state);

#endif