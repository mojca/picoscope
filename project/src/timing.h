#ifndef __TIMING_H__
#define __TIMING_H__

#include <windows.h>

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
	LARGE_INTEGER time_start;
	LARGE_INTEGER time_stop;
	LARGE_INTEGER ticksPerSecond;

	void GetTicksPerSecond();
};

#endif
