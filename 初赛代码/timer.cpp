#include "common.h"
#include <iostream>
#include <sys/timeb.h>
#include <time.h>


#ifdef WIN32
#include <windows.h>

LARGE_INTEGER startCount;
LARGE_INTEGER endCount;
LARGE_INTEGER freq;



void timer_start()
{

	QueryPerformanceFrequency(&freq);
	QueryPerformanceCounter(&startCount);
}

//ms
double timer_elapse()
{
	QueryPerformanceCounter(&endCount);
	double elapsed = (double)(endCount.QuadPart - startCount.QuadPart) / freq.QuadPart * 1000;

	return elapsed;
}

double timer_elapse_print(const char * head)
{
	QueryPerformanceCounter(&endCount);
	double elapsed = (double)(endCount.QuadPart - startCount.QuadPart) / freq.QuadPart * 1000;
	printf("%s : %f ms\n", head,elapsed);
	return elapsed;
}

#else


static int ms = 0;
static unsigned long s = 0;

void timer_start()
{
	struct timeb rawtime;
	ftime(&rawtime);

	ms = rawtime.millitm;
	s = rawtime.time;
}

//ms
unsigned long timer_elapse()
{
	struct timeb rawtime;
	ftime(&rawtime);

	int out_ms = rawtime.millitm - ms;
	unsigned long out_s = rawtime.time - s;

	return out_ms + out_s * 1000;
}

#endif

