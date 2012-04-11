/****************************************************************************
 *
 * Filename:    ps4000Api.h
 * Copyright:   Pico Technology Limited 2002 - 2010
 * Author:      MAS
 * Description:
 *
 * This header defines the interface to driver routines for the
 *	PicoScope4000 range of PC Oscilloscopes.
 *
 ****************************************************************************/
#ifndef __PS4000API_H__
#define __PS4000API_H__

#include "picoStatus.h"

#ifdef __cplusplus
	#define PREF0 extern "C"
	#define TYPE_ENUM
#else
	#define PREF0
	#define TYPE_ENUM enum
#endif

#ifdef WIN32
	typedef unsigned __int64 uint64_t;
	#ifdef PREF1
		#undef PREF1
	#endif
	#ifdef PREF2
		#undef PREF2
	#endif
	#ifdef PREF3
		#undef PREF3
	#endif
	/*	If you are dynamically linking PS4000.DLL into your project #define DYNLINK here
	 */
	#ifdef DYNLINK
		#define PREF1 typedef
		#define PREF2
		#define PREF3(x) (__stdcall *x)
	#else
		#define PREF1
		#ifdef _USRDLL
			#define PREF2 __declspec(dllexport) __stdcall
		#else
			#define PREF2 __declspec(dllimport) __stdcall
		#endif
		#define PREF3(x) x
	#endif
	#define PREF4 __stdcall
#else
	/* Define a 64-bit integer type */
	#include <stdint.h>
	typedef int64_t __int64;

	#ifdef DYNLINK
		#define PREF1 typedef
		#define PREF2
		#define PREF3(x) (*x)
	#else
		#ifdef _USRDLL
			#define PREF1 __attribute__((visibility("default")))
		#else
			#define PREF1
		#endif
		#define PREF2
		#define PREF3(x) x
	#endif
	#define PREF4
#endif

#define PS4000_MAX_OVERSAMPLE_12BIT  16
#define PS4000_MAX_OVERSAMPLE_8BIT	256

/* Depending on the adc; oversample (collect multiple readings at each time) by up to 256.
 * the results are therefore ALWAYS scaled up to 16-bits, even if
 * oversampling is not used.
 *
 * The maximum and minimum values returned are therefore as follows:
 */

#define PS4XXX_MAX_ETS_CYCLES 400
#define PS4XXX_MAX_INTERLEAVE  80

#define PS4000_MAX_VALUE  32764
#define PS4000_MIN_VALUE -32764
#define PS4000_LOST_DATA -32768

#define PS4000_EXT_MAX_VALUE  32767
#define PS4000_EXT_MIN_VALUE -32767

#define MAX_PULSE_WIDTH_QUALIFIER_COUNT 16777215L
#define MAX_DELAY_COUNT                  8388607L

#define MIN_SIG_GEN_FREQ      0.0f
#define MAX_SIG_GEN_FREQ 100000.0f

#define MAX_SIG_GEN_BUFFER_SIZE 8192
#define MIN_SIG_GEN_BUFFER_SIZE   10
#define MIN_DWELL_COUNT           10
#define MAX_SWEEPS_SHOTS   ((1 << 30) - 1)

typedef enum enChannelBufferIndex
{
	PS4000_CHANNEL_A_MAX,
	PS4000_CHANNEL_A_MIN,
	PS4000_CHANNEL_B_MAX,
	PS4000_CHANNEL_B_MIN,
	PS4000_CHANNEL_C_MAX,
	PS4000_CHANNEL_C_MIN,
	PS4000_CHANNEL_D_MAX,
	PS4000_CHANNEL_D_MIN,
	PS4000_MAX_CHANNEL_BUFFERS
} PS4000_CHANNEL_BUFFER_INDEX;

typedef enum enPS4000Channel
{
	PS4000_CHANNEL_A,
	PS4000_CHANNEL_B,
	PS4000_CHANNEL_C,
	PS4000_CHANNEL_D,
	PS4000_EXTERNAL,
	PS4000_MAX_CHANNELS = PS4000_EXTERNAL,
	PS4000_TRIGGER_AUX,
	PS4000_MAX_TRIGGER_SOURCES
} PS4000_CHANNEL;

