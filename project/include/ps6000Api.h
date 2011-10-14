/****************************************************************************
 *
 * Filename:    ps6000Api.h
 * Copyright:   Pico Technology Limited 2002 - 2009
 * Author:      MAS
 * Description:
 *
 * This header defines the interface to driver routines for the
 *	PicoScope6000 range of PC Oscilloscopes.
 *
 ****************************************************************************/
#ifndef __PS6000API_H__
#define __PS6000API_H__

#include "picoStatus.h"

#ifdef PREF0
	#undef PREF0
#endif
#ifdef PREF1
	#undef PREF1
#endif
#ifdef PREF2
	#undef PREF2
#endif
#ifdef PREF3
	#undef PREF3
#endif

#ifdef __cplusplus
	#define PREF0 extern "C"
#else
	#define PREF0
#endif

#ifdef WIN32
/*	If you are dynamically linking PS6000.DLL into your project #define DYNLINK here
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

#define PS6000_MAX_OVERSAMPLE_8BIT 256

/* Although the PS6000 uses an 8-bit ADC, it is usually possible to
 * oversample (collect multiple readings at each time) by up to 256.
 * the results are therefore ALWAYS scaled up to 16-bits, even if
 * oversampling is not used.
 *
 * The maximum and minimum values returned are therefore as follows:
 */
#define PS6000_MAX_VALUE  32512
#define PS6000_MIN_VALUE -32512

#define MAX_PULSE_WIDTH_QUALIFIER_COUNT 16777215L

//#define MAX_SIG_GEN_BUFFER_SIZE   16384
#define MIN_SIG_GEN_BUFFER_SIZE      10
#define MIN_DWELL_COUNT              10
#define MAX_SWEEPS_SHOTS ((1 << 30) - 1)

#define MAX_WAVEFORMS_PER_SECOND 1000000

#define MAX_ANALOGUE_OFFSET_50MV_200MV  0.500f
#define MIN_ANALOGUE_OFFSET_50MV_200MV -0.500f
#define MAX_ANALOGUE_OFFSET_500MV_2V    2.500f
#define MIN_ANALOGUE_OFFSET_500MV_2V   -2.500f
#define MAX_ANALOGUE_OFFSET_5V_20V     20.f
#define MIN_ANALOGUE_OFFSET_5V_20V    -20.f


#define PS6000_MAX_ETS_CYCLES 250
#define PS6000_MAX_INTERLEAVE  50

typedef enum enPS6000ExternalFrequency
{
	PS6000_FREQUENCY_OFF,
	PS6000_FREQUENCY_5MHZ,
	PS6000_FREQUENCY_10MHZ,
	PS6000_FREQUENCY_20MHZ,
	PS6000_FREQUENCY_25MHZ,
	PS6000_MAX_FREQUENCIES
} PS6000_EXTERNAL_FREQUENCY;

typedef enum enPS6000BandwidthLimiter
{
	PS6000_BW_FULL,
	PS6000_BW_20MHZ,
	PS6000_BW_25MHZ
} PS6000_BANDWIDTH_LIMITER;

typedef enum enPS6000Channel
{
	PS6000_CHANNEL_A,
	PS6000_CHANNEL_B,
	PS6000_CHANNEL_C,
	PS6000_CHANNEL_D,
	PS6000_EXTERNAL,
	PS6000_MAX_CHANNELS = PS6000_EXTERNAL,
	PS6000_TRIGGER_AUX,
	PS6000_MAX_TRIGGER_SOURCES
} PS6000_CHANNEL;

typedef enum enPS6000ChannelBufferIndex
{
	PS6000_CHANNEL_A_MAX,
	PS6000_CHANNEL_A_MIN,
	PS6000_CHANNEL_B_MAX,
	PS6000_CHANNEL_B_MIN,
	PS6000_CHANNEL_C_MAX,
	PS6000_CHANNEL_C_MIN,
	PS6000_CHANNEL_D_MAX,
	PS6000_CHANNEL_D_MIN,
	PS6000_MAX_CHANNEL_BUFFERS
} PS6000_CHANNEL_BUFFER_INDEX;

