#include "CLDevice.h"

cl_int
clGetBestWorkGroupSize(CLDeviceInfo* device,
                    cl_uint* global_sizes,
                    cl_uint dimension_count,
                    cl_uint* local_sizes)
{
    cl_int status = CL_SUCCESS;
    
    //Get max work item dimensions
    cl_uint max_work_item_dims = 0;
    status = clGetDeviceInfo(device->id,
                             CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS,
                             sizeof(cl_uint),
                             &max_work_item_dims,
                             NULL);
    if(max_work_item_dims < dimension_count)
        return -1;
    
    //Get max work item sizes
    cl_uint max_work_item_sizes[1024];
    status = clGetDeviceInfo(device->id,
                             CL_DEVICE_MAX_WORK_ITEM_SIZES,
                             1024 * sizeof(size_t),
                             max_work_item_sizes,
                             NULL);
    
    // Clear local sizes
    for(cl_uint i = 0; i < dimension_count; i++)
        local_sizes[i] = max_work_item_sizes[i];
    
    // MCD global size and max local size
    for(cl_uint i = 0; i < dimension_count; i++) {
        if(local_sizes[i] > 1) {
            cl_uint global_size = global_sizes[i];
            cl_uint local_size = local_sizes[i];
            cl_uint mcd = global_size % local_size;
            while(mcd != 0) {
                global_size = local_size;
                local_size = mcd;
                mcd = global_size % local_size;
            }
            
            // If local size is 1 no MCD found: find bigger divisor
            if(local_size == 1) {
                for(local_size = local_sizes[i]; local_size >= 1; local_size--) {
                    if(global_sizes[i] % local_size == 0) {
                        local_sizes[i] = local_size;
                        break;
                    }
                }
            }
        }
    }
    return CL_SUCCESS;
}
                   
CLDeviceInfo* 
	clGetDeviceList(
		unsigned int* count) 
{
	cl_device_id device_ids[100];
	CLDeviceInfo* devices;
	cl_platform_id platform;
	clGetPlatformIDs(1, &platform, NULL);
	clGetDeviceIDs(platform, CL_DEVICE_TYPE_ALL, 100, device_ids, count);
	
	devices = (CLDeviceInfo*)malloc(*count * sizeof(CLDeviceInfo));
	for(unsigned int i = 0; i < *count; i++) {
		devices[i].id = device_ids[i];
		devices[i].name = (char*)malloc(256 * sizeof(char));
		cl_device_type type;
		clGetDeviceInfo(devices[i].id, CL_DEVICE_NAME, 256, devices[i].name, NULL);		
		clGetDeviceInfo(devices[i].id, CL_DEVICE_TYPE, sizeof(type), &type, NULL);
		devices[i].type = type;
 	}
	return devices;
}

void 
	clPrintDeviceList(
		CLDeviceInfo* devices,
		unsigned int count,
		const char* before_item) 
{
	for(unsigned int i = 0; i < count; i++) {
		char* device_type = NULL;
		clDeviceTypeToString(devices[i].type, &device_type);
		printf("%s%s[%s]\n", before_item, devices[i].name, device_type);
		free(device_type);
	}
}

void 
	clFreeDeviceInfo(
		CLDeviceInfo* device) 
{	
	free(device->name);
}

void 
	clDeviceTypeToString(
		cl_device_type type, 
		char** output) 
{
	char* temp = "DEFAULT";
	switch(type) {
		case CL_DEVICE_TYPE_GPU: {
			temp = "GPU";
		} break;
		case CL_DEVICE_TYPE_CPU: {
			temp = "CPU";
		} break;
		case CL_DEVICE_TYPE_ACCELERATOR: {
			temp = "ACCELERATOR";
		} break;
		default: {
			temp = "DEFAULT";
		}
	}
	*output = (char*)malloc(strlen(temp) + 1);
	memset(*output, 0, strlen(temp) + 1);
	strncpy(*output, temp, strlen(temp));
}

CLDeviceInfo* 
	clGetDeviceById(
		cl_device_id id) 
{	
	unsigned int count = 0;
	CLDeviceInfo* devices = clGetDeviceList(&count);
	CLDeviceInfo* info = NULL;
	for(unsigned int i = 0; i < count; i++) {
		if(devices[i].id == id) {
			info = (CLDeviceInfo*)malloc(sizeof(CLDeviceInfo));
			info->id = devices[i].id;
			info->name = (char*)malloc((strlen(devices[i].name) + 1) * sizeof(char));
			memset(info->name, 0, strlen(devices[i].name) + 1);
			strncpy(info->name, devices[i].name, strlen(devices[i].name));
			info->type = devices[i].type;
			break;
		}
	}
	for(unsigned int i = 0; i < count; i++) 
		clFreeDeviceInfo(&devices[i]);
	free(devices);
	return info;
}