typedef enum enPS4000Range
{
	PS4000_10MV,
	PS4000_20MV,
	PS4000_50MV,
	PS4000_100MV,
	PS4000_200MV,
	PS4000_500MV,
	PS4000_1V,
	PS4000_2V,
	PS4000_5V,
	PS4000_10V,
	PS4000_20V,
	PS4000_50V,
	PS4000_100V,
	PS4000_MAX_RANGES,

	PS4000_RESISTANCE_100R = PS4000_MAX_RANGES,
	PS4000_RESISTANCE_1K,
	PS4000_RESISTANCE_10K,
	PS4000_RESISTANCE_100K,
	PS4000_RESISTANCE_1M,
	PS4000_MAX_RESISTANCES,

	PS4000_ACCELEROMETER_10MV = PS4000_MAX_RESISTANCES,
	PS4000_ACCELEROMETER_20MV,
	PS4000_ACCELEROMETER_50MV,
	PS4000_ACCELEROMETER_100MV,
	PS4000_ACCELEROMETER_200MV,
	PS4000_ACCELEROMETER_500MV,
	PS4000_ACCELEROMETER_1V,
	PS4000_ACCELEROMETER_2V,
	PS4000_ACCELEROMETER_5V,
	PS4000_ACCELEROMETER_10V,
	PS4000_ACCELEROMETER_20V,
	PS4000_ACCELEROMETER_50V,
	PS4000_ACCELEROMETER_100V,
	PS4000_MAX_ACCELEROMETER,

	PS4000_TEMPERATURE_UPTO_40 = PS4000_MAX_ACCELEROMETER,
	PS4000_TEMPERATURE_UPTO_70,
	PS4000_TEMPERATURE_UPTO_100,
	PS4000_TEMPERATURE_UPTO_130,
	PS4000_MAX_TEMPERATURES,

	PS4000_RESISTANCE_5K = PS4000_MAX_TEMPERATURES,
	PS4000_RESISTANCE_25K,
	PS4000_RESISTANCE_50K,
	PS4000_MAX_EXTRA_RESISTANCES
} PS4000_RANGE;

typedef enum enPS4000Probe
{
	P_NONE,
	P_CURRENT_CLAMP_10A,
	P_CURRENT_CLAMP_1000A,
	P_TEMPERATURE_SENSOR,
	P_CURRENT_MEASURING_DEVICE,
	P_PRESSURE_SENSOR_50BAR,
	P_PRESSURE_SENSOR_5BAR,
	P_OPTICAL_SWITCH,
	P_UNKNOWN,
	P_MAX_PROBES = P_UNKNOWN
} PS4000_PROBE;


typedef enum enPS4000ChannelInfo
{
	CI_RANGES,
	CI_RESISTANCES,
	CI_ACCELEROMETER,
	CI_PROBES,
	CI_TEMPERATURES
} PS4000_CHANNEL_INFO;
typedef enum enPS4000EtsMode
{
	PS4000_ETS_OFF,             // ETS disabled
	PS4000_ETS_FAST,
	PS4000_ETS_SLOW,
	PS4000_ETS_MODES_MAX
} PS4000_ETS_MODE;

typedef enum enPS4000TimeUnits
{
	PS4000_FS,
	PS4000_PS,
	PS4000_NS,
	PS4000_US,
	PS4000_MS,
	PS4000_S,
	PS4000_MAX_TIME_UNITS,
} PS4000_TIME_UNITS;

typedef enum enSweepType
{
	UP,
	DOWN,
	UPDOWN,
	DOWNUP,
	MAX_SWEEP_TYPES
} SWEEP_TYPE;

typedef enum enWaveType
{
	PS4000_SINE,
	PS4000_SQUARE,
	PS4000_TRIANGLE,
	PS4000_RAMP_UP,
	PS4000_RAMP_DOWN,
	PS4000_SINC,
	PS4000_GAUSSIAN,
	PS4000_HALF_SINE,
	PS4000_DC_VOLTAGE,
	PS4000_WHITE_NOISE,
	MAX_WAVE_TYPES
} WAVE_TYPE;

typedef enum enSigGenTrigType
{
	SIGGEN_RISING,
	SIGGEN_FALLING,
	SIGGEN_GATE_HIGH,
	SIGGEN_GATE_LOW
} SIGGEN_TRIG_TYPE;

