// ps4000wrap.cpp : Defines the entry point for the DLL application.
//
#include <windows.h>
#include <stdio.h>
#include "ps4000Api.h"

short _ready;
short _autoStop;
unsigned long _numSamples;
unsigned long _triggeredAt = 0;
short _triggered = FALSE;
unsigned long _startIndex = 0;

void __stdcall StreamingCallback(
    short handle,
		long noOfSamples,
		unsigned long startIndex,
		short overflow,
		unsigned long triggerAt,
		short triggered,
		short autoStop,
		void * pParameter)
{
  _numSamples = noOfSamples;
  _autoStop = autoStop;
	_startIndex = startIndex;

	_triggered = triggered;
	_triggeredAt = triggerAt;
  
  _ready = 1;
}

void __stdcall BlockCallback(short handle, PICO_STATUS status, void * pParameter)
{
  _ready = 1;
}

extern short __declspec(dllexport) __stdcall RunBlock(short handle, long preTriggerSamples, long postTriggerSamples, 
            unsigned long timebase, short oversample, short segmentIndex)
{
  _ready = 0;
  _numSamples = preTriggerSamples + postTriggerSamples;

  return (short) ps4000RunBlock(handle, preTriggerSamples, postTriggerSamples, timebase, oversample, 
    NULL, segmentIndex, BlockCallback, NULL);
}

extern short __declspec(dllexport) __stdcall GetStreamingLatestValues(short handle)
{
  _ready = 0;
  _numSamples = 0;
  _autoStop = 0;

  return (short) ps4000GetStreamingLatestValues(handle, StreamingCallback, NULL);
}

extern unsigned long __declspec(dllexport) __stdcall AvailableData(short handle, unsigned long *startIndex)
{
	if( _ready )
	{
		*startIndex = _startIndex;
			
		return _numSamples;
	}
  return 0l;
}

extern short __declspec(dllexport) __stdcall AutoStopped(short handle)
{
  if( _ready) return _autoStop;
  return 0;
}

extern short __declspec(dllexport) __stdcall IsReady(short handle)
{
  return _ready;
}

extern short __declspec(dllexport) __stdcall IsTriggerReady(short handle, unsigned long *triggeredAt)
{
	if (_triggered)
		*triggeredAt = _triggeredAt;

  return _triggered;
}

extern short __declspec(dllexport) __stdcall ClearTriggerReady(void)
{
	_triggeredAt = 0;
	_triggered = FALSE;
	return 1;
}

extern short __declspec(dllexport) __stdcall SetTriggerConditions(short handle, int *conditionsArray, short nConditions)
{
	PICO_STATUS ok;
	short i = 0;
	short j = 0;
	TRIGGER_CONDITIONS *conditions = (TRIGGER_CONDITIONS *) calloc (nConditions, sizeof(TRIGGER_CONDITIONS));

	for (i = 0; i < nConditions; i++)
	{
		conditions[i].channelA 						= conditionsArray[j];
		conditions[i].channelB 						= conditionsArray[j + 1];
		conditions[i].channelC 						= conditionsArray[j + 2];
		conditions[i].channelD 						= conditionsArray[j + 3];
		conditions[i].external 						= conditionsArray[j + 4];
		conditions[i].pulseWidthQualifier = conditionsArray[j + 5];
		conditions[i].aux									= conditionsArray[j + 6];

		j = j + 7;
	}
	ok = ps4000SetTriggerChannelConditions(handle, conditions, nConditions);
	free (conditions);

	return ok;
}

extern short __declspec(dllexport) __stdcall SetTriggerProperties(
	short handle, 
	int *propertiesArray, 
	short nProperties, 
	short auxEnable, 
	long autoTrig)
{
	TRIGGER_CHANNEL_PROPERTIES *channelProperties = (TRIGGER_CHANNEL_PROPERTIES *) calloc(nProperties, sizeof(TRIGGER_CHANNEL_PROPERTIES));
	short i;
	short j=0;
	PICO_STATUS ok;
	
	for (i = 0; i < nProperties; i++)
	{
		channelProperties[i].thresholdUpper							 = propertiesArray[j];
		channelProperties[i].thresholdUpperHysteresis		 = propertiesArray[j + 1];
		channelProperties[i].thresholdLower							 = propertiesArray[j + 2];
		channelProperties[i].thresholdLowerHysteresis		 = propertiesArray[j + 3];
		channelProperties[i].channel										 = propertiesArray[j + 4];
		channelProperties[i].thresholdMode							 = propertiesArray[j + 5];

		j=j+6;
	}
	ok = ps4000SetTriggerChannelProperties(handle, channelProperties, nProperties, auxEnable, autoTrig);
	free(channelProperties);
	return ok;
}

extern short __declspec(dllexport) __stdcall SetRapidBlockDataBuffers(short handle, short channel, short *buffer, short nCaptures, unsigned long nSamples)
{
	int capture;
	short status = 0;
	short *currentBufferPtr;

	for(capture = 0; capture < nCaptures && status == 0; capture++)
	{
		currentBufferPtr = buffer + (capture * nSamples);
		
		status = (short)ps4000SetDataBufferBulk(handle, (PS4000_CHANNEL)channel, currentBufferPtr, nSamples, capture);
	}
	return status;
}