cl_device_type 
	clGetDeviceType(
		cl_device_id device)
{
	cl_device_type t;
	clGetDeviceInfo(device, CL_DEVICE_TYPE, sizeof(cl_device_type), &t, NULL);
	return t;
}

CLDeviceCombination*
	clGetDeviceCombinations(
		cl_device_type device_type,
		unsigned int* comb_count) 
{
	cl_device_id device_ids[1024];
	unsigned int dev_count;
	cl_platform_id platform;
	clGetPlatformIDs(1, &platform, NULL);
	clGetDeviceIDs(platform, device_type, 1024, device_ids, &dev_count);
		
	CLDeviceCombination* combinations = (CLDeviceCombination*)malloc(dev_count * dev_count * sizeof(CLDeviceCombination));
	combinations[0].count = 0;
	unsigned int combination_count = 1;
	for(unsigned int i = 0; i < dev_count; i++) {
		int end_index = combination_count;
		for(unsigned int comb_index = 0; comb_index < end_index; comb_index++) {
			combinations[combination_count].count = combinations[comb_index].count + 1;
			combinations[combination_count].ids = (cl_device_id*)malloc(combinations[combination_count].count * sizeof(cl_device_id));
			for(unsigned int di = 0; di < combinations[comb_index].count; di++)
				combinations[combination_count].ids[di] = combinations[comb_index].ids[di];
			combinations[combination_count].ids[combinations[combination_count].count - 1] = device_ids[i];
			combination_count++;
		}
	}

	//Remove empty combination and sort
	for(unsigned int i = 1; i < combination_count; i++)
		combinations[i - 1] = combinations[i];
	combination_count--;
	for(unsigned int i = 1; i < combination_count; i++) {
		CLDeviceCombination saved = combinations[i];
		int j = i - 1;
		while(j >= 0 && saved.count < combinations[j].count) {
			combinations[j + 1] = combinations[j];
			j--;
		}
		if(j != i - 1)
			combinations[j + 1] = saved;
	}

	*comb_count = combination_count;
	return combinations;
}

char* 
	clGetDeviceName(
		cl_device_id device) 
{
	size_t tempSize = 0;
	clGetDeviceInfo(device, CL_DEVICE_NAME, 0, NULL, &tempSize);		
	char* name = (char*)malloc(tempSize);
	clGetDeviceInfo(device, CL_DEVICE_NAME, tempSize, name, NULL);
	return name;
}

