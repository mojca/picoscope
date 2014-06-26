#ifndef __TIMING_H__
#define __TIMING_H__

#ifdef _WIN32
#include "windows.h"
#else
#include <time.h>
#endif

// LARGE_INTEGER ticksPerSecond;
// LARGE_INTEGER time1;
// LARGE_INTEGER time2;

class Timing {
public:
	Timing();
	~Timing() {};

	void Start();
	void Stop();
	double GetSecondsDouble();
private:
#ifdef _WIN32
	LARGE_INTEGER time_start;
	LARGE_INTEGER time_stop;
	LARGE_INTEGER ticksPerSecond;
#else
	struct timespec time_start;
	struct timespec time_stop;
#endif

	void GetTicksPerSecond();
};

#endif
