//
//  CLUtil.h
//  CLUtil
//
//  Created by Gabriele Cocco on 7/21/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#ifndef CL_OPTION_H
#define CL_OPTION_H

#include "CLDevice.h"

typedef enum CLOptionType {
    CL_INT_OPTION,
    CL_FLOAT_OPTION,
    CL_DOUBLE_OPTION,
    CL_STRING_OPTION,
    CL_BOOL_OPTION ,
    CL_DEVICE_OPTION
} CLOptionType;

typedef struct CLOption {
    char* short_name;
    char* long_name;
    char* description;
    void* value;
    CLOptionType type;
} CLOption;

void 
clShowOptions(CLOption* options, 
              cl_uint count);
void 
clParseOptions(CLOption* options, 
               cl_uint option_count, 
               int argc, 
               const char * argv[]);
#endif
