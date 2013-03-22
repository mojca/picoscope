#ifndef __MEASUREMENT_H__
#define __MEASUREMENT_H__

// typedef enum picoChannel
// {
// 	CHANNEL_A,
// 	CHANNEL_B,
// 	CHANNEL_C,
// 	CHANNEL_D //,
// //	PS_EXTERNAL,
// //	PS_MAX_CHANNELS = PS_EXTERNAL,
// //	PS_TRIGGER_AUX,
// //	PS_MAX_TRIGGER_SOURCES
// } PICO_CHANNEL;

#include <vector>

#include "picoscope.h"
#include "channel.h"
#include "trigger.h"

// TODO: get rid of this dependency
#include "ps6000Api.h"

// class Picoscope;
// class Channel;
class Trigger;


/*
	- filename
	- are we going to use type 4000 or 6000 osciloscope?
	- which channels have to be enabled?
	- what rate (either "samples per second" or "time difference")
	- what length
	- what's the maximum memory usage
	- voltage
	- do we want to store in text or binary format or both?
 */


class Measurement {
public:
	Measurement(Picoscope *pico);
	~Measurement();

	// TODO: create a complete set
	// void SetMaxTimebase();
	void SetTimebaseInPs(unsigned long);
	void SetTimebaseInNs(unsigned long);
	void FixTimebase();
	double GetTimebaseInNs();
	double GetReportedTimebaseInNs() const { return timebase_reported_by_osciloscope; };

	bool IsTriggered() const { return is_triggered; };
	void SetTimebaseInPicoscope(); // sends the information to picoscope

	void SetLength(unsigned long);
	void SetNTraces(unsigned long);

	unsigned long      GetTimebase()   const { return timebase; };
	unsigned long      GetLength()     const { return length;   };
	unsigned long      GetNTraces()    const { return ntraces;  };
	// unsigned long GetFileLength() const;
	unsigned long      GetMemLength()  const;
	unsigned long      GetLengthBeforeTrigger();
	unsigned long      GetLengthAfterTrigger();

	void EnableChannels(bool a, bool b, bool c, bool d);
	int  GetNumberOfChannels() const;
	int  GetNumberOfEnabledChannels();

	void          SetMaxMemoryConsumption(unsigned long bytes);
	unsigned long GetMaxMemoryConsuption() const { return max_memory_consumption; };
	unsigned long GetMaxTraceLengthToFetch() const { return max_trace_length_to_fetch; };
	unsigned long GetMaxTracesToFetch() const { return max_traces_to_fetch; };
	void          AllocateMemoryBlock(unsigned long);
	void          AllocateMemoryRapidBlock(unsigned long);

	Picoscope*  GetPicoscope()  const { return picoscope; };
	PICO_SERIES GetSeries()     const { return GetPicoscope()->GetSeries(); };
	short       GetHandle()     const { return GetPicoscope()->GetHandle(); };
	Trigger*    GetTrigger()    const { return trigger; };
	Channel*    GetChannel(int);

	void RunBlock();
	unsigned long GetNextData();
	unsigned long GetNextDataBulk();
	void WriteDataBin(FILE*,int);
	void WriteDataTxt(FILE*,int);

	std::vector<int8_t> GetDataVector(int channel, int index);

	unsigned long GetNextIndex() const { return next_index; };
	void SetLengthFetched(unsigned long l);
	unsigned long GetLengthFetched() const { return length_fetched; };

	void AddSimpleTrigger(Channel *, double, double);
	void SetTrigger(Trigger *);

	void AddSignalGeneratorSquare(unsigned long peak_to_peak_in_microvolts, float frequency);
	void InitializeSignalGenerator();

	void SetRate(long n_events, long long t1, PS6000_TIME_UNITS time_unit1, long long t2, PS6000_TIME_UNITS time_unit2);
	double GetRatePerSecond();

private:
	Picoscope         *picoscope;
	Trigger           *trigger;
	unsigned long      timebase;
	double             timebase_reported_by_osciloscope;
	unsigned long      length;
	unsigned long      ntraces;
	unsigned long      length_fetched;
	unsigned long      next_index; // where to start reading data next
	// unsigned long      fileLength; // length of a single file
	unsigned long      max_memory_consumption;    // in bytes
	unsigned long      max_trace_length_to_fetch; // max_memory_consumption / (sizeof(short) * number_of_channels)
	unsigned long      max_traces_to_fetch;

	unsigned long      number_of_points_to_write;

	double             rate_per_second;

	bool is_triggered;
	bool use_signal_generator;

	unsigned long signal_generator_peak_to_peak_in_microvolts;
	float         signal_generator_frequency;

	Channel *channels[PICOSCOPE_N_CHANNELS];
	short *data[PICOSCOPE_N_CHANNELS];
	bool data_allocated[PICOSCOPE_N_CHANNELS];
	unsigned long data_length[PICOSCOPE_N_CHANNELS];

	void SetNextIndex(unsigned long);

	// PICO_STATUS return_status;
};

#endif
