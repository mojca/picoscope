#include <iostream>
#include <conio.h>
#include "windows.h"
#include "math.h"

#include "picoscope.h"
#include "measurement.h"
#include "channel.h"

#include "picoStatus.h"
#include "ps4000Api.h"
#include "ps6000Api.h"

Measurement::Measurement(Picoscope *p)
{
	int i;

	picoscope = p;
	max_memory_consumption    = 0;
	max_trace_length_to_fetch = 0;

	for(i=0; i<GetNumberOfChannels(); i++) {
		// initialize the channels
		channels[i]=new Channel(i, this);
	}
	// only a temporary setting; this needs to be fixed if more than a single channel is enabled
	timebase = 0UL;
}

Measurement::~Measurement()
{
	int i;
	for(i=0; i<GetNumberOfChannels(); i++) {
		// delete the channels
		delete channels[i];
	}
}
// TODO: improve this; check for number of channels
void Measurement::EnableChannels(bool a, bool b, bool c, bool d)
{
	/*if(GetSeries() == PICO_4000) {
		if(a) channels[PS4000_CHANNEL_A].Enable(); else channels[PS4000_CHANNEL_A].Disable();
		if(b) channels[PS4000_CHANNEL_B].Enable(); else channels[PS4000_CHANNEL_B].Disable();
		if(c) channels[PS4000_CHANNEL_C].Enable(); else channels[PS4000_CHANNEL_C].Disable();
		if(d) channels[PS4000_CHANNEL_D].Enable(); else channels[PS4000_CHANNEL_D].Disable();
	}*/
	if(a) GetChannel(0)->Enable(); else GetChannel(0)->Disable();
	if(b) GetChannel(1)->Enable(); else GetChannel(1)->Disable();
	if(c) GetChannel(2)->Enable(); else GetChannel(2)->Disable();
	if(d) GetChannel(3)->Enable(); else GetChannel(3)->Disable();

	// if memory constraint has been set, make sure that
	if(GetMaxMemoryConsuption() != 0) {
		SetMaxMemoryConsumption(max_memory_consumption);
	}
}

// set the maximum possible timebase; TODO: this may depend on the number of channels,
// so this is not really safe
// void SetMaxTimebase()
// {
// 	timebase = 0UL;
// }

void Measurement::SetTimebaseInPs(unsigned long picoseconds)
{
	unsigned long i, result;

	if(GetSeries() == PICO_6000) {
		// it doesn't go beyond a certain minimum level
		if(picoseconds < 200) {
			picoseconds = 200;
		}
		if(picoseconds < 6400) {
			// it could be round() instead of floor()
			timebase = (unsigned long)floor(log2((double)(picoseconds/200)));
		} else {
			timebase = (unsigned long)(picoseconds/6400+4);
		}
	} else {
		// TODO
		timebase = 0UL;
	}
}

// when multiple channels are enabled we cannot have the highest sampling rate
void Measurement::FixTimebase()
{
	int n = GetNumberOfEnabledChannels();
	// when 3-4 channels are enabled, we need to set at least timebase=2 (4 times slower sampling)
	if(n >= 3) {
		if(timebase < 2UL) {
			timebase = 2UL;
		}
	} else if(n == 2) {
		if((GetChannel(0)->IsEnabled() && GetChannel(1)->IsEnabled()) ||
		   (GetChannel(2)->IsEnabled() && GetChannel(3)->IsEnabled())) {
			if(timebase < 2UL) {
				timebase = 2UL;
			}
		} else {
			if(timebase < 1UL) {
				timebase = 1UL;
			}
		}
	}
}

// this sets the timebase to picoscope
void Measurement::SetTimebaseInPicoscope()
{
	// 4000
	if(GetSeries() == PICO_4000) {
		GetPicoscope()->SetStatus(ps4000GetTimebase2(
			GetHandle(),   // handle
			GetTimebase(), // timebase
			GetLength(),   // noSamples
			NULL,          // timeIntervalNanoseconds
			0,             // oversample (TODO if needed)
			NULL,          // maxSamples - the maximum number of samples available
			0));           // segmentIndex - the index of the memory segment to use
	// 6000
	} else {
		GetPicoscope()->SetStatus(ps6000GetTimebase2(
			GetHandle(),   // handle
			GetTimebase(), // timebase
			GetLength(),   // noSamples
			NULL,          // timeIntervalNanoseconds
			0,             // oversample (TODO if needed)
			NULL,          // maxSamples - the maximum number of samples available
			0));           // segmentIndex - the index of the memory segment to use
	}
	if(GetPicoscope()->GetStatus() != PICO_OK) {
		std::cerr << "Unable to set the requested timebase (" << GetTimebase()
		          << ") and sample number (" << GetLength() << ")" << std::endl;
		throw Picoscope::PicoscopeException(GetPicoscope()->GetStatus());
	}
}

void Measurement::SetLength(unsigned long l)
{
	length = l;
}

// sets the maximum allowed memory consumption
// and the maximum trace length to be read at once
void Measurement::SetMaxMemoryConsumption(unsigned long bytes)
{
	int enabled_channels = GetNumberOfEnabledChannels();

	max_memory_consumption    = bytes;
	if (enabled_channels > 0) {
		max_trace_length_to_fetch = bytes/(sizeof(short)*enabled_channels);
	} else {
		max_trace_length_to_fetch = bytes;
	}
	assert(enabled_channels<=4);
}

int Measurement::GetNumberOfChannels() const
{
	return GetPicoscope()->GetNumberOfChannels();
}

int Measurement::GetNumberOfEnabledChannels()
{
	int i, enabled = 0;

	for(i=1; i<4; i++) {
		if(GetChannel(i)->IsEnabled()) {
			enabled++;
		}
	}

	return enabled;
}

Channel* Measurement::GetChannel(int ch)
{
	int n = GetNumberOfChannels();
	if(ch>=0 && ch<n) {
		return channels[ch];
	} else {
		char message[200];
		sprintf(message, "Picoscope has only channels 0-%d. You cannot request channel with number %d",
		                 GetNumberOfChannels(), ch);
		throw(message);
		return NULL;
	}
}

void Measurement::RunBlock()
{
	int i;
	// test if channel settings have already been passed to picoscope
	// and only pass them again if that isn't the case
	for(i=0; i<GetNumberOfChannels(); i++) {
		GetChannel(i)->SetChannelInPicoscope();
	}
	// this fixes the timebase if more than a single channel is selected
	FixTimebase();
	// timebase
	SetTimebaseInPicoscope();

	if(GetSeries() == PICO_4000) {
		// TODO
	} else {
		GetPicoscope()->SetStatus(ps6000RunBlock(
			GetHandle(),   // handle
			0,             // noOfPreTriggerSamples
			GetLength(),   // noOfPostTriggerSamples
			timebase,      // timebase
			1,             // ovesample
			NULL,          // *timeIndisposedMs
			0,             // segmentIndex
			CallBackBlock, // lpReady
			NULL));        // *pParameter
	}
	if(GetPicoscope()->GetStatus() != PICO_OK) {
		std::cerr << "Unable to start collecting samples" << std::endl;
		throw Picoscope::PicoscopeException(GetPicoscope()->GetStatus());
	} else {
		std::cerr << "Start collecting samples" << std::endl;
	}
	while (!Picoscope::IsReady() && !_kbhit()) {
		Sleep(0);
	}
	std::cerr << "Stop collecting samples" << std::endl;
}
