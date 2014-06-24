/****************************************************************************
 *
 * Filename:    ps4000Wrap.h
 * Copyright:   Pico Technology Limited 2008 - 2013
 * Author:      HSM
 * Description:
 *
 * This header defines the interface to the wrapper dll for the 
 *	PicoScope 4000 series of PC Oscilloscopes.
 *
 ****************************************************************************/
#ifndef __PS4000WRAP_H__
#define __PS4000WRAP_H__

#include "ps4000Api.h"

extern int16_t __declspec(dllexport) __stdcall RunBlock
(
	int16_t handle, 
	int32_t preTriggerSamples, 
	int32_t postTriggerSamples, 
    uint32_t timebase, 
	int16_t oversample, 
	uint16_t segmentIndex
);

extern int16_t __declspec(dllexport) __stdcall GetStreamingLatestValues
(
	int16_t handle
);

extern uint32_t __declspec(dllexport) __stdcall AvailableData
(
	int16_t handle, 
	uint32_t *startIndex
);

extern int16_t __declspec(dllexport) __stdcall AutoStopped
(
	int16_t handle
);

extern int16_t __declspec(dllexport) __stdcall IsReady
(
	int16_t handle
);

extern int16_t __declspec(dllexport) __stdcall IsTriggerReady
(
	int16_t handle, 
	uint32_t *triggeredAt
);

extern int16_t __declspec(dllexport) __stdcall ClearTriggerReady(void);

extern PICO_STATUS __declspec(dllexport) __stdcall SetTriggerConditions
(
	int16_t handle, 
	int32_t *conditionsArray, 
	int16_t nConditions
);

extern PICO_STATUS __declspec(dllexport) __stdcall SetTriggerProperties
(
	int16_t handle, 
	int32_t *propertiesArray, 
	int16_t nProperties,  
	int32_t autoTrig
);

extern PICO_STATUS __declspec(dllexport) __stdcall SetRapidBlockDataBuffers
(
	int16_t handle, 
	uint16_t channel, 
	int16_t *buffer, 
	uint16_t nCaptures, 
	int32_t nSamples
);

extern int16_t __declspec(dllexport) __stdcall HasOverflowed
(
	int16_t handle
);

extern void __declspec(dllexport) __stdcall setChannelCount
(
	int16_t handle, 
	int16_t channelCount
);

extern short __declspec(dllexport) __stdcall setEnabledChannels
(
	int16_t handle, 
	int16_t * enabledChannels
);

extern short __declspec(dllexport) __stdcall setAppAndDriverBuffers
(
	int16_t handle, 
	int16_t channel, 
	int16_t * appBuffer, 
	int16_t * driverBuffer, 
	int32_t bufferLength
);

extern short __declspec(dllexport) __stdcall setMaxMinAppAndDriverBuffers
(
	int16_t handle, 
	int16_t channel, 
	int16_t * appMaxBuffer, 
	int16_t * appMinBuffer, 
	int16_t * driverMaxBuffer, 
	int16_t * driverMinBuffer, 
	int32_t bufferLength
);

#endif