typedef enum enSigGenTrigSource
{
	SIGGEN_NONE,
	SIGGEN_SCOPE_TRIG,
	SIGGEN_AUX_IN,
	SIGGEN_EXT_IN,
	SIGGEN_SOFT_TRIG
} SIGGEN_TRIG_SOURCE;

typedef enum enIndexMode
{
	SINGLE,
	DUAL,
	QUAD,
	MAX_INDEX_MODES
} INDEX_MODE;

typedef enum enPS4000ThresholdMode
{
	PS4000_LEVEL,
	PS4000_WINDOW
} PS4000_THRESHOLD_MODE;

typedef enum enThresholdDirection
{
	PS4000_ABOVE, //using upper threshold
	PS4000_BELOW,
	PS4000_RISING, // using upper threshold
	PS4000_FALLING, // using upper threshold
	PS4000_RISING_OR_FALLING, // using both threshold
	PS4000_ABOVE_LOWER, // using lower threshold
	PS4000_BELOW_LOWER, // using lower threshold
	PS4000_RISING_LOWER,			 // using upper threshold
	PS4000_FALLING_LOWER,		 // using upper threshold

	// Windowing using both thresholds
	PS4000_INSIDE = PS4000_ABOVE,
	PS4000_OUTSIDE = PS4000_BELOW,
	PS4000_ENTER = PS4000_RISING,
	PS4000_EXIT = PS4000_FALLING,
	PS4000_ENTER_OR_EXIT = PS4000_RISING_OR_FALLING,
	PS4000_POSITIVE_RUNT = 9,
	PS4000_NEGATIVE_RUNT,

	// no trigger set
	PS4000_NONE = PS4000_RISING
} PS4000_THRESHOLD_DIRECTION;

typedef enum enPS4000TriggerState
{
	PS4000_CONDITION_DONT_CARE,
	PS4000_CONDITION_TRUE,
	PS4000_CONDITION_FALSE,
	PS4000_CONDITION_MAX
} PS4000_TRIGGER_STATE;

#pragma pack(1)
typedef struct tPS4000TriggerConditions
{
	PS4000_TRIGGER_STATE channelA;
	PS4000_TRIGGER_STATE channelB;
	PS4000_TRIGGER_STATE channelC;
	PS4000_TRIGGER_STATE channelD;
	PS4000_TRIGGER_STATE external;
	PS4000_TRIGGER_STATE aux;
	PS4000_TRIGGER_STATE pulseWidthQualifier;
} PS4000_TRIGGER_CONDITIONS;
#pragma pack()

#pragma pack(1)
typedef struct tPS4000PwqConditions
{
	PS4000_TRIGGER_STATE channelA;
	PS4000_TRIGGER_STATE channelB;
	PS4000_TRIGGER_STATE channelC;
	PS4000_TRIGGER_STATE channelD;
	PS4000_TRIGGER_STATE external;
	PS4000_TRIGGER_STATE aux;
} PWQ_CONDITIONS;
#pragma pack()

#pragma pack(1)
typedef struct tPS4000TriggerChannelProperties
{
	short thresholdUpper;
	unsigned short thresholdUpperHysteresis;
	short thresholdLower;
	unsigned short thresholdLowerHysteresis;
	PS4000_CHANNEL channel;
	PS4000_THRESHOLD_MODE thresholdMode;
} PS4000_TRIGGER_CHANNEL_PROPERTIES;
#pragma pack()

typedef enum enPS4000RatioMode
{
	PS4000_RATIO_MODE_NONE,
	PS4000_RATIO_MODE_AGGREGATE = 1,
	PS4000_RATIO_MODE_AVERAGE = 2
} PS4000_RATIO_MODE;

#ifndef __PSX000API_enPulseWidthType_H__
#define __PSX000API_enPulseWidthType_H__

typedef enum enPulseWidthType
{
	PW_TYPE_NONE,
	PW_TYPE_LESS_THAN,
	PW_TYPE_GREATER_THAN,
	PW_TYPE_IN_RANGE,
	PW_TYPE_OUT_OF_RANGE
} PULSE_WIDTH_TYPE;

#endif

typedef enum enPs4000HoldOffType
{
	PS4000_TIME,
	PS4000_MAX_HOLDOFF_TYPE
} PS4000_HOLDOFF_TYPE;

typedef enum enPS4000FrequencyCounterRange
{
	FC_2K,
	FC_20K,
	FC_MAX
} PS4000_FREQUENCY_COUNTER_RANGE;