typedef enum enPS6000Range
{
	PS6000_10MV,
	PS6000_20MV,
	PS6000_50MV,
	PS6000_100MV,
	PS6000_200MV,
	PS6000_500MV,
	PS6000_1V,
	PS6000_2V,
	PS6000_5V,
	PS6000_10V,
	PS6000_20V,
	PS6000_50V,
	PS6000_MAX_RANGES
} PS6000_RANGE;


typedef enum enPS6000Coupling
{
	PS6000_AC,
	PS6000_DC_1M,
	PS6000_DC_50R
} PS6000_COUPLING;

typedef enum enPS6000EtsMode
{
	PS6000_ETS_OFF,             // ETS disabled
	PS6000_ETS_FAST,            // Return ready as soon as requested no of interleaves is available
	PS6000_ETS_SLOW,            // Return ready every time a new set of no_of_cycles is collected
	PS6000_ETS_MODES_MAX
} PS6000_ETS_MODE;

typedef enum enPS6000TimeUnits
{
	PS6000_FS,
	PS6000_PS,
	PS6000_NS,
	PS6000_US,
	PS6000_MS,
	PS6000_S,
	PS6000_MAX_TIME_UNITS,
} PS6000_TIME_UNITS;

typedef enum enPS6000SweepType
{
	PS6000_UP,
	PS6000_DOWN,
	PS6000_UPDOWN,
	PS6000_DOWNUP,
	PS6000_MAX_SWEEP_TYPES
} PS6000_SWEEP_TYPE;

typedef enum enPS6000WaveType
{
	PS6000_SINE,
	PS6000_SQUARE,
	PS6000_TRIANGLE,
	PS6000_RAMP_UP,
	PS6000_RAMP_DOWN,
	PS6000_SINC,
	PS6000_GAUSSIAN,
	PS6000_HALF_SINE,
	PS6000_DC_VOLTAGE,
	PS6000_MAX_WAVE_TYPES
} PS6000_WAVE_TYPE;

typedef enum enPS6000ExtraOperations
{
	PS6000_ES_OFF,
	PS6000_WHITENOISE,
	PS6000_PRBS // Pseudo-Random Bit Stream
} PS6000_EXTRA_OPERATIONS;

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

#define PS6000_PRBS_MAX_FREQUENCY      20000000.f
#define PS6000_SINE_MAX_FREQUENCY      20000000.f
#define PS6000_SQUARE_MAX_FREQUENCY    20000000.f
#define PS6000_TRIANGLE_MAX_FREQUENCY  20000000.f
#define PS6000_SINC_MAX_FREQUENCY      20000000.f
#define PS6000_RAMP_MAX_FREQUENCY      20000000.f
#define PS6000_HALF_SINE_MAX_FREQUENCY 20000000.f
#define PS6000_GAUSSIAN_MAX_FREQUENCY  20000000.f
#define PS6000_MIN_FREQUENCY                0.03f

typedef enum enPS6000SigGenTrigType
{
	PS6000_SIGGEN_RISING,
	PS6000_SIGGEN_FALLING,
	PS6000_SIGGEN_GATE_HIGH,
	PS6000_SIGGEN_GATE_LOW
} PS6000_SIGGEN_TRIG_TYPE;

typedef enum enPS6000SigGenTrigSource
{
	PS6000_SIGGEN_NONE,
	PS6000_SIGGEN_SCOPE_TRIG,
	PS6000_SIGGEN_AUX_IN,
	PS6000_SIGGEN_EXT_IN,
	PS6000_SIGGEN_SOFT_TRIG,
	PS6000_SIGGEN_TRIGGER_RAW
} PS6000_SIGGEN_TRIG_SOURCE;

