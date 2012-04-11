#include "timing.h"

#ifdef WIN32
#include "windows.h"

Timing::Timing()
{
	GetTicksPerSecond();
	QueryPerformanceCounter(&time_start);
}

void Timing::GetTicksPerSecond()
{
	QueryPerformanceFrequency(&ticksPerSecond);
}

void Timing::Start()
{
	QueryPerformanceCounter(&time_start);
}

void Timing::Stop()
{
	QueryPerformanceCounter(&time_stop);
}

double Timing::GetSecondsDouble()
{
	return ((double)time_stop.QuadPart - (double)time_start.QuadPart)/((double)ticksPerSecond.QuadPart);
}


#else
// #include <sys/types.h>
// #include <string.h>
// #include "linux_utils.h"


Timing::Timing()
{
	// GetTicksPerSecond();
	// QueryPerformanceCounter(&time_start);
}

void Timing::GetTicksPerSecond()
{
	// QueryPerformanceFrequency(&ticksPerSecond);
}

void Timing::Start()
{
	// QueryPerformanceCounter(&time_start);
}

void Timing::Stop()
{
	// QueryPerformanceCounter(&time_stop);
}

double Timing::GetSecondsDouble()
{
	// return ((double)time_stop.QuadPart - (double)time_start.QuadPart)/((double)ticksPerSecond.QuadPart);
	return 0;
}

#endif

