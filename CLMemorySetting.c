#include "CLMemorySetting.h"

void 
clCreateMemorySetting(char* name, 
                      cl_mem_flags flags,
                    unsigned int mapping,
                      CLMemorySetting* setting) 
{	
	setting->name = (char*)malloc(strlen(name) + 1);
	memset(setting->name, 0, strlen(name) + 1);
	strncpy(setting->name, name, strlen(name));

	setting->flags = flags;
	setting->mapping = mapping;
}

void 
clFreeMemorySetting(CLMemorySetting* setting) 
{
		free(setting->name);
}

char* 
clMemFlagsToString(cl_mem_flags flags) 
{	
	char* output = (char*)malloc(256);
	memset(output, 0, 256);
	unsigned int pointer = 0;
#ifdef CL_MEM_USE_PERSISTENT_MEM_AMD
	if(flags & CL_MEM_USE_PERSISTENT_MEM_AMD) {
		char* temp = "CL_MEM_USE_PERSISTENT_MEM_AMD";
		strncpy(output + pointer, temp, strlen(temp));	
		pointer += strlen(temp);
	}
#endif
	if(flags & CL_MEM_ALLOC_HOST_PTR) {
		char* temp = "CL_MEM_ALLOC_HOST_PTR";
		strncpy(output + pointer, temp, strlen(temp));	
		pointer += strlen(temp);
	}
	else if(flags & CL_MEM_USE_HOST_PTR) {
		char* temp = "CL_MEM_USE_HOST_PTR";
		strncpy(output + pointer, temp, strlen(temp));	
		pointer += strlen(temp);
	}
	else if(flags & CL_MEM_COPY_HOST_PTR) {
		char* temp = "CL_MEM_COPY_HOST_PTR";
		strncpy(output + pointer, temp, strlen(temp));	
		pointer += strlen(temp);
	}
	else {
		char* temp = "DEVICE COPY";
		strncpy(output + pointer, temp, strlen(temp));	
		pointer += strlen(temp);
	}
    
	if(flags & CL_MEM_READ_ONLY) {
		char* temp = " | CL_MEM_READ_ONLY";
		strncpy(output + pointer, temp, strlen(temp));	
		pointer += strlen(temp);
	}
	else if(flags & CL_MEM_WRITE_ONLY) {
		char* temp = " | CL_MEM_WRITE_ONLY";
		strncpy(output + pointer, temp, strlen(temp));	
		pointer += strlen(temp);
	}
	else {
		char* temp = " | CL_MEM_READ_WRITE";
		strncpy(output + pointer, temp, strlen(temp));	
		pointer += strlen(temp);
	}
#ifdef CL_MEM_HOST_READ_ONLY
	if(flags & CL_MEM_HOST_READ_ONLY) {
		char* temp = " | CL_MEM_HOST_READ_ONLY";
		strncpy(output + pointer, temp, strlen(temp));	
		pointer += strlen(temp);
	}	
	else if(flags & CL_MEM_HOST_WRITE_ONLY) {
		char* temp = " | CL_MEM_HOST_WRITE_ONLY";
		strncpy(output + pointer, temp, strlen(temp));	
		pointer += strlen(temp);
	}
	else if(flags & CL_MEM_HOST_NO_ACCESS) {
		char* temp = " | CL_MEM_HOST_NO_ACCESS";
		strncpy(output + pointer, temp, strlen(temp));	
		pointer += strlen(temp);
	}
	else {
		char* temp = " | CL_MEM_HOST_READ_WRITE";
		strncpy(output + pointer, temp, strlen(temp));	
		pointer += strlen(temp);
	}
#endif
	return output;
}

cl_mem_flags 
clStringToMemFlags(char* flags) 
{	
    cl_mem_flags mem_flags = 0;
#ifdef CL_MEM_USE_PERSISTENT_MEM_AMD
	if(strstr(flags, "CL_MEM_USE_PERSISTENT_MEM_AMD") != NULL)
        mem_flags |= CL_MEM_USE_PERSISTENT_MEM_AMD;
#endif
	if(strstr(flags, "CL_MEM_ALLOC_HOST_PTR") != NULL)
        mem_flags |= CL_MEM_ALLOC_HOST_PTR;
	if(strstr(flags, "CL_MEM_USE_HOST_PTR") != NULL)
        mem_flags |= CL_MEM_USE_HOST_PTR;    
	if(strstr(flags, "CL_MEM_COPY_HOST_PTR") != NULL)
        mem_flags |= CL_MEM_COPY_HOST_PTR;
    
	if(strstr(flags, "CL_MEM_READ_ONLY") != NULL)
        mem_flags |= CL_MEM_READ_ONLY;
	else if(strstr(flags, "CL_MEM_WRITE_ONLY") != NULL)
        mem_flags |= CL_MEM_WRITE_ONLY;
    else
        mem_flags |= CL_MEM_READ_WRITE;
    
#ifdef CL_MEM_HOST_READ_ONLY
	if(strstr(flags, "CL_MEM_HOST_READ_ONLY") != NULL)
        mem_flags |= CL_MEM_HOST_READ_ONLY;
	else if(strstr(flags, "CL_MEM_HOST_WRITE_ONLY") != NULL)
        mem_flags |= CL_MEM_HOST_WRITE_ONLY;
    else if(strstr(flags, "CL_MEM_HOST_NO_ACCESS") != NULL)
        mem_flags |= CL_MEM_HOST_NO_ACCESS;
#endif
	return mem_flags;
}