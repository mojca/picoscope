#include "timing.h"

#ifdef _WIN32
#include "windows.h"
#else
#include <time.h>
#endif

Timing::Timing()
{
#ifdef _WIN32
	GetTicksPerSecond();
	QueryPerformanceCounter(&time_start);
#else
	clock_gettime(CLOCK_MONOTONIC, &time_start);
#endif
}

void Timing::GetTicksPerSecond()
{
#ifdef _WIN32
	QueryPerformanceFrequency(&ticksPerSecond);
#else
// <nothing>
#endif
}

void Timing::Start()
{
#ifdef _WIN32
	QueryPerformanceCounter(&time_start);
#else
	// clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time_start);
	clock_gettime(CLOCK_MONOTONIC, &time_start);
#endif
}

void Timing::Stop()
{
#ifdef _WIN32
	QueryPerformanceCounter(&time_stop);
#else
	clock_gettime(CLOCK_MONOTONIC, &time_stop);
#endif
}

double Timing::GetSecondsDouble()
{
#ifdef _WIN32
	return ((double)time_stop.QuadPart - (double)time_start.QuadPart)/((double)ticksPerSecond.QuadPart);
#else
	return (time_stop.tv_sec - time_start.tv_sec) + (time_stop.tv_nsec - time_start.tv_nsec)*1e-9;
#endif
}
