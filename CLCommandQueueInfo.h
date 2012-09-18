#ifndef CL_COMMAND_QUEUE_INFO_H
#define CL_COMMAND_QUEUE_INFO_H

#include "CLGeneral.h"

typedef struct CLCommandQueueInfo 
{
	cl_command_queue queue;
	char* name;
} CLCommandQueueInfo;

CLCommandQueueInfo* 
	clCreateCommandQueueInfo(cl_command_queue queue, char* name);

void 
	clFreeCommandQueueInfo(CLCommandQueueInfo* info);

#endif