typedef enum enPS6000IndexMode
{
	PS6000_SINGLE,
	PS6000_DUAL,
	PS6000_QUAD,
	PS6000_MAX_INDEX_MODES
} PS6000_INDEX_MODE;

typedef enum enPS6000ThresholdMode
{
	PS6000_LEVEL,
	PS6000_WINDOW
} PS6000_THRESHOLD_MODE;

typedef enum enPS6000ThresholdDirection
{
	PS6000_ABOVE,             // using upper threshold
	PS6000_BELOW,
	PS6000_RISING,            // using upper threshold
	PS6000_FALLING,           // using upper threshold
	PS6000_RISING_OR_FALLING, // using both  threshold
	PS6000_ABOVE_LOWER,       // using lower threshold
	PS6000_BELOW_LOWER,       // using lower threshold
	PS6000_RISING_LOWER,      // using upper threshold
	PS6000_FALLING_LOWER,     // using upper threshold

	// Windowing using both thresholds
	PS6000_INSIDE = PS6000_ABOVE,
	PS6000_OUTSIDE = PS6000_BELOW,
	PS6000_ENTER = PS6000_RISING,
	PS6000_EXIT = PS6000_FALLING,
	PS6000_ENTER_OR_EXIT = PS6000_RISING_OR_FALLING,
	PS6000_POSITIVE_RUNT = 9,
	PS6000_NEGATIVE_RUNT,

	// no trigger set
	PS6000_NONE = PS6000_RISING
} PS6000_THRESHOLD_DIRECTION;

typedef enum enPS6000TriggerState
{
	PS6000_CONDITION_DONT_CARE,
	PS6000_CONDITION_TRUE,
	PS6000_CONDITION_FALSE,
	PS6000_CONDITION_MAX
} PS6000_TRIGGER_STATE;

#pragma pack(1)
typedef struct tPS6000TriggerConditions
{
	PS6000_TRIGGER_STATE channelA;
	PS6000_TRIGGER_STATE channelB;
	PS6000_TRIGGER_STATE channelC;
	PS6000_TRIGGER_STATE channelD;
	PS6000_TRIGGER_STATE external;
	PS6000_TRIGGER_STATE aux;
	PS6000_TRIGGER_STATE pulseWidthQualifier;
} PS6000_TRIGGER_CONDITIONS;
#pragma pack()

#pragma pack(1)
typedef struct tPS6000PwqConditions
{
	PS6000_TRIGGER_STATE channelA;
	PS6000_TRIGGER_STATE channelB;
	PS6000_TRIGGER_STATE channelC;
	PS6000_TRIGGER_STATE channelD;
	PS6000_TRIGGER_STATE external;
	PS6000_TRIGGER_STATE aux;
} PS6000_PWQ_CONDITIONS;
#pragma pack()


#pragma pack(1)
typedef struct tPS6000TriggerChannelProperties
{
	short          thresholdUpper;
	unsigned short hysteresisUpper;
	short          thresholdLower;
	unsigned short hysteresisLower;

	PS6000_CHANNEL        channel;
	PS6000_THRESHOLD_MODE thresholdMode;
} PS6000_TRIGGER_CHANNEL_PROPERTIES;
#pragma pack()

typedef enum enPS6000RatioMode
{
	PS6000_RATIO_MODE_NONE,
	PS6000_RATIO_MODE_AGGREGATE = 1,
	PS6000_RATIO_MODE_AVERAGE = 2,
	PS6000_RATIO_MODE_DECIMATE = 4,
	PS6000_RATIO_MODE_DISTRIBUTION = 8
} PS6000_RATIO_MODE;

typedef enum enPS6000PulseWidthType
{
	PS6000_PW_TYPE_NONE,
	PS6000_PW_TYPE_LESS_THAN,
	PS6000_PW_TYPE_GREATER_THAN,
	PS6000_PW_TYPE_IN_RANGE,
	PS6000_PW_TYPE_OUT_OF_RANGE
} PS6000_PULSE_WIDTH_TYPE;

