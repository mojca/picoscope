#ifndef __TRIGGER_H__
#define __TRIGGER_H__

#include "picoscope.h"
#include "measurement.h"
#include "channel.h"

class Trigger {
public:
	Trigger(Channel *, double, double);
	~Trigger() {};

	// void SetSimpleTrigger(Channel*, double, double);
	void SetTriggerInPicoscope();

	// Channel*     GetChannel()     const { return channel; };
	// Measurement* GetMeasurement() const { return GetChannel()->GetMeasurement(); };
	// Picoscope*   GetPicoscope()   const { return GetMeasurement()->GetPicoscope(); };
	// PICO_SERIES  GetSeries()      const { return GetPicoscope()->GetSeries(); };
	// short        GetHandle()      const { return GetPicoscope()->GetHandle(); };

	Channel*     GetChannel()     const;
	Measurement* GetMeasurement() const;
	Picoscope*   GetPicoscope()   const;
	PICO_SERIES  GetSeries()      const;
	short        GetHandle()      const;

	double GetXFraction() const { return x_frac; }
	double GetYFraction() const { return y_frac; }
private:
	Channel *channel;
	// Measurement *measurement;
	// x_frac has to be between 0 and 1 (0 by default)
	// y_frac has to be between -1 and 1 (0 by default)
	double x_frac, y_frac;
	// just a simple trigger on a single channel
	// PICO_CHANNEL channel;
};

#endif