typedef void (PREF4 *ps4000BlockReady)
	(
		short                     handle,
		PICO_STATUS               status,
		void                    * pParameter
	);

typedef void (PREF4 *ps4000StreamingReady)
	(
		short                     handle,
		long                      noOfSamples,
		unsigned long             startIndex,
		short                     overflow,
		unsigned long             triggerAt,
		short                     triggered,
		short                     autoStop,
		void                    * pParameter
	);

typedef void (PREF4 *ps4000DataReady)
	(
		short                     handle,
		long                      noOfSamples,
		short                     overflow,
		unsigned long             triggerAt,
		short                     triggered,
		void                    * pParameter
	);

PREF0 PREF1 PICO_STATUS PREF2 PREF3 (ps4000OpenUnit)
	(
		short                     * handle
	);


PREF0 PREF1 PICO_STATUS PREF2 PREF3 (ps4000OpenUnitAsync)
	(
		short                     * status
	);

PREF0 PREF1 PICO_STATUS PREF2 PREF3 (ps4000OpenUnitEx)
	(
		short                     * handle,
		char                      * serial
	);

PREF0 PREF1 PICO_STATUS PREF2 PREF3 (ps4000OpenUnitAsyncEx)
	(
		short                     * status,
		char                      * serial
	);


PREF0 PREF1 PICO_STATUS PREF2 PREF3 (ps4000OpenUnitProgress)
	(
		short                     * handle,
		short                     * progressPercent,
		short                     * complete
	);

PREF0 PREF1 PICO_STATUS PREF2 PREF3 (ps4000GetUnitInfo)
	(
		short                       handle,
		char                      * string,
		short                       stringLength,
		short                     * requiredSize,
		PICO_INFO                   info
	);

PREF0 PREF1 PICO_STATUS PREF2 PREF3 (ps4000FlashLed)
	(
		short                       handle,
		short                       start
	);

PREF0 PREF1 PICO_STATUS PREF2 PREF3 (ps4000IsLedFlashing)
	(
		short                       handle,
		short                     * status
	);

PREF0 PREF1 PICO_STATUS PREF2 PREF3 (ps4000CloseUnit)
	(
		short                       handle
	);

PREF0 PREF1 PICO_STATUS PREF2 PREF3 (ps4000MemorySegments)
	(
		short                       handle,
		unsigned short              nSegments,
		unsigned long             * nMaxSamples
	);

PREF0 PREF1 PICO_STATUS PREF2 PREF3 (ps4000SetChannel)
	(
		short                       handle,
		PS4000_CHANNEL              channel,
		short                       enabled,
		short                       dc,
		PS4000_RANGE                range
	);


PREF0 PREF1 PICO_STATUS PREF2 PREF3 (ps4000SetNoOfCaptures)
	(
		short handle,
		unsigned short nCaptures
	);

PREF0 PREF1 PICO_STATUS PREF2 PREF3 (ps4000GetTimebase)
	(
		short                      handle,
		unsigned long              timebase,
		long                       noSamples,
		long                     * timeIntervalNanoseconds,
		short                      oversample,
		long                     * maxSamples,
		unsigned short             segmentIndex
	);

PREF0 PREF1 PICO_STATUS PREF2 PREF3 (ps4000GetTimebase2)
	(
		short                      handle,
		unsigned long              timebase,
		long                       noSamples,
		float                    * timeIntervalNanoseconds,
		short                      oversample,
		long                     * maxSamples,
		unsigned short             segmentIndex
	);

PREF0 PREF1 PICO_STATUS PREF2 PREF3 (ps4000SetSigGenArbitrary)
	(
	 	short                       handle,
	 	long                        offsetVoltage,
	 	unsigned long               pkToPk,
	 	unsigned long               startDeltaPhase,
	 	unsigned long               stopDeltaPhase,
	 	unsigned long               deltaPhaseIncrement,
	 	unsigned long               dwellCount,
	 	short                     * arbitraryWaveform,
	 	long                        arbitraryWaveformSize,
		SWEEP_TYPE                  sweepType,
		short                       whiteNoise,
		INDEX_MODE                  indexMode,
		unsigned long               shots,
		unsigned long               sweeps,
		SIGGEN_TRIG_TYPE            triggerType,
		SIGGEN_TRIG_SOURCE          triggerSource,
		short                       extInThreshold
	);