typedef void (PREF4 *ps6000BlockReady)
	(
		short               handle,
		PICO_STATUS         status,
		void              * pParameter
	);

typedef void (PREF4 *ps6000StreamingReady)
	(
		short               handle,
		unsigned long       noOfSamples,
		unsigned long       startIndex,
		short               overflow,
		unsigned long       triggerAt,
		short               triggered,
		short               autoStop,
		void              * pParameter
	);

typedef void (PREF4 *ps6000DataReady)
	(
		short               handle,
		PICO_STATUS         status,
		unsigned long       noOfSamples,
		short               overflow,
		void              * pParameter
	);

PREF0 PREF1 PICO_STATUS PREF2 PREF3 (ps6000OpenUnit)
	(
		short             * handle,
		char              * serial
	);

PREF0 PREF1 PICO_STATUS PREF2 PREF3 (ps6000OpenUnitAsync)
	(
		short                     * status,
		char                      * serial
	);

PREF0 PREF1 PICO_STATUS PREF2 PREF3 (ps6000OpenUnitProgress)
	(
		short                     * handle,
		short                     * progressPercent,
		short                     * complete
	);

PREF0 PREF1 PICO_STATUS PREF2 PREF3 (ps6000GetUnitInfo)
	(
		short                       handle,
		char                      * string,
		short                       stringLength,
		short                     * requiredSize,
		PICO_INFO                   info
	);

PREF0 PREF1 PICO_STATUS PREF2 PREF3 (ps6000FlashLed)
	(
		short                       handle,
		short                       start
	);

PREF0 PREF1 PICO_STATUS PREF2 PREF3 (ps6000CloseUnit)
	(
		short                       handle
	);

PREF0 PREF1 PICO_STATUS PREF2 PREF3 (ps6000MemorySegments)
	(
		short                       handle,
		unsigned long               nSegments,
		unsigned long             * nMaxSamples
	);

PREF0 PREF1 PICO_STATUS PREF2 PREF3 (ps6000SetChannel)
	(
		short                       handle,
		PS6000_CHANNEL              channel,
		short                       enabled,
		PS6000_COUPLING             type,
		PS6000_RANGE                range,
		float                       analogueOffset,
		PS6000_BANDWIDTH_LIMITER    bandwidth
	);


PREF0 PREF1 PICO_STATUS PREF2 PREF3 (ps6000GetTimebase)
	(
		short                     handle,
		unsigned long             timebase,
		unsigned long             noSamples,
		long                    * timeIntervalNanoseconds,
		short                     oversample,
		unsigned long           * maxSamples,
		unsigned long             segmentIndex
	);

PREF0 PREF1 PICO_STATUS PREF2 PREF3 (ps6000GetTimebase2)
	(
		short                     handle,
		unsigned long             timebase,
		unsigned long             noSamples,
		float                   * timeIntervalNanoseconds,
		short                     oversample,
		unsigned long           * maxSamples,
		unsigned long             segmentIndex
	);

PREF0 PREF1 PICO_STATUS PREF2 PREF3 (ps6000SetSigGenArbitrary)
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
		PS6000_SWEEP_TYPE           sweepType,
		PS6000_EXTRA_OPERATIONS     operation,
		PS6000_INDEX_MODE           indexMode,
		unsigned long               shots,
		unsigned long               sweeps,
		PS6000_SIGGEN_TRIG_TYPE     triggerType,
		PS6000_SIGGEN_TRIG_SOURCE   triggerSource,
		short                       extInThreshold
	);

PREF0 PREF1 PICO_STATUS PREF2 PREF3(ps6000SetSigGenBuiltIn)
	(
		short                      handle,
		long                       offsetVoltage,
		unsigned long              pkToPk,
		short                      waveType,
		float                      startFrequency,
		float                      stopFrequency,
		float                      increment,
		float                      dwellTime,
		PS6000_SWEEP_TYPE          sweepType,
		PS6000_EXTRA_OPERATIONS    operation,
		unsigned long              shots,
		unsigned long              sweeps,
		PS6000_SIGGEN_TRIG_TYPE    triggerType,
		PS6000_SIGGEN_TRIG_SOURCE  triggerSource,
		short                      extInThreshold
	);

