#include "CLCommandQueueInfo.h"

CLCommandQueueInfo* 
	clCreateCommandQueueInfo(cl_command_queue queue, char* name) 
{
	CLCommandQueueInfo* info = (CLCommandQueueInfo*)malloc(sizeof(CLCommandQueueInfo));
	info->queue = queue;
	info->name = (char*)malloc(strlen(name) + 1);
	memset(info->name, 0, strlen(name) + 1);
	strncpy(info->name, name, strlen(name));

	return info;
}

void 
	clFreeCommandQueueInfo(CLCommandQueueInfo* info) 
{
	free(info->name);
}