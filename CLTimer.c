#include "CLTimer.h"

void 
	clStartConditionalTimer(CLTimer* timer, bool condition)
{
	if(condition) {
        gettimeofday(&timer->t1, NULL);
    }
}

double
	clGetConditionalTimer(CLTimer* timer, bool condition)
{
	if(condition) {
        gettimeofday(&timer->t2, NULL);
		double elapsedTime = ((double)(timer->t2.tv_sec - timer->t1.tv_sec) * 1000);
        elapsedTime += ((double)(timer->t2.tv_usec - timer->t1.tv_usec) / 1000.0);
		return elapsedTime;  
    }
	else
		return 0;
}