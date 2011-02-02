#ifndef __CHANNEL_H__
#define __CHANNEL_H__

class Measurement;
class Picoscope;

// #include "picoscope.h"
// #include "measurement.h"

class Channel {
public:
	Channel(int, Measurement*);
	~Channel();

	void Enable()           { is_enabled = true;  }
	void Disable()          { is_enabled = false; }
	bool IsEnabled() const  { return is_enabled;  }
	void SetVoltage(PICO_VOLTAGE);
	unsigned int GetVoltage() const { return voltage; } // has to be cast to (PS4000_RANGE/PS6000_RANGE)
	int GetIndex() const    { return index; }

	Measurement* GetMeasurement() const { return measurement; }
	Picoscope*   GetPicoscope()   const { return GetMeasurement()->GetPicoscope(); }
	PICO_SERIES  GetSeries()      const { return GetPicoscope()->GetSeries(); }
	short        GetHandle()      const { return GetPicoscope()->GetHandle(); }

	void SetChannelInPicoscope(); // passes the data to picoscope
private:
	Measurement* measurement; // to be able to access the variables and functions of parent measurement ...
	int index; // A, B, C, D

	bool is_enabled;
	unsigned int voltage;
};

#endif
