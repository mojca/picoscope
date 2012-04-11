#include <iostream>
#include "picoscope.h"
// #include "args.h"
#include "ps4000Api.h"
#include "ps6000Api.h"
// #include "picoStatus.h"

#include "log.h"

#ifdef WIN32
#include "windows.h"
#include <conio.h>
#else
#include <sys/types.h>
#include <string.h>
#include "linux_utils.h"
#endif

// #include <stdio.h>

// #include <sys/types.h>
// #include <time.h>

using namespace std;

int main(int argc, char** argv)
{
	short handle;
	int return_status;
	float time_interval_ns;
	unsigned long ntraces = 6700; // TODO: input parameter
	unsigned long max_length;

	try {
		FILELog::ReportingLevel() = FILELog::FromString("DEBUG4");
		FILE_LOG(logDEBUG2) << "ps6000OpenUnit(&handle, serial=NULL)";
		return_status = ps6000OpenUnit(&handle, NULL);
		FILE_LOG(logDEBUG2) << "-> handle=" << handle;
		if (return_status != PICO_OK) throw Picoscope::PicoscopeException(return_status);

		sleep(20);

		// ps6000SetChannel
		FILE_LOG(logDEBUG2) << "ps6000SetChannel(handle=" << handle << ", channel=" << 0 << ", enabled=" << 1 << ", type=PS6000_DC_1M, range=PS6000_5V, analogueOffset=0.0, bandwidth=PS6000_BW_FULL)";
		return_status = ps6000SetChannel(handle, (PS6000_CHANNEL)0, 1, PS6000_DC_1M, PS6000_5V, 0.0, PS6000_BW_FULL);
		if (return_status != PICO_OK) throw Picoscope::PicoscopeException(return_status);

		sleep(20);

		// FILE_LOG(logDEBUG2) << "ps6000SetChannel(handle=" << handle << ", channel=" << 1 << ", enabled=" << 0 << ", type=PS6000_DC_1M, range=PS6000_5V, analogueOffset=0.0, bandwidth=PS6000_BW_FULL)";
		// return_status = ps6000SetChannel(handle, (PS6000_CHANNEL)1, 0, PS6000_DC_1M, PS6000_5V, 0.0, PS6000_BW_FULL);
		// if (return_status != PICO_OK) throw Picoscope::PicoscopeException(return_status);
		// FILE_LOG(logDEBUG2) << "ps6000SetChannel(handle=" << handle << ", channel=" << 2 << ", enabled=" << 1 << ", type=PS6000_DC_1M, range=PS6000_5V, analogueOffset=0.0, bandwidth=PS6000_BW_FULL)";
		// return_status = ps6000SetChannel(handle, (PS6000_CHANNEL)2, 1, PS6000_DC_1M, PS6000_5V, 0.0, PS6000_BW_FULL);
		// if (return_status != PICO_OK) throw Picoscope::PicoscopeException(return_status);
		// FILE_LOG(logDEBUG2) << "ps6000SetChannel(handle=" << handle << ", channel=" << 3 << ", enabled=" << 0 << ", type=PS6000_DC_1M, range=PS6000_5V, analogueOffset=0.0, bandwidth=PS6000_BW_FULL)";
		// return_status = ps6000SetChannel(handle, (PS6000_CHANNEL)3, 0, PS6000_DC_1M, PS6000_5V, 0.0, PS6000_BW_FULL);
		// if (return_status != PICO_OK) throw Picoscope::PicoscopeException(return_status);
		
		// ps6000GetTimebase
/*		FILE_LOG(logDEBUG2) << "ps6000GetTimebase2(handle=" << handle << ", timebase=3, length=100, &time_interval_ns, oversample=0, maxSamples=NULL, segmentIndex=0)";
		return_status = ps6000GetTimebase2(handle, 3, 100, &time_interval_ns, 0, NULL, 0);
		FILE_LOG(logDEBUG2) << "-> time_interval_ns=" << time_interval_ns << " ns";
		if (return_status != PICO_OK) throw Picoscope::PicoscopeException(return_status);
*/		
		// ps6000SetTriggerChannelConditions,
		// ps6000SetTriggerChannelDirections
		// ps6000SetTriggerChannelProperties
		// ps6000MemorySegments
/*		FILE_LOG(logDEBUG2) << "ps6000MemorySegments(handle=" << handle << ", nSegments=" << ntraces << ", &max_length)";
		return_status = ps6000MemorySegments(handle, ntraces, &max_length);
		FILE_LOG(logDEBUG2) << "-> max_length=" << max_length << ", return_status=" << return_status;
		if (return_status != PICO_OK) throw Picoscope::PicoscopeException(return_status);
*/
		// ps6000Stop
		FILE_LOG(logDEBUG2) << "ps6000Stop(handle=" << handle << ")";
		return_status = ps6000Stop(handle);
		if (return_status != PICO_OK) throw Picoscope::PicoscopeException(return_status);

	} catch(Picoscope::PicoscopeException& ex) {
		cerr << "Some picoscope exception:" << endl
		     << ex.GetErrorMessage() << " (Error number " << ex.GetErrorNumber() << ")" << endl
		     << '(' << ex.GetVerboseErrorMessage() << ')' << endl;
		try {
			// pico.Close();
		} catch(...) {}
	} catch(Picoscope::PicoscopeUserException& ex) {
		cerr << "Some exception:" << endl
		     << ex.GetErrorMessage() << endl;
	// catch any exceptions
	} catch(...) {
		cout << "Some exception has occurred" << endl;
	}
	return 0;
}
