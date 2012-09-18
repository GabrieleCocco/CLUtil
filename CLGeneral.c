//
//  CLGeneral.c
//  CLUtil
//
//  Created by Gabriele Cocco on 7/23/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#include "CLGeneral.h"

void
clCheckOrExit(cl_int status) {
    if(status != CL_SUCCESS) {
        printf("OpenCL error %d: exit...", status);
        pthread_exit((void*)status);
    }        
}

