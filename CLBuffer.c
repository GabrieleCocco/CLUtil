#include "CLBuffer.h"

void internalFreeReadBufferData(
	void* pdata) 
{
	CLBufferAsyncState* state = (CLBufferAsyncState*)pdata;
	CLReadBufferData** data = (CLReadBufferData**)state->data;
	for(unsigned int i = 0; i < state->state.count; i++) 
		free(data[i]);
	free(data);
}

void internalFreeWriteBufferData(
	void* pdata) 
{
	CLBufferAsyncState* state = (CLBufferAsyncState*)pdata;
	CLWriteBufferData** data = (CLWriteBufferData**)state->data;
	for(unsigned int i = 0; i < state->state.count; i++) 
		free(data[i]);
	free(data);
}

void internalFreeMemcpyData(
	void* pdata) 
{
	CLBufferAsyncState* state = (CLBufferAsyncState*)pdata;
	CLMemcpyData** data = (CLMemcpyData**)state->data;
	for(unsigned int i = 0; i < state->state.count; i++) 
		free(data[i]);
	free(data);
}

void*
	clEnqueueReadBufferMTTask(
		void* pdata)
{
	CLReadBufferData* data = (CLReadBufferData*)pdata;
	clEnqueueReadBuffer(
		data->queue,
		data->buffer,
		data->blocking,
		data->offset,
		data->size,
		data->ptr,
		data->event_count,
		data->wait_event,
		data->gen_event);
    
    pthread_exit(0);
}

void*
	clEnqueueWriteBufferMTTask(
		void* pdata)
{
	CLWriteBufferData* data = (CLWriteBufferData*)pdata;
	clEnqueueWriteBuffer(
		data->queue,
		data->buffer,
		data->blocking,
		data->offset,
		data->size,
		data->ptr,
		data->event_count,
		data->wait_event,
        data->gen_event);
    
    pthread_exit(0);
}

