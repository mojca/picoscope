#include <windows.h>
#include <stdio.h>
#include "..\ps6000Api.h"

short _ready;
short _autoStop;
unsigned long _numSamples;
unsigned long _triggeredAt = 0;
short _triggered = FALSE;
unsigned long _startIndex;

void __stdcall StreamingCallback(
    short handle,
		unsigned long noOfSamples,
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

  return (short) ps6000RunBlock(handle, preTriggerSamples, postTriggerSamples, timebase, oversample, 
    NULL, segmentIndex, BlockCallback, NULL);
}

extern short __declspec(dllexport) __stdcall GetStreamingLatestValues(short handle)
{
  _ready = 0;
  _numSamples = 0;
  _autoStop = 0;

  return (short) ps6000GetStreamingLatestValues(handle, StreamingCallback, NULL);
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

extern PICO_STATUS __declspec(dllexport) __stdcall SetTriggerConditions(short handle, int *conditionsArray, short nConditions)
{
	PICO_STATUS status;
	short i = 0;
	short j = 0;
	PS6000_TRIGGER_CONDITIONS *conditions = (PS6000_TRIGGER_CONDITIONS *) calloc (nConditions, sizeof(PS6000_TRIGGER_CONDITIONS));

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
	status = ps6000SetTriggerChannelConditions(handle, conditions, nConditions);
	free (conditions);

	return status;
}

extern PICO_STATUS __declspec(dllexport) __stdcall SetTriggerProperties(
	short handle, 
	int *propertiesArray, 
	short nProperties, 
	short auxEnable, 
	long autoTrig)
{
	PS6000_TRIGGER_CHANNEL_PROPERTIES *channelProperties = (PS6000_TRIGGER_CHANNEL_PROPERTIES *) calloc(nProperties, sizeof(PS6000_TRIGGER_CHANNEL_PROPERTIES));
	short i;
	short j=0;
	PICO_STATUS status;
	
	for (i = 0; i < nProperties; i++)
	{
		channelProperties[i].thresholdUpper		= propertiesArray[j];
		channelProperties[i].hysteresisUpper	= propertiesArray[j + 1];
		channelProperties[i].thresholdLower		= propertiesArray[j + 2];
		channelProperties[i].hysteresisLower	= propertiesArray[j + 3];
		channelProperties[i].channel					= propertiesArray[j + 4];
		channelProperties[i].thresholdMode		= propertiesArray[j + 5];

		j=j+6;
	}
	status = ps6000SetTriggerChannelProperties(handle, channelProperties, nProperties, auxEnable, autoTrig);
	free(channelProperties);
	return status;
}