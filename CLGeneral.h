#ifndef CL_GENERAL_H
#define CL_GENERAL_H

#include <OpenCL/OpenCL.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/time.h>
#include <math.h>
#include <pthread.h>

void
clCheckOrExit(cl_int status);

#endif