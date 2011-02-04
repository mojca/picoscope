#include "windows.h"

#include "timing.h"

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
