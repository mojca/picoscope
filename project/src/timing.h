#ifndef __TIMING_H__
#define __TIMING_H__

#ifdef WIN32
#include "windows.h"
#else
// #include <sys/types.h>
// #include <string.h>
// #include "linux_utils.h"
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
#ifdef WIN32
	LARGE_INTEGER time_start;
	LARGE_INTEGER time_stop;
	LARGE_INTEGER ticksPerSecond;
#endif

	void GetTicksPerSecond();
};

#endif
