#ifndef CL_SIZE_H
#define CL_SIZE_H

#include "CLGeneral.h"

typedef struct CLSize1D {
	unsigned int x_size;
} CLSize1D;

typedef struct CLSize2D {
	unsigned int x_size;
	unsigned int y_size;
} CLSize2D;

typedef struct CLSize3D {
	unsigned int x_size;
	unsigned int y_size;
	unsigned int z_size;
} CLSize3D;

#endif