PREF0 PREF1 PICO_STATUS PREF2 PREF3 (ps6000SigGenSoftwareControl)
	(
		short                       handle,
		short                       state
	);


PREF0 PREF1 PICO_STATUS PREF2 PREF3 (ps6000SetSimpleTrigger)
	(
		short handle,
		short enable,
		PS6000_CHANNEL source,
		short threshold,
		PS6000_THRESHOLD_DIRECTION direction,
		unsigned long delay,
		short autoTrigger_ms
	);

PREF0 PREF1 PICO_STATUS PREF2 PREF3 (ps6000SetEts)
	(
		short                       handle,
		PS6000_ETS_MODE             mode,
		short                       etsCycles,
		short                       etsInterleave,
		long                      * sampleTimePicoseconds
	);


PREF0 PREF1 PICO_STATUS PREF2 PREF3 (ps6000SetTriggerChannelProperties)
	(
		short                               handle,
		PS6000_TRIGGER_CHANNEL_PROPERTIES * channelProperties,
		short                               nChannelProperties,
		short                               auxOutputEnable,
		long                                autoTriggerMilliseconds
	);


PREF0 PREF1 PICO_STATUS PREF2 PREF3 (ps6000SetTriggerChannelConditions)
	(
		short                       handle,
		PS6000_TRIGGER_CONDITIONS * conditions,
		short                       nConditions
	);

PREF0 PREF1 PICO_STATUS PREF2 PREF3 (ps6000SetTriggerChannelDirections)
	(
		short                         handle,
		PS6000_THRESHOLD_DIRECTION    channelA,
		PS6000_THRESHOLD_DIRECTION    channelB,
		PS6000_THRESHOLD_DIRECTION    channelC,
		PS6000_THRESHOLD_DIRECTION    channelD,
		PS6000_THRESHOLD_DIRECTION    ext,
		PS6000_THRESHOLD_DIRECTION    aux
	);

PREF0 PREF1 PICO_STATUS PREF2 PREF3 (ps6000SetTriggerDelay)
	(
		short                 handle,
		unsigned long         delay
	);

PREF0 PREF1 PICO_STATUS PREF2 PREF3 (ps6000SetPulseWidthQualifier)
	(
		short                           handle,
		PS6000_PWQ_CONDITIONS         * conditions,
		short                           nConditions,
		PS6000_THRESHOLD_DIRECTION      direction,
		unsigned long                   lower,
		unsigned long                   upper,
		PS6000_PULSE_WIDTH_TYPE         type
	);

PREF0 PREF1 PICO_STATUS PREF2 PREF3 (ps6000IsTriggerOrPulseWidthQualifierEnabled)
	(
		short                 handle,
		short               * triggerEnabled,
		short               * pulseWidthQualifierEnabled
	);

PREF0 PREF1 PICO_STATUS PREF2 PREF3 (ps6000GetTriggerTimeOffset)
	(
		short                 handle,
		unsigned long       * timeUpper,
		unsigned long       * timeLower,
		PS6000_TIME_UNITS   * timeUnits,
		unsigned long         segmentIndex
	);


PREF0 PREF1 PICO_STATUS PREF2 PREF3 (ps6000GetTriggerTimeOffset64)
	(
		short                 handle,
		__int64             * time,
		PS6000_TIME_UNITS   * timeUnits,
		unsigned long         segmentIndex
	);

PREF0 PREF1 PICO_STATUS PREF2 PREF3 (ps6000GetValuesTriggerTimeOffsetBulk)
	(
		short               handle,
		unsigned long     * timesUpper,
		unsigned long     * timesLower,
		PS6000_TIME_UNITS * timeUnits,
		unsigned long       fromSegmentIndex,
		unsigned long       toSegmentIndex
	);