void 
	clPrintDeviceSpecification(
		cl_device_id deviceId)
{
	cl_int status = CL_SUCCESS;
	cl_device_type dType;				/**< dType device type*/
	cl_uint venderId;					/**< vendorId VendorId of device*/
	cl_uint maxComputeUnits;			/**< maxComputeUnits maxComputeUnits of device*/
	cl_uint maxWorkItemDims;			/**< maxWorkItemDims maxWorkItemDimensions VendorId of device*/
	size_t* maxWorkItemSizes;			/**< maxWorkItemSizes maxWorkItemSizes of device*/
	size_t maxWorkGroupSize;			/**< maxWorkGroupSize max WorkGroup Size of device*/
	cl_uint preferredCharVecWidth;		/**< preferredCharVecWidth preferred Char VecWidth of device*/
	cl_uint preferredShortVecWidth;		/**< preferredShortVecWidth preferred Short VecWidth of device*/
	cl_uint preferredIntVecWidth;		/**< preferredIntVecWidth preferred Int VecWidth of device*/
	cl_uint preferredLongVecWidth;		/**< preferredLongVecWidth preferred Long VecWidth of device*/
	cl_uint preferredFloatVecWidth;		/**< preferredFloatVecWidth preferredFloatVecWidth of device*/
	cl_uint preferredDoubleVecWidth;	/**< preferredDoubleVecWidth preferred Double VecWidth of device*/
	cl_uint preferredHalfVecWidth;		/**< preferredHalfVecWidth preferred Half VecWidth of device*/
	cl_uint nativeCharVecWidth;		 	/**< nativeCharVecWidth native Char VecWidth of device*/
	cl_uint nativeShortVecWidth;		/**< nativeShortVecWidth nativeShortVecWidth of device*/
	cl_uint nativeIntVecWidth;			/**< nativeIntVecWidth nativeIntVecWidth of device*/
	cl_uint nativeLongVecWidth;			/**< nativeLongVecWidth nativeLongVecWidth of device*/
	cl_uint nativeFloatVecWidth;		/**< nativeFloatVecWidth native Float VecWidth of device*/
	cl_uint nativeDoubleVecWidth;		/**< nativeDoubleVecWidth native Double VecWidth of device*/
	cl_uint nativeHalfVecWidth;			/**< nativeHalfVecWidth native Half VecWidth of device*/
	cl_uint maxClockFrequency;			/**< maxClockFrequency max Clock Frequency of device*/
	cl_uint addressBits;				/**< addressBits address Bits of device*/
	cl_ulong maxMemAllocSize;			/**< maxMemAllocSize max Mem Alloc Size of device*/
	cl_bool imageSupport;				/**< imageSupport image Support of device*/
	cl_uint maxReadImageArgs;			/**< maxReadImageArgs max ReadImage Args of device*/
	cl_uint maxWriteImageArgs;			/**< maxWriteImageArgs max Write Image Args of device*/
	size_t image2dMaxWidth;				/**< image2dMaxWidth image 2dMax Width of device*/
	size_t image2dMaxHeight;			/**< image2dMaxHeight image 2dMax Height of device*/
	size_t image3dMaxWidth;				/**< image3dMaxWidth image3d MaxWidth of device*/ 
	size_t image3dMaxHeight;			/**< image3dMaxHeight image 3dMax Height of device*/
	size_t image3dMaxDepth;				/**< image3dMaxDepth image 3dMax Depth of device*/
	size_t maxSamplers;					/**< maxSamplers maxSamplers of device*/
	size_t maxParameterSize;			/**< maxParameterSize maxParameterSize of device*/
	cl_uint memBaseAddressAlign;		/**< memBaseAddressAlign memBase AddressAlign of device*/
	cl_uint minDataTypeAlignSize;		/**< minDataTypeAlignSize minDataType AlignSize of device*/
	cl_device_fp_config singleFpConfig;	/**< singleFpConfig singleFpConfig of device*/
	cl_device_fp_config doubleFpConfig; /**< doubleFpConfig doubleFpConfig of device*/
	cl_device_mem_cache_type globleMemCacheType; /**< globleMemCacheType globleMem CacheType of device*/
	cl_uint globalMemCachelineSize;		/**< globalMemCachelineSize globalMem Cacheline Size of device*/
	cl_ulong globalMemCacheSize;		/**< globalMemCacheSize globalMem CacheSize of device*/
	cl_ulong globalMemSize;				/**< globalMemSize globalMem Size of device*/
	cl_ulong maxConstBufSize;			/**< maxConstBufSize maxConst BufSize of device*/
	cl_uint maxConstArgs;				/**< maxConstArgs max ConstArgs of device*/
	cl_device_local_mem_type localMemType;/**< localMemType local MemType of device*/
	cl_ulong localMemSize;				/**< localMemSize localMem Size of device*/
	cl_bool errCorrectionSupport;		/**< errCorrectionSupport errCorrectionSupport of device*/
	cl_bool hostUnifiedMem;				/**< hostUnifiedMem hostUnifiedMem of device*/
	size_t timerResolution;				/**< timerResolution timerResolution of device*/
	cl_bool endianLittle;				/**< endianLittle endian Little of device*/
	cl_bool available;					/**< available available of device*/
	cl_bool compilerAvailable;			/**< compilerAvailable compilerAvailable of device*/
	cl_device_exec_capabilities execCapabilities;/**< execCapabilities exec Capabilities of device*/
	cl_command_queue_properties queueProperties;/**< queueProperties queueProperties of device*/
	cl_platform_id platform;			/**< platform platform of device*/
	char* name;							/**< name name of device*/
	char* vendorName;					/**< venderName vender Name of device*/
	char* driverVersion;				/**< driverVersion driver Version of device*/
	char* profileType;					/**< profileType profile Type of device*/
	char* deviceVersion;				/**< deviceVersion device Version of device*/
	char* openclCVersion;				/**< openclCVersion opencl C Version of device*/
	char* extensions;					/**< extensions extensions of device*/

    //Get device type
    status = clGetDeviceInfo(
                    deviceId, 
                    CL_DEVICE_TYPE, 
                    sizeof(cl_device_type), 
                    &dType, 
                    NULL);
    printf("%20s %llu\n", "CL_DEVICE_TYPE:", dType);

    //Get vender ID
    status = clGetDeviceInfo(
                    deviceId, 
                    CL_DEVICE_VENDOR_ID, 
                    sizeof(cl_uint), 
                    &venderId, 
                    NULL);
    printf("%20s %d\n", "CL_DEVICE_VENDOR_ID:", venderId);

    //Get max compute units
    status = clGetDeviceInfo(
                    deviceId, 
                    CL_DEVICE_MAX_COMPUTE_UNITS, 
                    sizeof(cl_uint), 
                    &maxComputeUnits, 
                    NULL);
    printf("%20s %d\n", "CL_DEVICE_VENDOR_ID:", venderId);

    //Get max work item dimensions
    status = clGetDeviceInfo(
                    deviceId, 
                    CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS,
                    sizeof(cl_uint),
                    &maxWorkItemDims,
                    NULL);
    printf("%20s %d\n", "CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS", maxWorkItemDims);

    //Get max work item sizes
    maxWorkItemSizes = (size_t*)malloc(maxWorkItemDims * sizeof(size_t));

    status = clGetDeviceInfo(
                    deviceId, 
                    CL_DEVICE_MAX_WORK_ITEM_SIZES,
                    maxWorkItemDims * sizeof(size_t),
                    maxWorkItemSizes,
                    NULL);
    printf("%20s %zu\n", "CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS", maxWorkItemSizes[0]);
	for(unsigned int i = 1; i < maxWorkItemDims; i++)
		printf("%20s %zu\n", "", maxWorkItemSizes[i]);
	free(maxWorkItemSizes);

    // Maximum work group size
    status = clGetDeviceInfo(
                    deviceId, 
                    CL_DEVICE_MAX_WORK_GROUP_SIZE,
                    sizeof(size_t),
                    &maxWorkGroupSize,
                    NULL);
	printf("%20s %zu\n", "CL_DEVICE_MAX_WORK_GROUP_SIZE", maxWorkGroupSize);

    // Preferred vector sizes of all data types
    status = clGetDeviceInfo(
                    deviceId, 
                    CL_DEVICE_PREFERRED_VECTOR_WIDTH_CHAR,
                    sizeof(cl_uint),
                    &preferredCharVecWidth,
                    NULL);
    printf("%20s %d\n", "CL_DEVICE_PREFERRED_VECTOR_WIDTH_CHAR", preferredCharVecWidth);

    status = clGetDeviceInfo(
                    deviceId, 
                    CL_DEVICE_PREFERRED_VECTOR_WIDTH_SHORT,
                    sizeof(cl_uint),
                    &preferredShortVecWidth,
                    NULL);
    printf("%20s %d\n", "CL_DEVICE_PREFERRED_VECTOR_WIDTH_SHORT", preferredShortVecWidth);

    status = clGetDeviceInfo(
                    deviceId, 
                    CL_DEVICE_PREFERRED_VECTOR_WIDTH_INT,
                    sizeof(cl_uint),
                    &preferredIntVecWidth,
                    NULL);
    printf("%20s %d\n", "CL_DEVICE_PREFERRED_VECTOR_WIDTH_INT", preferredIntVecWidth);

    status = clGetDeviceInfo(
                    deviceId, 
                    CL_DEVICE_PREFERRED_VECTOR_WIDTH_LONG,
                    sizeof(cl_uint),
                    &preferredLongVecWidth,
                    NULL);
    printf("%20s %d\n", "CL_DEVICE_PREFERRED_VECTOR_WIDTH_LONG", preferredLongVecWidth);

    status = clGetDeviceInfo(
                    deviceId, 
                    CL_DEVICE_PREFERRED_VECTOR_WIDTH_FLOAT,
                    sizeof(cl_uint),
                    &preferredFloatVecWidth,
                    NULL);
    printf("%20s %d\n", "CL_DEVICE_PREFERRED_VECTOR_WIDTH_FLOAT", preferredFloatVecWidth);

    status = clGetDeviceInfo(
                    deviceId, 
                    CL_DEVICE_PREFERRED_VECTOR_WIDTH_DOUBLE,
                    sizeof(cl_uint),
                    &preferredDoubleVecWidth,
                    NULL);
    printf("%20s %d\n", "CL_DEVICE_PREFERRED_VECTOR_WIDTH_DOUBLE", preferredDoubleVecWidth);

    status = clGetDeviceInfo(
                    deviceId, 
                    CL_DEVICE_PREFERRED_VECTOR_WIDTH_HALF,
                    sizeof(cl_uint),
                    &preferredHalfVecWidth,
                    NULL);
    printf("%20s %d\n", "CL_DEVICE_PREFERRED_VECTOR_WIDTH_HALF", preferredHalfVecWidth);

    // Clock frequency
    status = clGetDeviceInfo(
                    deviceId, 
                    CL_DEVICE_MAX_CLOCK_FREQUENCY,
                    sizeof(cl_uint),
                    &maxClockFrequency,
                    NULL);
    printf("%20s %d\n", "CL_DEVICE_MAX_CLOCK_FREQUENCY", maxClockFrequency);

    // Address bits
    status = clGetDeviceInfo(
                    deviceId, 
                    CL_DEVICE_ADDRESS_BITS,
                    sizeof(cl_uint),
                    &addressBits,
                    NULL);
    printf("%20s %d\n", "CL_DEVICE_ADDRESS_BITS", addressBits);

    // Maximum memory alloc size
    status = clGetDeviceInfo(
                    deviceId, 
                    CL_DEVICE_MAX_MEM_ALLOC_SIZE,
                    sizeof(cl_ulong),
                    &maxMemAllocSize,
                    NULL);
    printf("%20s %llu\n", "CL_DEVICE_MAX_MEM_ALLOC_SIZE", maxMemAllocSize);

    // Image support
    status = clGetDeviceInfo(
                    deviceId, 
                    CL_DEVICE_IMAGE_SUPPORT,
                    sizeof(cl_bool),
                    &imageSupport,
                    NULL);
    printf("%20s %s\n", "CL_DEVICE_IMAGE_SUPPORT", imageSupport? "true" : "false");

    // Maximum read image arguments
    status = clGetDeviceInfo(
                    deviceId, 
                    CL_DEVICE_MAX_READ_IMAGE_ARGS,
                    sizeof(cl_uint),
                    &maxReadImageArgs,
                    NULL);
    printf("%20s %d\n", "CL_DEVICE_MAX_READ_IMAGE_ARGS", maxReadImageArgs);

    // Maximum write image arguments
    status = clGetDeviceInfo(
                    deviceId, 
                    CL_DEVICE_MAX_WRITE_IMAGE_ARGS,
                    sizeof(cl_uint),
                    &maxWriteImageArgs,
                    NULL);
    printf("%20s %d\n", "CL_DEVICE_MAX_WRITE_IMAGE_ARGS", maxWriteImageArgs);

    // 2D image and 3D dimensions
    status = clGetDeviceInfo(
                    deviceId, 
                    CL_DEVICE_IMAGE2D_MAX_WIDTH,
                    sizeof(size_t),
                    &image2dMaxWidth,
                    NULL);
    printf("%20s %zu\n", "CL_DEVICE_IMAGE2D_MAX_WIDTH", image2dMaxWidth);

    status = clGetDeviceInfo(
                    deviceId, 
                    CL_DEVICE_IMAGE2D_MAX_HEIGHT,
                    sizeof(size_t),
                    &image2dMaxHeight,
                    NULL);
    printf("%20s %zu\n", "CL_DEVICE_IMAGE2D_MAX_HEIGHT", image2dMaxHeight);

    status = clGetDeviceInfo(
                    deviceId, 
                    CL_DEVICE_IMAGE3D_MAX_WIDTH,
                    sizeof(size_t),
                    &image3dMaxWidth,
                    NULL);
    printf("%20s %zu\n", "CL_DEVICE_IMAGE3D_MAX_WIDTH", image3dMaxWidth);

    status = clGetDeviceInfo(
                    deviceId, 
                    CL_DEVICE_IMAGE3D_MAX_HEIGHT,
                    sizeof(size_t),
                    &image3dMaxHeight,
                    NULL);
    printf("%20s %zu\n", "CL_DEVICE_IMAGE3D_MAX_HEIGHT", image3dMaxHeight);

    status = clGetDeviceInfo(
                    deviceId, 
                    CL_DEVICE_IMAGE3D_MAX_DEPTH,
                    sizeof(size_t),
                    &image3dMaxDepth,
                    NULL);
    printf("%20s %zu\n", "CL_DEVICE_IMAGE3D_MAX_DEPTH", image3dMaxDepth);

    // Maximum samplers
    status = clGetDeviceInfo(
                    deviceId, 
                    CL_DEVICE_MAX_SAMPLERS,
                    sizeof(cl_uint),
                    &maxSamplers,
                    NULL);
    printf("%20s %zu\n", "CL_DEVICE_MAX_SAMPLERS", maxSamplers);

    // Maximum parameter size
    status = clGetDeviceInfo(
                    deviceId, 
                    CL_DEVICE_MAX_PARAMETER_SIZE,
                    sizeof(size_t),
                    &maxParameterSize,
                    NULL);
    printf("%20s %zu\n", "CL_DEVICE_MAX_PARAMETER_SIZE", maxParameterSize);

    // Memory base address align
    status = clGetDeviceInfo(
                    deviceId, 
                    CL_DEVICE_MEM_BASE_ADDR_ALIGN,
                    sizeof(cl_uint),
                    &memBaseAddressAlign,
                    NULL);
    printf("%20s %d\n", "CL_DEVICE_MEM_BASE_ADDR_ALIGN", memBaseAddressAlign);

    // Minimum data type align size
    status = clGetDeviceInfo(
                    deviceId, 
                    CL_DEVICE_MIN_DATA_TYPE_ALIGN_SIZE,
                    sizeof(cl_uint),
                    &minDataTypeAlignSize,
                    NULL);
    printf("%20s %d\n", "CL_DEVICE_MIN_DATA_TYPE_ALIGN_SIZE", minDataTypeAlignSize);

    // Single precision floating point configuration
    status = clGetDeviceInfo(
                    deviceId, 
                    CL_DEVICE_SINGLE_FP_CONFIG,
                    sizeof(cl_device_fp_config),
                    &singleFpConfig,
                    NULL);
    printf("%20s %llu\n", "CL_DEVICE_SINGLE_FP_CONFIG", singleFpConfig);

    // Double precision floating point configuration
    status = clGetDeviceInfo(
                    deviceId, 
                    CL_DEVICE_DOUBLE_FP_CONFIG,
                    sizeof(cl_device_fp_config),
                    &doubleFpConfig,
                    NULL);
    printf("%20s %llu\n", "CL_DEVICE_DOUBLE_FP_CONFIG", doubleFpConfig);

    // Global memory cache type
    status = clGetDeviceInfo(
                    deviceId, 
                    CL_DEVICE_GLOBAL_MEM_CACHE_TYPE,
                    sizeof(cl_device_mem_cache_type),
                    &globleMemCacheType,
                    NULL);
    printf("%20s %d\n", "CL_DEVICE_GLOBAL_MEM_CACHE_TYPE", globleMemCacheType);

    // Global memory cache line size
    status = clGetDeviceInfo(
                    deviceId, 
                    CL_DEVICE_GLOBAL_MEM_CACHELINE_SIZE,
                    sizeof(cl_uint),
                    &globalMemCachelineSize,
                    NULL);
    printf("%20s %d\n", "CL_DEVICE_GLOBAL_MEM_CACHELINE_SIZE", globalMemCachelineSize);

    // Global memory cache size
    status = clGetDeviceInfo(
                    deviceId, 
                    CL_DEVICE_GLOBAL_MEM_CACHE_SIZE,
                    sizeof(cl_ulong),
                    &globalMemCacheSize,
                    NULL);
    printf("%20s %llu\n", "CL_DEVICE_GLOBAL_MEM_CACHE_SIZE", globalMemCacheSize);

    // Global memory size
    status = clGetDeviceInfo(
                    deviceId, 
                    CL_DEVICE_GLOBAL_MEM_SIZE,
                    sizeof(cl_ulong),
                    &globalMemSize,
                    NULL);
    printf("%20s %llu\n", "CL_DEVICE_GLOBAL_MEM_SIZE", globalMemSize);

    // Maximum constant buffer size
    status = clGetDeviceInfo(
                    deviceId, 
                    CL_DEVICE_MAX_CONSTANT_BUFFER_SIZE,
                    sizeof(cl_ulong),
                    &maxConstBufSize,
                    NULL);
    printf("%20s %llu\n", "CL_DEVICE_MAX_CONSTANT_BUFFER_SIZE", maxConstBufSize);

    // Maximum constant arguments
    status = clGetDeviceInfo(
                    deviceId, 
                    CL_DEVICE_MAX_CONSTANT_ARGS,
                    sizeof(cl_uint),
                    &maxConstArgs,
                    NULL);
   printf("%20s %d\n", "CL_DEVICE_MAX_CONSTANT_ARGS", maxConstArgs);

    // Local memory type
    status = clGetDeviceInfo(
                    deviceId, 
                    CL_DEVICE_LOCAL_MEM_TYPE,
                    sizeof(cl_device_local_mem_type),
                    &localMemType,
                    NULL);
    printf("%20s %d\n", "CL_DEVICE_LOCAL_MEM_TYPE", localMemType);

    // Local memory size
    status = clGetDeviceInfo(
                    deviceId, 
                    CL_DEVICE_LOCAL_MEM_SIZE,
                    sizeof(cl_ulong),
                    &localMemSize,
                    NULL);
    printf("%20s %llu\n", "CL_DEVICE_LOCAL_MEM_SIZE", localMemSize);

    // Error correction support
    status = clGetDeviceInfo(
                    deviceId, 
                    CL_DEVICE_ERROR_CORRECTION_SUPPORT,
                    sizeof(cl_bool),
                    &errCorrectionSupport,
                    NULL);
    printf("%20s %s\n", "CL_DEVICE_ERROR_CORRECTION_SUPPORT", errCorrectionSupport? "true" : "false");

    // Profiling timer resolution
    status = clGetDeviceInfo(
                    deviceId, 
                    CL_DEVICE_PROFILING_TIMER_RESOLUTION,
                    sizeof(size_t),
                    &timerResolution,
                    NULL);
   printf("%20s %zu\n", "CL_DEVICE_PROFILING_TIMER_RESOLUTION", timerResolution);

    // Endian little
    status = clGetDeviceInfo(
                    deviceId, 
                    CL_DEVICE_ENDIAN_LITTLE,
                    sizeof(cl_bool),
                    &endianLittle,
                    NULL);
    printf("%20s %s\n", "CL_DEVICE_ENDIAN_LITTLE", endianLittle? "true" : "false");

    // Device available
    status = clGetDeviceInfo(
                    deviceId, 
                    CL_DEVICE_AVAILABLE,
                    sizeof(cl_bool),
                    &available,
                    NULL);
    printf("%20s %s\n", "CL_DEVICE_AVAILABLE", available? "true" : "false");

    // Device compiler available
    status = clGetDeviceInfo(
                    deviceId, 
                    CL_DEVICE_COMPILER_AVAILABLE,
                    sizeof(cl_bool),
                    &compilerAvailable,
                    NULL);
    printf("%20s %s\n", "CL_DEVICE_COMPILER_AVAILABLE", compilerAvailable? "true" : "false");

    // Device execution capabilities
    status = clGetDeviceInfo(
                    deviceId, 
                    CL_DEVICE_EXECUTION_CAPABILITIES,
                    sizeof(cl_device_exec_capabilities),
                    &execCapabilities,
                    NULL);
    printf("%20s %llu\n", "CL_DEVICE_EXECUTION_CAPABILITIES", execCapabilities);

    // Device queue properities
    status = clGetDeviceInfo(
                    deviceId, 
                    CL_DEVICE_QUEUE_PROPERTIES,
                    sizeof(cl_command_queue_properties),
                    &queueProperties,
                    NULL);
    printf("%20s %llu\n", "CL_DEVICE_QUEUE_PROPERTIES", queueProperties);

    // Platform
    status = clGetDeviceInfo(
                    deviceId, 
                    CL_DEVICE_PLATFORM,
                    sizeof(cl_platform_id),
                    &platform,
                    NULL);
    printf("%20s %d\n", "CL_DEVICE_PLATFORM", (int)platform);

    // Device name
    size_t tempSize = 0;
    status = clGetDeviceInfo(
                    deviceId, 
                    CL_DEVICE_NAME,
                    0,
                    NULL,
                    &tempSize);
    name = (char*)malloc(tempSize);

    status = clGetDeviceInfo(
                    deviceId, 
                    CL_DEVICE_NAME,
                    sizeof(char) * tempSize,
                    name,
                    NULL);
    printf("%20s %s\n", "CL_DEVICE_NAME", name);
	free(name);

    // Vender name
    status = clGetDeviceInfo(
                    deviceId, 
                    CL_DEVICE_VENDOR,
                    0,
                    NULL,
                    &tempSize);
    vendorName = (char*)malloc(tempSize);

    status = clGetDeviceInfo(
                    deviceId, 
                    CL_DEVICE_VENDOR,
                    sizeof(char) * tempSize,
                    vendorName,
                    NULL);
    printf("%20s %s\n", "CL_DEVICE_VENDOR", vendorName);
	free(vendorName);

    // Driver name
    status = clGetDeviceInfo(
                    deviceId, 
                    CL_DRIVER_VERSION,
                    0,
                    NULL,
                    &tempSize);
    driverVersion = (char*)malloc(tempSize);

    status = clGetDeviceInfo(
                    deviceId, 
                    CL_DRIVER_VERSION,
                    sizeof(char) * tempSize,
                    driverVersion,
                    NULL);
    printf("%20s %s\n", "CL_DRIVER_VERSION", driverVersion);
	free(driverVersion);

    // Device profile
    status = clGetDeviceInfo(
                    deviceId, 
                    CL_DEVICE_PROFILE,
                    0,
                    NULL,
                    &tempSize);

    profileType = (char*)malloc(tempSize);

    status = clGetDeviceInfo(
                    deviceId, 
                    CL_DEVICE_PROFILE,
                    sizeof(char) * tempSize,
                    profileType,
                    NULL);
	printf("%20s %s\n", "CL_DEVICE_PROFILE", profileType);
	free(profileType);

    // Device version
    status = clGetDeviceInfo(
                    deviceId, 
                    CL_DEVICE_VERSION,
                    0,
                    NULL,
                    &tempSize);

    deviceVersion = (char*)malloc(tempSize);

    status = clGetDeviceInfo(
                    deviceId, 
                    CL_DEVICE_VERSION,
                    sizeof(char) * tempSize,
                    deviceVersion,
                    NULL);
    printf("%20s %s\n", "CL_DEVICE_VERSION", deviceVersion);
	free(deviceVersion);

    // Device extensions
    status = clGetDeviceInfo(
                    deviceId, 
                    CL_DEVICE_EXTENSIONS,
                    0,
                    NULL,
                    &tempSize);

    extensions = (char*)malloc(tempSize);

    status = clGetDeviceInfo(
                    deviceId, 
                    CL_DEVICE_EXTENSIONS,
                    sizeof(char) * tempSize,
                    extensions,
                    NULL);
    printf("%20s %s\n", "CL_DEVICE_EXTENSIONS", extensions);
	free(extensions);

    // Device parameters of OpenCL 1.1 Specification
#ifdef CL_VERSION_1_1
    char* vStart = strstr(deviceVersion, " ");
    char* vEnd = strstr(vStart, " ");
	char vStrVal[64] = { 0 };
	strncpy(vStrVal, vStart + 1, vEnd - vStart - 1);
    if(strcmp(vStrVal, "1.0") > 0)
    {
        // Native vector sizes of all data types
        status = clGetDeviceInfo(
                        deviceId, 
                        CL_DEVICE_NATIVE_VECTOR_WIDTH_CHAR,
                        sizeof(cl_uint),
                        &nativeCharVecWidth,
                        NULL);
        printf("%20s %d\n", "CL_DEVICE_NATIVE_VECTOR_WIDTH_CHAR", nativeCharVecWidth);

        status = clGetDeviceInfo(
                        deviceId, 
                        CL_DEVICE_NATIVE_VECTOR_WIDTH_SHORT,
                        sizeof(cl_uint),
                        &nativeShortVecWidth,
                        NULL);
        printf("%20s %d\n", "CL_DEVICE_NATIVE_VECTOR_WIDTH_SHORT", nativeShortVecWidth);

        status = clGetDeviceInfo(
                        deviceId, 
                        CL_DEVICE_NATIVE_VECTOR_WIDTH_INT,
                        sizeof(cl_uint),
                        &nativeIntVecWidth,
                        NULL);
        printf("%20s %d\n", "CL_DEVICE_NATIVE_VECTOR_WIDTH_INT", nativeIntVecWidth);

        status = clGetDeviceInfo(
                        deviceId, 
                        CL_DEVICE_NATIVE_VECTOR_WIDTH_LONG,
                        sizeof(cl_uint),
                        &nativeLongVecWidth,
                        NULL);
        printf("%20s %d\n", "CL_DEVICE_NATIVE_VECTOR_WIDTH_LONG", nativeLongVecWidth);

        status = clGetDeviceInfo(
                        deviceId, 
                        CL_DEVICE_NATIVE_VECTOR_WIDTH_FLOAT,
                        sizeof(cl_uint),
                        &nativeFloatVecWidth,
                        NULL);
        printf("%20s %d\n", "CL_DEVICE_NATIVE_VECTOR_WIDTH_FLOAT", nativeFloatVecWidth);

        status = clGetDeviceInfo(
                        deviceId, 
                        CL_DEVICE_NATIVE_VECTOR_WIDTH_DOUBLE,
                        sizeof(cl_uint),
                        &nativeDoubleVecWidth,
                        NULL);
        printf("%20s %d\n", "CL_DEVICE_NATIVE_VECTOR_WIDTH_DOUBLE", nativeDoubleVecWidth);

        status = clGetDeviceInfo(
                        deviceId, 
                        CL_DEVICE_NATIVE_VECTOR_WIDTH_HALF,
                        sizeof(cl_uint),
                        &nativeHalfVecWidth,
                        NULL);
        printf("%20s %d\n", "CL_DEVICE_NATIVE_VECTOR_WIDTH_HALF", nativeHalfVecWidth);

        // Host unified memory
        status = clGetDeviceInfo(
                        deviceId, 
                        CL_DEVICE_HOST_UNIFIED_MEMORY,
                        sizeof(cl_bool),
                        &hostUnifiedMem,
                        NULL);
        printf("%20s %s\n", "CL_DEVICE_HOST_UNIFIED_MEMORY", hostUnifiedMem ? "true" : "false");

        // Device OpenCL C version
        status = clGetDeviceInfo(
                        deviceId, 
                        CL_DEVICE_OPENCL_C_VERSION,
                        0,
                        NULL,
                        &tempSize);

		openclCVersion = (char*)malloc(tempSize);

        status = clGetDeviceInfo(
                        deviceId, 
                        CL_DEVICE_OPENCL_C_VERSION,
                        sizeof(char) * tempSize,
                        openclCVersion,
                        NULL);
        printf("%20s %s\n", "CL_DEVICE_OPENCL_C_VERSION", openclCVersion);
		free(openclCVersion);
    }
#endif
}