PREF0 PREF1 PICO_STATUS PREF2 PREF3(ps4000SetSigGenBuiltIn)
	(
		short                       handle,
		long                        offsetVoltage,
		unsigned long               pkToPk,
		short                       waveType,
		float                       startFrequency,
		float                       stopFrequency,
		float                       increment,
		float                       dwellTime,
		SWEEP_TYPE                  sweepType,
		short                       whiteNoise,
		unsigned long               shots,
		unsigned long               sweeps,
		SIGGEN_TRIG_TYPE            triggerType,
		SIGGEN_TRIG_SOURCE          triggerSource,
		short                       extInThreshold
	);

PREF0 PREF1 PICO_STATUS PREF2 PREF3 (ps4000SigGenSoftwareControl)
	(
		short                       handle,
		short                       state
	);

PREF0 PREF1 PICO_STATUS PREF2 PREF3 (ps4000SetEts)
	(
		short                       handle,
		PS4000_ETS_MODE             mode,
		short                       etsCycles,
		short                       etsInterleave,
		long                      * sampleTimePicoseconds
	);


PREF0 PREF1 PICO_STATUS PREF2 PREF3 (ps4000SetSimpleTrigger)
	(
		short handle,
		short enable,
		PS4000_CHANNEL source,
		short threshold,
		PS4000_THRESHOLD_DIRECTION direction,
		unsigned long delay,
		short autoTrigger_ms
	);

PREF0 PREF1 PICO_STATUS PREF2 PREF3 (ps4000SetTriggerChannelProperties)
	(
		short                               handle,
		PS4000_TRIGGER_CHANNEL_PROPERTIES * channelProperties,
		short                               nChannelProperties,
		short                               auxOutputEnable,
		long                                autoTriggerMilliseconds
	);


PREF0 PREF1 PICO_STATUS PREF2 PREF3 (ps4000SetTriggerChannelConditions)
	(
		short                       handle,
		PS4000_TRIGGER_CONDITIONS * conditions,
		short                       nConditions
	);

PREF0 PREF1 PICO_STATUS PREF2 PREF3 (ps4000SetTriggerChannelDirections)
	(
		short                        handle,
		PS4000_THRESHOLD_DIRECTION   channelA,
		PS4000_THRESHOLD_DIRECTION   channelB,
		PS4000_THRESHOLD_DIRECTION   channelC,
		PS4000_THRESHOLD_DIRECTION   channelD,
		PS4000_THRESHOLD_DIRECTION   ext,
		PS4000_THRESHOLD_DIRECTION   aux
	);

PREF0 PREF1 PICO_STATUS PREF2 PREF3 (ps4000SetTriggerDelay)
	(
		short                 handle,
		unsigned long         delay
	);

PREF0 PREF1 PICO_STATUS PREF2 PREF3 (ps4000SetPulseWidthQualifier)
	(
		short                      handle,
		PWQ_CONDITIONS           * conditions,
		short                      nConditions,
		PS4000_THRESHOLD_DIRECTION direction,
		unsigned long              lower,
		unsigned long              upper,
		PULSE_WIDTH_TYPE           type
	);

PREF0 PREF1 PICO_STATUS PREF2 PREF3 (ps4000IsTriggerOrPulseWidthQualifierEnabled)
	(
		short                 handle,
		short               * triggerEnabled,
		short               * pulseWidthQualifierEnabled
	);

PREF0 PREF1 PICO_STATUS PREF2 PREF3 (ps4000GetTriggerTimeOffset)
	(
		short                 handle,
		unsigned long       * timeUpper,
		unsigned long       * timeLower,
		PS4000_TIME_UNITS   * timeUnits,
		unsigned short        segmentIndex
	);

PREF0 PREF1 PICO_STATUS PREF2 PREF3 (ps4000GetTriggerChannelTimeOffset)
	(
		short                 handle,
		unsigned long       * timeUpper,
		unsigned long       * timeLower,
		PS4000_TIME_UNITS   * timeUnits,
		unsigned short        segmentIndex,
		PS4000_CHANNEL        channel
	);


PREF0 PREF1 PICO_STATUS PREF2 PREF3 (ps4000GetTriggerTimeOffset64)
	(
		short                 handle,
		__int64             * time,
		PS4000_TIME_UNITS   * timeUnits,
		unsigned short        segmentIndex
	);