PREF0 PREF1 PICO_STATUS PREF2 PREF3 (ps6000GetValuesTriggerTimeOffsetBulk64)
	(
		short               handle,
		__int64     * times,
		PS6000_TIME_UNITS * timeUnits,
		unsigned long       fromSegmentIndex,
		unsigned long       toSegmentIndex
	);

PREF0 PREF1 PICO_STATUS PREF2 PREF3 (ps6000SetDataBuffers)
	(
		short                 handle,
		PS6000_CHANNEL        channel,
		short               * bufferMax,
		short               * bufferMin,
		unsigned long         bufferLth,
		PS6000_RATIO_MODE     downSampleRatioMode
	);

PREF0 PREF1 PICO_STATUS PREF2 PREF3 (ps6000SetDataBuffer)
	(
		short                 handle,
		PS6000_CHANNEL        channel,
		short               * buffer,
		unsigned long         bufferLth,
		PS6000_RATIO_MODE     downSampleRatioMode
	);

PREF0 PREF1 PICO_STATUS PREF2 PREF3 (ps6000SetDataBufferBulk)
	(
		short                 handle,
		PS6000_CHANNEL        channel,
		short             *   buffer,
		unsigned long         bufferLth,
		unsigned long         waveform,
		PS6000_RATIO_MODE     downSampleRatioMode
	);


PREF0 PREF1 PICO_STATUS PREF2 PREF3 (ps6000SetDataBuffersBulk)
	(
		short                 handle,
		PS6000_CHANNEL        channel,
		short             *   bufferMax,
		short             *   bufferMin,
		unsigned long         bufferLth,
		unsigned long         waveform,
		PS6000_RATIO_MODE     downSampleRatioMode
	);

PREF0 PREF1 PICO_STATUS PREF2 PREF3 (ps6000SetEtsTimeBuffer)
	(
		short                 handle,
		__int64 *             buffer,
		unsigned long         bufferLth
	);

PREF0 PREF1 PICO_STATUS PREF2 PREF3 (ps6000SetEtsTimeBuffers)
	(
		short                 handle,
		unsigned long       * timeUpper,
		unsigned long       * timeLower,
		unsigned long         bufferLth
	);

PREF0 PREF1 PICO_STATUS PREF2 PREF3 (ps6000RunBlock)
	(
		short                 handle,
		unsigned long         noOfPreTriggerSamples,
		unsigned long         noOfPostTriggerSamples,
		unsigned long         timebase,
		short                 oversample,
		long                * timeIndisposedMs,
		unsigned long         segmentIndex,
		ps6000BlockReady      lpReady,
		void                * pParameter
	);

PREF0 PREF1 PICO_STATUS PREF2 PREF3 (ps6000IsReady)
	(
		short handle,
		short * ready
	);

PREF0 PREF1 PICO_STATUS PREF2 PREF3 (ps6000RunStreaming)
	(
		short                 handle,
		unsigned long       * sampleInterval,
		PS6000_TIME_UNITS     sampleIntervalTimeUnits,
		unsigned long         maxPreTriggerSamples,
		unsigned long         maxPostPreTriggerSamples,
		short                 autoStop,
		unsigned long         downSampleRatio,
		PS6000_RATIO_MODE     downSampleRatioMode,
		unsigned long         overviewBufferSize
	);

PREF0 PREF1 PICO_STATUS PREF2 PREF3 (ps6000GetStreamingLatestValues)
	(
		short                 handle,
		ps6000StreamingReady  lpPs6000Ready,
		void                * pParameter
	);

PREF0 PREF1 PICO_STATUS PREF2 PREF3 (ps6000NoOfStreamingValues)
	(
		short               handle,
		unsigned long     * noOfValues
	);

