#ifndef CL_MEMORY_SETTING_H
#define CL_MEMORY_SETTING_H

#include "CLGeneral.h"

typedef struct CLMemorySetting 
{
	char* name;
	cl_mem_flags flags;
	unsigned int mapping;
} CLMemorySetting;

void 
clCreateMemorySetting(char* name, 
                      cl_mem_flags flags,
                      unsigned int mapping,
                      CLMemorySetting* setting);

void 
clFreeMemorySetting(CLMemorySetting* setting);

char* 
clMemFlagsToString(cl_mem_flags flags);

cl_mem_flags 
clStringToMemFlags(char* flags);

#endif