PREF0 PREF1 PICO_STATUS PREF2 PREF3 (ps4000GetTriggerChannelTimeOffset64)
	(
		short                 handle,
		__int64             * time,
		PS4000_TIME_UNITS   * timeUnits,
		unsigned short        segmentIndex,
		PS4000_CHANNEL        channel
	);


PREF0 PREF1 PICO_STATUS PREF2 PREF3 (ps4000GetValuesTriggerTimeOffsetBulk)
	(
		short               handle,
		unsigned long     * timesUpper,
		unsigned long     * timesLower,
		PS4000_TIME_UNITS * timeUnits,
		unsigned short      fromSegmentIndex,
		unsigned short      toSegmentIndex
	);

PREF0 PREF1 PICO_STATUS PREF2 PREF3 (ps4000GetValuesTriggerChannelTimeOffsetBulk)
	(
		short               handle,
		unsigned long     * timesUpper,
		unsigned long     * timesLower,
		PS4000_TIME_UNITS * timeUnits,
		unsigned short      fromSegmentIndex,
		unsigned short      toSegmentIndex,
		PS4000_CHANNEL      channel
	);


PREF0 PREF1 PICO_STATUS PREF2 PREF3 (ps4000GetValuesTriggerTimeOffsetBulk64)
	(
		short               handle,
		__int64           * times,
		PS4000_TIME_UNITS * timeUnits,
		unsigned short      fromSegmentIndex,
		unsigned short      toSegmentIndex
	);

PREF0 PREF1 PICO_STATUS PREF2 PREF3 (ps4000GetValuesTriggerChannelTimeOffsetBulk64)
	(
		short               handle,
		__int64           * times,
		PS4000_TIME_UNITS * timeUnits,
		unsigned short      fromSegmentIndex,
		unsigned short      toSegmentIndex,
		PS4000_CHANNEL      channel
	);

PREF0 PREF1 PICO_STATUS PREF2 PREF3 (ps4000SetDataBufferBulk)
	(
		short                 handle,
		PS4000_CHANNEL        channel,
		short              *  buffer,
		long                  bufferLth,
		unsigned short        waveform
	);


PREF0 PREF1 PICO_STATUS PREF2 PREF3 (ps4000SetDataBuffers)
	(
		short                  handle,
		PS4000_CHANNEL         channel,
		short                * bufferMax,
		short                * bufferMin,
		long                   bufferLth
	);

PREF0 PREF1 PICO_STATUS PREF2 PREF3 (ps4000SetDataBufferWithMode)
	(
		short                  handle,
		PS4000_CHANNEL         channel,
		short                * buffer,
		long                   bufferLth,
		PS4000_RATIO_MODE      mode
	);

PREF0 PREF1 PICO_STATUS PREF2 PREF3 (ps4000SetDataBuffersWithMode)
	(
		short                  handle,
		PS4000_CHANNEL         channel,
		short                * bufferMax,
		short                * bufferMin,
		long                   bufferLth,
		PS4000_RATIO_MODE      mode
	);

PREF0 PREF1 PICO_STATUS PREF2 PREF3 (ps4000SetDataBuffer)
	(
		short                  handle,
		PS4000_CHANNEL         channel,
		short                * buffer,
		long                   bufferLth
	);

PREF0 PREF1 PICO_STATUS PREF2 PREF3 (ps4000SetEtsTimeBuffer)
	(
		short                  handle,
		__int64 *              buffer,
		long                   bufferLth
	);

PREF0 PREF1 PICO_STATUS PREF2 PREF3 (ps4000SetEtsTimeBuffers)
	(
		short                  handle,
		unsigned long        * timeUpper,
		unsigned long        * timeLower,
		long                   bufferLth
	);

PREF0 PREF1 PICO_STATUS PREF2 PREF3 (ps4000RunBlock)
	(
		short                 handle,
		long                  noOfPreTriggerSamples,
		long                  noOfPostTriggerSamples,
		unsigned long         timebase,
		short                 oversample,
		long                * timeIndisposedMs,
		unsigned short        segmentIndex,
		ps4000BlockReady      lpReady,
		void                * pParameter
	);


