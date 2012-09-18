#ifndef CL_TIMER_H
#define CL_TIMER_H

#include "CLGeneral.h"

typedef struct CLTimer 
{
#ifdef _WIN32
	LARGE_INTEGER frequency;
	LARGE_INTEGER start_time;
	void start() {
		QueryPerformanceFrequency(&frequency);	
		QueryPerformanceCounter(&start_time);
	}
	double get() {
		LARGE_INTEGER end;
		QueryPerformanceCounter(&end);
		double elapsedTime = (end.QuadPart - start_time.QuadPart) * 1000.0 / frequency.QuadPart;
		return elapsedTime;
	}
#elif __APPLE__
    struct timeval t1, t2;
} CLTimer;
#endif

void 
	clStartConditionalTimer(CLTimer* timer, bool condition);

double
	clGetConditionalTimer(CLTimer* timer, bool condition);

#endif