void*
	clMemcpyMTTask(
		void* pdata)
{
	CLMemcpyData* data = (CLMemcpyData*)pdata;
	memcpy((char*)data->dst + data->offset, data->src, data->size);
    pthread_exit(0);
}

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
		cl_event** gen_event) 
{
	cl_int size_per_thread = (size - offset) / thread_count;
	CLReadBufferData** data = (CLReadBufferData**)malloc(thread_count * sizeof(CLReadBufferData*));
	for(unsigned int i = 0; i < thread_count; i++) {
		data[i] = (CLReadBufferData*)malloc(sizeof(CLReadBufferData));
		data[i]->buffer = buffer;
		data[i]->queue = queue;
		data[i]->blocking = true;
		data[i]->offset = offset + (size_per_thread * i);
		data[i]->size = size_per_thread;
		data[i]->ptr = (char*)ptr + (offset + (size_per_thread * i));
		data[i]->event_count = event_count;
		data[i]->wait_event = wait_event;
		data[i]->gen_event = gen_event == NULL ? NULL : gen_event[i];
	}

	cl_int result = clRunAndWaitThread(
		thread_count,
		(void**)data,
		clEnqueueReadBufferMTTask);
	
	for(unsigned int i = 0; i < thread_count; i++) 
		free(data[i]);
	free(data);

	return result;
}

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
		CLBufferAsyncState* state) 
{
	cl_int size_per_thread = (size - offset) / thread_count;
	CLReadBufferData** data = (CLReadBufferData**)malloc(thread_count * sizeof(CLReadBufferData*));
	for(unsigned int i = 0; i < thread_count; i++) {
		data[i] = (CLReadBufferData*)malloc(sizeof(CLReadBufferData));
		data[i]->buffer = buffer;
		data[i]->queue = queue;
		data[i]->blocking = false;
		data[i]->offset = offset + (size_per_thread * i);
		data[i]->size = size_per_thread;
		data[i]->ptr = (char*)ptr + (offset + (size_per_thread * i));
		data[i]->event_count = event_count;
		data[i]->wait_event = wait_event;
		data[i]->gen_event = gen_event == NULL ? NULL : gen_event[i];
	}

	state->data = (void**)data;
	state->callback = internalFreeReadBufferData;
	
	return clRunThread(
		thread_count,
		(void**)data,
		clEnqueueReadBufferMTTask,
		&state->state);
}

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
		cl_event** gen_event) 
{
	cl_int size_per_thread = (size - offset) / thread_count;
	CLWriteBufferData** data = (CLWriteBufferData**)malloc(thread_count * sizeof(CLWriteBufferData*));
	for(unsigned int i = 0; i < thread_count; i++) {
		data[i] = (CLWriteBufferData*)malloc(sizeof(CLWriteBufferData));
		data[i]->buffer = buffer;
		data[i]->queue = queue;
		data[i]->blocking = true;
		data[i]->offset = offset + (size_per_thread * i);
		data[i]->size = size_per_thread;
		data[i]->ptr = (char*)ptr + (offset + (size_per_thread * i));
		data[i]->event_count = event_count;
		data[i]->wait_event = wait_event;
		data[i]->gen_event = gen_event == NULL ? NULL : gen_event[i];
	}

	cl_int result = clRunAndWaitThread(
		thread_count,
		(void**)data,
		clEnqueueWriteBufferMTTask);

	for(unsigned int i = 0; i < thread_count; i++) 
		free(data[i]);
	free(data);

	return result;
}

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
		CLBufferAsyncState* state) 
{
	cl_int size_per_thread = (size - offset) / thread_count;
	CLWriteBufferData** data = (CLWriteBufferData**)malloc(thread_count * sizeof(CLWriteBufferData*));
	for(unsigned int i = 0; i < thread_count; i++) {
		data[i] = (CLWriteBufferData*)malloc(thread_count * sizeof(CLWriteBufferData));
		data[i]->buffer = buffer;
		data[i]->queue = queue;
		data[i]->blocking = false;
		data[i]->offset = offset + (size_per_thread * i);
		data[i]->size = size_per_thread;
		data[i]->ptr = (char*)ptr + (offset + (size_per_thread * i));
		data[i]->event_count = event_count;
		data[i]->wait_event = wait_event;
		data[i]->gen_event = gen_event == NULL ? NULL : gen_event[i];
	}

	state->data = (void**)data;
	state->callback = internalFreeWriteBufferData;

	return clRunThread(
		thread_count,
		(void**)data,
		clEnqueueWriteBufferMTTask,
		&state->state);
}

cl_int
	clMemcpyMT(
		cl_uint thread_count,
		size_t size,
		void* src,
		void* dst)
{
	cl_int size_per_thread = size / thread_count;

	CLMemcpyData** data = (CLMemcpyData**)malloc(thread_count * sizeof(CLMemcpyData*));
	for(unsigned int i = 0; i < thread_count; i++) {
		data[i] = (CLMemcpyData*)malloc(sizeof(CLMemcpyData));
		data[i]->src = src;
		data[i]->dst = dst;
		data[i]->offset = (size_per_thread * i);
		data[i]->size = size_per_thread;
	}

	cl_int result = clRunAndWaitThread(
		thread_count,
		(void**)data,
		clMemcpyMTTask);

	for(unsigned int i = 0; i < thread_count; i++) 
		free(data[i]);
	free(data);

	return result;
}

cl_int
	clMemcpyMTAsync(
		cl_uint thread_count,
		size_t size,
		void* src,
		void* dst,
		CLBufferAsyncState* state)
{
	cl_int size_per_thread = size / thread_count;

	CLMemcpyData* data = (CLMemcpyData*)malloc(thread_count * sizeof(CLMemcpyData));
	for(unsigned int i = 0; i < thread_count; i++) {
		data[i].src = src;
		data[i].dst = dst;
		data[i].offset = (size_per_thread * i);
		data[i].size = size_per_thread;
	}
	
	state->data = (void**)data;
	state->callback = internalFreeMemcpyData;

	return clRunThread(
		thread_count,
		(void**)&data,
		clMemcpyMTTask,
		&state->state);
}

cl_int
	clWaitBuffer(
		CLBufferAsyncState* state)
{
	// Wait thread
	clWaitThread(&state->state);

	// Call callback to free
	state->callback(state);

	return 0;
}