PREF0 PREF1 PICO_STATUS PREF2 PREF3 (ps4000RunStreaming)
	(
		short                 handle,
		unsigned long       * sampleInterval,
		PS4000_TIME_UNITS     sampleIntervalTimeUnits,
		unsigned long         maxPreTriggerSamples,
		unsigned long         maxPostPreTriggerSamples,
		short                 autoStop,
		unsigned long         downSampleRatio,
		unsigned long         overviewBufferSize
	);

PREF0 PREF1 PICO_STATUS PREF2 PREF3 (ps4000RunStreamingEx)
	(
		short                 handle,
		unsigned long       * sampleInterval,
		PS4000_TIME_UNITS     sampleIntervalTimeUnits,
		unsigned long         maxPreTriggerSamples,
		unsigned long         maxPostPreTriggerSamples,
		short                 autoStop,
		unsigned long         downSampleRatio,
		short                 downSampleRatioMode,
		unsigned long         overviewBufferSize
	);

PREF0 PREF1 PICO_STATUS PREF2 PREF3 (ps4000IsReady)
	(
		short handle,
		short * ready
	);

PREF0 PREF1 PICO_STATUS PREF2 PREF3 (ps4000GetStreamingLatestValues)
	(
		short                 handle,
		ps4000StreamingReady  lpPs4000Ready,
		void                * pParameter
		);

PREF0 PREF1 PICO_STATUS PREF2 PREF3 (ps4000NoOfStreamingValues)
	(
		short               handle,
		unsigned long     * noOfValues
	);

PREF0 PREF1 PICO_STATUS PREF2 PREF3 (ps4000GetMaxDownSampleRatio)
	(
		short               handle,
		unsigned long       noOfUnaggreatedSamples,
		unsigned long     * maxDownSampleRatio,
		short               downSampleRatioMode,
		unsigned short      segmentIndex
	);

PREF0 PREF1 PICO_STATUS PREF2 PREF3 (ps4000GetValues)
	(
		short               handle,
		unsigned long       startIndex,
		unsigned long     * noOfSamples,
		unsigned long       downSampleRatio,
		short               downSampleRatioMode,
		unsigned short      segmentIndex,
		short             * overflow
	);

PREF0 PREF1 PICO_STATUS PREF2 PREF3 (ps4000GetValuesBulk)
	(
		short               handle,
		unsigned long     * noOfSamples,
		unsigned short      fromSegmentIndex,
		unsigned short      toSegmentIndex,
		short             * overflow
	);

PREF0 PREF1 PICO_STATUS PREF2 PREF3 (ps4000GetValuesAsync)
	(
		short               handle,
		unsigned long       startIndex,
		unsigned long       noOfSamples,
		unsigned long       downSampleRatio,
		short               downSampleRatioMode,
		unsigned short      segmentIndex,
		void              * lpDataReady,
		void              * pParameter
	);

PREF0 PREF1 PICO_STATUS PREF2 PREF3 (ps4000Stop)
	(
		short handle
	);

PREF0 PREF1 PICO_STATUS PREF2 PREF3 (ps4000SetProbe)
	(
		short         handle,
		PS4000_PROBE  probe,
		PS4000_RANGE  range
	);

PREF0 PREF1 PICO_STATUS PREF2 PREF3 (ps4000HoldOff)
	(
		short                          handle,
		uint64_t                       holdoff,
		PS4000_HOLDOFF_TYPE            type
	);

PREF0 PREF1 PICO_STATUS PREF2 PREF3 (ps4000GetProbe)
	(
		short                          handle,
		PS4000_PROBE                 * probe
	);

PREF0 PREF1 PICO_STATUS PREF2 PREF3 (ps4000GetChannelInformation)
	(
		short                          handle,
		PS4000_CHANNEL_INFO            info,
		int                            probe,
		int                          * ranges,
		int                          * length,
		int                            channels
	);

PREF0 PREF1 PICO_STATUS PREF2 PREF3 (ps4000SetFrequencyCounter)
	(
		short                          handle,
		PS4000_CHANNEL                 channel,
		short                          enabled,
		PS4000_FREQUENCY_COUNTER_RANGE range,
		short                          thresholdMajor,
		short                          thresholdMinor
	);

PREF0 PREF1 PICO_STATUS PREF2 PREF3 (ps4000EnumerateUnits)
	(
		short                        * count,
		char                         * serials,
		short                        * serialLth
	);

PREF0 PREF1 PICO_STATUS PREF2 PREF3 (ps4000PingUnit)
	(
		short handle
	);

#endif
