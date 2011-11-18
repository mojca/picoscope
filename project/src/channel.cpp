#include <iostream>

#include "picoscope.h"
#include "measurement.h"
#include "channel.h"
#include "log.h"

#include "picoStatus.h"
#include "ps4000Api.h"
#include "ps6000Api.h"

// channel constructor
Channel::Channel(int i, Measurement* m)
{
	FILE_LOG(logDEBUG4) << "Channel::Channel(" << i << ", " << m << ")";

	index = i;
	measurement = m;
	Disable();
	// if nothing else is set, make sure that at least some voltage is there ..., that is the maximum
	SetVoltage(U_MAX);
}

Channel::~Channel()
{}

Measurement* Channel::GetMeasurement() const { return measurement; }
Picoscope*   Channel::GetPicoscope()   const { return GetMeasurement()->GetPicoscope(); }
PICO_SERIES  Channel::GetSeries()      const { return GetPicoscope()->GetSeries(); }
short        Channel::GetHandle()      const { return GetPicoscope()->GetHandle(); }

void Channel::SetVoltage(PICO_VOLTAGE u)
{
	// 6000
	if(GetSeries() == PICO_6000) {
		switch(u) {
			case(U_50mV ): voltage = (unsigned int)PS6000_50MV;  break;
			case(U_100mV): voltage = (unsigned int)PS6000_100MV; break;
			case(U_200mV): voltage = (unsigned int)PS6000_200MV; break;
			case(U_500mV): voltage = (unsigned int)PS6000_500MV; break;
			case(U_1V   ): voltage = (unsigned int)PS6000_1V;    break;
			case(U_2V   ): voltage = (unsigned int)PS6000_2V;    break;
			case(U_5V   ): voltage = (unsigned int)PS6000_5V;    break;
			case(U_10V  ): voltage = (unsigned int)PS6000_10V;   break;
			case(U_20V  ): voltage = (unsigned int)PS6000_20V;   break;
			case(U_50V  ): throw("you are now allowed to set 50V");   break;
			case(U_100V ): throw("you are now allowed to set 100V");  break;
			case(U_MAX  ): voltage = (unsigned int)PS6000_MAX_RANGES; break; // maybe PS6000_20V?
			default      : throw("unknown voltage setting; should not happen"); break;
		}
	// 4000
	} else {
		switch(u) {
			case(U_50mV ): voltage = (unsigned int)PS4000_50MV;  break;
			case(U_100mV): voltage = (unsigned int)PS4000_100MV; break;
			case(U_200mV): voltage = (unsigned int)PS4000_200MV; break;
			case(U_500mV): voltage = (unsigned int)PS4000_500MV; break;
			case(U_1V   ): voltage = (unsigned int)PS4000_1V;    break;
			case(U_2V   ): voltage = (unsigned int)PS4000_2V;    break;
			case(U_5V   ): voltage = (unsigned int)PS4000_5V;    break;
			case(U_10V  ): voltage = (unsigned int)PS4000_10V;   break;
			case(U_20V  ): voltage = (unsigned int)PS4000_20V;   break;
			case(U_50V  ): voltage = (unsigned int)PS4000_50V;   break;
			case(U_100V ): voltage = (unsigned int)PS4000_100V;  break;
			case(U_MAX  ): voltage = (unsigned int)PS4000_MAX_RANGES; break; // maybe PS4000_100V?
			default      : throw("unknown voltage setting; should not happen"); break;
		}
	}
}

// pass the data to picoscope
void Channel::SetChannelInPicoscope()
{
	// 4000
	if(GetSeries() == PICO_4000) {
		GetPicoscope()->SetStatus(ps4000SetChannel(
			GetHandle(),                  // handle
			(PS4000_CHANNEL)GetIndex(),   // channel
			(short)IsEnabled(),           // enabled
			(short)true,                  // TRUE for DC
			(PS4000_RANGE)GetVoltage())); // range = voltage
	// 6000
	} else {
		FILE_LOG(logDEBUG2) << "ps6000SetChannel(handle=" << GetHandle() << ", channel=" << GetIndex() << ", enabled=" << IsEnabled() << ", type=PS6000_DC_1M, range=" << GetVoltage() << ", analogueOffset=0.0, bandwidth=PS6000_BW_FULL)";
		GetPicoscope()->SetStatus(ps6000SetChannel(
			GetHandle(),                // handle
			(PS6000_CHANNEL)GetIndex(), // channel
			(short)IsEnabled(),         // enabled
			PS6000_DC_1M,               // type
			(PS6000_RANGE)GetVoltage(), // range = voltage
			0.0,                        // analogueOffset
			PS6000_BW_FULL));           // bandwidth
	}
	if(GetPicoscope()->GetStatus() != PICO_OK) {
		FILE_LOG(logERROR) << "Channel::SetChannelInPicoscope - Unable to setup the channel with index " << GetIndex() << "/" << index << " " <<  (char)('A'+GetIndex());

		std::cerr << "Unable to setup the channel " << (char)('A'+GetIndex()) << std::endl;
		throw Picoscope::PicoscopeException(GetPicoscope()->GetStatus());
	}
}