PREF0 PREF1 PICO_STATUS PREF2 PREF3 (ps6000GetMaxDownSampleRatio)
	(
		short               handle,
		unsigned long       noOfUnaggreatedSamples,
		unsigned long     * maxDownSampleRatio,
		PS6000_RATIO_MODE   downSampleRatioMode,
		unsigned long       segmentIndex
	);

PREF0 PREF1 PICO_STATUS PREF2 PREF3 (ps6000GetValues)
	(
		short               handle,
		unsigned long       startIndex,
		unsigned long     * noOfSamples,
		unsigned long       downSampleRatio,
		PS6000_RATIO_MODE   downSampleRatioMode,
		unsigned long       segmentIndex,
		short             * overflow
	);

PREF0 PREF1 PICO_STATUS PREF2 PREF3 (ps6000GetValuesBulk)
	(
		short               handle,
		unsigned long     * noOfSamples,
		unsigned long       fromSegmentIndex,
		unsigned long       toSegmentIndex,
		unsigned long       downSampleRatio,
		PS6000_RATIO_MODE   downSampleRatioMode,
		short             * overflow
	);

PREF0 PREF1 PICO_STATUS PREF2 PREF3 (ps6000GetValuesAsync)
	(
		short               handle,
		unsigned long       startIndex,
		unsigned long       noOfSamples,
		unsigned long       downSampleRatio,
		PS6000_RATIO_MODE   downSampleRatioMode,
		unsigned long       segmentIndex,
		void              * lpDataReady,
		void              * pParameter
	);

PREF0 PREF1 PICO_STATUS PREF2 PREF3 (ps6000GetValuesOverlapped)
	(
		short               handle,
		unsigned long       startIndex,
		unsigned long     * noOfSamples,
		unsigned long       downSampleRatio,
		PS6000_RATIO_MODE   downSampleRatioMode,
		unsigned long       segmentIndex,
		short             * overflow
	);


PREF0 PREF1 PICO_STATUS PREF2 PREF3 (ps6000GetValuesOverlappedBulk)
	(
		short               handle,
		unsigned long       startIndex,
		unsigned long     * noOfSamples,
		unsigned long       downSampleRatio,
		PS6000_RATIO_MODE   downSampleRatioMode,
		unsigned long       fromSegmentIndex,
		unsigned long       toSegmentIndex,
		short             * overflow
	);

PREF0 PREF1 PICO_STATUS PREF2 PREF3 (ps6000GetValuesBulkAsyc)
	(
		short               handle,
		unsigned long       startIndex,
		unsigned long     * noOfSamples,
		unsigned long       downSampleRatio,
		PS6000_RATIO_MODE   downSampleRatioMode,
		unsigned long       fromSegmentIndex,
		unsigned long       toSegmentIndex,
		short             * overflow
	);

PREF0 PREF1 PICO_STATUS PREF2 PREF3 (ps6000GetNoOfCaptures)
	(
		short               handle,
		unsigned long     * nCaptures
	);

PREF0 PREF1 PICO_STATUS PREF2 PREF3 (ps6000Stop)
	(
		short handle
	);

PREF0 PREF1 PICO_STATUS PREF2 PREF3 (ps6000SetNoOfCaptures) (short handle, unsigned long nCaptures);

PREF0 PREF1 PICO_STATUS PREF2 PREF3 (ps6000SetWaveformLimiter)
	(
		short handle,
		unsigned long nWaveformsPerSecond
	);

PREF0 PREF1 PICO_STATUS PREF2 PREF3 (ps6000EnumerateUnits)
	(
		short * count,
		char  * serials,
		short * serialLth
	);


PREF0 PREF1 PICO_STATUS PREF2 PREF3 (ps6000SetExternalClock)
	(
		short handle,
		PS6000_EXTERNAL_FREQUENCY frequency,
		short threshold
	);

PREF0 PREF1 PICO_STATUS PREF2 PREF3 (ps6000PingUnit)
	(
		short handle
	);

#endif
