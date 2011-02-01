#ifndef __MEASUREMENT_H__
#define __MEASUREMENT_H__

#include "picoscope.h"

class Measurement {
public:
	Measurement(Picoscope pico);
	~Measurement();

	unsigned long      GetTimebase();
	unsigned long long GetLength();
	unsigned long      GetFileLength();
	unsigned long      GetMemLength();

	void EnableChannels(bool a, bool b, bool c, bool d);

	class Channel {
	public:
		void Enable()           { is_enabled = true;  }
		void Disable()          { is_enabled = false; }
		bool IsEnabled()        { return is_enabled;  }
		void SetVoltage();
	private:
		bool is_enabled;
		
	};

private:
	unsigned long      timebase;
	unsigned long long length;
	unsigned long      fileLength; // length of a single file
	unsigned long      memLength;  // length of buffer in memory (used for transfering data)

	Channel channels[PICOSCOPE_N_CHANNELS];
	short *data[PICOSCOPE_N_CHANNELS];
};

#endif
