/**************************************************************************
*
* Filename:    PS6000con.c
*
* Copyright:   Pico Technology Limited 2010
*
* Author:      RPM
*
* Description:
*   This is a console mode program that demonstrates how to use the
*   PicoScope 6000 series API.
*
* Examples:
*    Collect a block of samples immediately
*    Collect a block of samples when a trigger event occurs
*    Collect a stream of data immediately
*    Collect a stream of data when a trigger event occurs
*    Set Signal Generator, using standard or custom signals
*
*	To build this application:
*			 Set up a project for a 32-bit console mode application
*			 Add this file to the project
*			 Add PS6000.lib to the project
*			 Add ps6000Api.h and picoStatus.h to the project
*			 Build the project
*
***************************************************************************/

/* Headers for Windows */
#include "windows.h"
#include <conio.h>
#include <stdio.h>

#include "..\ps6000Api.h"

#define PREF4 __stdcall

int cycles = 0;

#define BUFFER_SIZE 	1024

typedef struct
{
	short DCcoupled;
	short range;
	short enabled;
}CHANNEL_SETTINGS;

typedef struct tTriggerDirections
{
	enum enThresholdDirection channelA;
	enum enThresholdDirection channelB;
	enum enThresholdDirection channelC;
	enum enThresholdDirection channelD;
	enum enThresholdDirection ext;
	enum enThresholdDirection aux;
}TRIGGER_DIRECTIONS;

typedef struct tPwq
{
	struct tPS6000PwqConditions * conditions;
	short nConditions;
	enum enThresholdDirection direction;
	unsigned long lower;
	unsigned long upper;
	enum enPulseWidthType type;
}PWQ;

typedef struct
{
	short handle;
	PS6000_RANGE			firstRange;
	PS6000_RANGE			lastRange;
	short							channelCount;
	CHANNEL_SETTINGS	channelSettings [PS6000_MAX_CHANNELS];
}UNIT;

unsigned long timebase = 8;
short       oversample = 1;
int      scaleVoltages = TRUE;

unsigned short inputRanges [PS6000_MAX_RANGES] = {	50,
																										100,
																										200,
																										500,
																										1000,
																										2000,
																										5000,
																										10000,
																										20000};
short     		g_ready = FALSE;
long long 		g_times [PS6000_MAX_CHANNELS];
short     		g_timeUnit;
long      		g_sampleCount;
unsigned long g_startIndex;
short         g_autoStopped;

/****************************************************************************
* Callback
* used by PS6000 data streaimng collection calls, on receipt of data.
* used to set global flags etc checked by user routines
****************************************************************************/
void PREF4 CallBackStreaming(	short handle,
															unsigned long noOfSamples,
															unsigned long	startIndex,
															short overflow,
															unsigned long triggerAt,
															short triggered,
															short autoStop,
															void	*pParameter)
{
	// used for streaming
	g_sampleCount = noOfSamples;
	g_startIndex	= startIndex;
	g_autoStopped		= autoStop;

	// flag to say done reading data
	g_ready = TRUE;
}

/****************************************************************************
* Callback
* used by PS6000 data block collection calls, on receipt of data.
* used to set global flags etc checked by user routines
****************************************************************************/
void PREF4 CallBackBlock(	short handle,
													PICO_STATUS status,
													void * pParameter)
{
	g_ready = TRUE;
}

/****************************************************************************
* SetDefaults - restore default settings
****************************************************************************/
void SetDefaults(UNIT * unit)
{
	PICO_STATUS status;
	int i;

	status = ps6000SetEts(unit->handle, PS6000_ETS_OFF, 0, 0, NULL); // Turn off ETS

	for (i = 0; i < unit->channelCount; i++) // reset channels to most recent settings
	{
		status = ps6000SetChannel(unit->handle, PS6000_CHANNEL_A + i,
			unit->channelSettings[PS6000_CHANNEL_A + i].enabled,
			unit->channelSettings[PS6000_CHANNEL_A + i].DCcoupled,
			unit->channelSettings[PS6000_CHANNEL_A + i].range, 0, PS6000_BW_FULL);
	}
}

/****************************************************************************
* adc_to_mv
*
* If the user selects scaling to millivolts,
* Convert an 16-bit ADC count into millivolts
****************************************************************************/
int adc_to_mv(long raw, int ch)
{
	return scaleVoltages ? (raw * inputRanges[ch]) / PS6000_MAX_VALUE : raw;
}

/****************************************************************************
* mv_to_adc
*
* Convert a millivolt value into a 16-bit ADC count
*
*  (useful for setting trigger thresholds)
****************************************************************************/
short mv_to_adc(short mv, short ch)
{
	return (mv * PS6000_MAX_VALUE) / inputRanges[ch];
}

/****************************************************************************
* BlockDataHandler
* - Used by all block data routines
* - acquires data (user sets trigger mode before calling), displays 10 items
*   and saves all to data.txt
* Input :
* - unit : the unit to use.
* - text : the text to display before the display of data slice
* - offset : the offset into the data buffer to start the display's slice.
****************************************************************************/
void BlockDataHandler(UNIT * unit, char * text, int offset)
{
	int i, j;
	long timeInterval;
	long sampleCount= BUFFER_SIZE;
	FILE * fp;
	long maxSamples;
	short * buffers[PS6000_MAX_CHANNEL_BUFFERS];
	long timeIndisposed;
	PICO_STATUS status;

	for (i = 0; i < unit->channelCount; i++) 
	{
		buffers[i * 2] = (short*)malloc(sampleCount * sizeof(short));
		buffers[i * 2 + 1] = (short*)malloc(sampleCount * sizeof(short));
		status = ps6000SetDataBuffers(unit->handle, (short)i, buffers[i * 2], buffers[i * 2 + 1], sampleCount, PS6000_RATIO_MODE_NONE);

		printf("BlockDataHandler:ps6000SetDataBuffers(channel %d) ------ %d \n", i, status);
	}

	/*  find the maximum number of samples, the time interval (in timeUnits),
	*		 the most suitable time units, and the maximum oversample at the current timebase*/
	while (ps6000GetTimebase(unit->handle, timebase, sampleCount, &timeInterval, oversample, &maxSamples, 0))
	{
		timebase++;
	}

	printf("timebase: %ld\toversample:%hd\n", timebase, oversample);

	/* Start it collecting, then wait for completion*/
	g_ready = FALSE;
	status = ps6000RunBlock(unit->handle, 0, sampleCount, timebase, oversample,	&timeIndisposed, 0, CallBackBlock, NULL);

	printf("BlockDataHandler:ps6000RunBlock ------ %i \n", status);
	printf("Waiting for trigger...Press a key to abort\n");

	while (!g_ready && !_kbhit())
	{
		Sleep(0);
	}

	status = ps6000Stop(unit->handle);
	printf("BlockDataHandler:ps6000Stop ------ %i \n", status);

	if(g_ready) 
	{
		status = ps6000GetValues(unit->handle, 0, (unsigned long*) &sampleCount, 1, PS6000_RATIO_MODE_NONE, 0, NULL);
		printf("BlockDataHandler:ps6000GetValues ------ %i \n", status);

		/* Print out the first 10 readings, converting the readings to mV if required */
		printf(text);
		printf("Value (%s)\n", ( scaleVoltages ) ? ("mV") : ("ADC Counts"));

		for (i = offset; i < offset+10; i++) 
		{
			for (j = 0; j < unit->channelCount; j++) 
			{
				if (unit->channelSettings[j].enabled) 
				{
					printf("%d\t", adc_to_mv(buffers[j * 2][i], unit->channelSettings[PS6000_CHANNEL_A + j].range));
				}
			}
			printf("\n");
		}

		sampleCount = __min(sampleCount, BUFFER_SIZE);

		fopen_s(&fp, "data.txt", "w");
		if (fp != NULL)
		{
			for (i = 0; i < sampleCount; i++) 
			{
				for (j = 0; j < unit->channelCount; j++) 
				{
					fprintf(fp, "%lld ", g_times[j] + (long long)(i * timeInterval));
					if (unit->channelSettings[j].enabled) 
					{
						fprintf(fp,
										", %d, %d", 
										buffers[j * 2][i], 
										adc_to_mv(buffers[j * 2][i], 
										unit->channelSettings[PS6000_CHANNEL_A + j].range));
					}
				}
				fprintf(fp, "\n");
			}
		}
		else
			printf(	"Cannot open the file data.txt for writing.\n"
							"Please ensure that you have permission to access.\n");
	} 
	else 
	{
		printf("data collection aborted\n");
		_getch();
	}

	if (fp != NULL)
		fclose(fp);

	for (i = 0; i < unit->channelCount * 2; i++) 
	{
		free(buffers[i]);
	}
}
/****************************************************************************
* Stream Data Handler
* - Used by the two stream data examples - untriggered and triggered
* Inputs:
* - unit - the unit to sample on
* - preTrigger - the number of samples in the pre-trigger phase 
*					(0 if no trigger has been set)
***************************************************************************/
void StreamDataHandler(UNIT * unit, unsigned long preTrigger)
{
	long i, j;
	unsigned long sampleCount= BUFFER_SIZE * 10; /*  *10 is to make sure buffer large enough */
	FILE * fp;
	short * buffers[PS6000_MAX_CHANNEL_BUFFERS];
	PICO_STATUS status;
	unsigned long sampleInterval = 1;
	int index = 0;
	int totalSamples;

	for (i = 0; i < unit->channelCount; i++) // create data buffers
	{
		buffers[i * 2] = (short*) malloc(sampleCount * sizeof(short));
		buffers[i * 2 + 1] = (short*)malloc(sampleCount * sizeof(short));
		status = ps6000SetDataBuffers(	unit->handle, 
																		i, 
																		buffers[i * 2],
																		buffers[i * 2 + 1], 
																		sampleCount, 
																		PS6000_RATIO_MODE_NONE);
	}

	printf("Waiting for trigger...Press a key to abort\n");
	g_autoStopped = FALSE;

	status = ps6000RunStreaming(unit->handle, 
															&sampleInterval, 
															PS6000_US,
															preTrigger, 
															1000000 - preTrigger, 
															FALSE,
															//TRUE,
															1000,
															PS6000_RATIO_MODE_AGGREGATE,
															sampleCount);

	printf("Streaming data...Press a key to abort\n");

	fopen_s(&fp, "data.txt", "w");

	totalSamples = 0;
	while (!_kbhit() && !g_autoStopped)
	{
		/* Poll until data is received. Until then, GetStreamingLatestValues wont call the callback */
		Sleep(100);
		g_ready = FALSE;

		status = ps6000GetStreamingLatestValues(unit->handle, CallBackStreaming, NULL);
		index ++;

		if (g_ready && g_sampleCount > 0) /* can be ready and have no data, if autoStop has fired */
		{
			totalSamples += g_sampleCount;
			printf("Collected %li samples, index = %lu, Total: %li samples\n", g_sampleCount, g_startIndex, totalSamples);

			if (fp != NULL)
			{
				for (i = g_startIndex; i < (long)(g_startIndex + g_sampleCount); i++) 
				{
					for (j = 0; j < unit->channelCount; j++) 
					{
						if (unit->channelSettings[j].enabled) 
						{
							fprintf(	fp,
												"%d, %d, %d, %d,",
												buffers[j * 2][i],
												adc_to_mv(buffers[j * 2][i], unit->channelSettings[PS6000_CHANNEL_A + j].range),
												buffers[j * 2 + 1][i],
												adc_to_mv(buffers[j * 2 + 1][i], unit->channelSettings[PS6000_CHANNEL_A + j].range));
						}
					}
					fprintf(fp, "\n");
				}
			}
			else
				printf("Cannot open the file data.txt for writing.\n");
		}
	}

	if(fp != NULL) fclose(fp);	

	ps6000Stop(unit->handle);

	if (!g_autoStopped) 
	{
		printf("data collection aborted\n");
		_getch();
	}

	for (i = 0; i < unit->channelCount * 2; i++) 
	{
		free(buffers[i]);
	}
}

PICO_STATUS SetTrigger(	short handle,
												struct tPS6000TriggerChannelProperties * channelProperties,
												short nChannelProperties,
												struct tPS6000TriggerConditions * triggerConditions,
												short nTriggerConditions,
												TRIGGER_DIRECTIONS * directions,
												struct tPwq * pwq,
												unsigned long delay,
												short auxOutputEnabled,
												long autoTriggerMs)
{
	PICO_STATUS status;

	if ((status = ps6000SetTriggerChannelProperties(handle,
																									channelProperties,
																									nChannelProperties,
																									auxOutputEnabled,
																									autoTriggerMs)) != PICO_OK) 
	{
			printf("SetTrigger:ps6000SetTriggerChannelProperties ------ %d \n", status);
			return status;
	}

	if ((status = ps6000SetTriggerChannelConditions(handle,	triggerConditions, nTriggerConditions)) != PICO_OK) 
	{
			printf("SetTrigger:ps6000SetTriggerChannelConditions ------ %d \n", status);
			return status;
	}

	if ((status = ps6000SetTriggerChannelDirections(handle,
																									directions->channelA,
																									directions->channelB,
																									directions->channelC,
																									directions->channelD,
																									directions->ext,
																									directions->aux)) != PICO_OK) 
	{
			printf("SetTrigger:ps6000SetTriggerChannelDirections ------ %d \n", status);
			return status;
	}

	if ((status = ps6000SetTriggerDelay(handle, delay)) != PICO_OK) 
	{
		printf("SetTrigger:ps6000SetTriggerDelay ------ %d \n", status);
		return status;
	}

	if((status = ps6000SetPulseWidthQualifier(handle, 
																						pwq->conditions,
																						pwq->nConditions, 
																						pwq->direction,
																						pwq->lower, 
																						pwq->upper, 
																						pwq->type)) != PICO_OK)
	{
		printf("SetTrigger:ps6000SetPulseWidthQualifier ------ %d \n", status);
		return status;
	}

	return status;
}

/****************************************************************************
* CollectBlockImmediate
*  this function demonstrates how to collect a single block of data
*  from the unit (start collecting immediately)
****************************************************************************/
void CollectBlockImmediate(UNIT * unit)
{
	struct tPwq pulseWidth;
	struct tTriggerDirections directions;

	memset(&directions, 0, sizeof(struct tTriggerDirections));
	memset(&pulseWidth, 0, sizeof(struct tPwq));

	printf("Collect block immediate...\n");
	printf("Press a key to start\n");
	_getch();

	SetDefaults(unit);

	/* Trigger disabled	*/
	SetTrigger(unit->handle, NULL, 0, NULL, 0, &directions, &pulseWidth, 0, 0, 0);

	BlockDataHandler(unit, "First 10 readings\n", 0);
}

/****************************************************************************
* CollectBlockEts
*  this function demonstrates how to collect a block of
*  data using equivalent time sampling (ETS).
****************************************************************************/
void CollectBlockEts(UNIT * unit)
{
	PICO_STATUS status;
	long ets_sampletime;
	short	triggerVoltage = mv_to_adc(100,	unit->channelSettings[PS6000_CHANNEL_A].range);
	unsigned long delay = 0;
	struct tPwq pulseWidth;
	struct tTriggerDirections directions;

	struct tPS6000TriggerChannelProperties sourceDetails = {	triggerVoltage,
																														triggerVoltage,
																														10,
																														PS6000_CHANNEL_A,
																														PS6000_LEVEL };

	struct tPS6000TriggerConditions conditions = {	PS6000_CONDITION_TRUE,
																									PS6000_CONDITION_DONT_CARE,
																									PS6000_CONDITION_DONT_CARE,
																									PS6000_CONDITION_DONT_CARE,
																									PS6000_CONDITION_DONT_CARE,
																									PS6000_CONDITION_DONT_CARE,
																									PS6000_CONDITION_DONT_CARE };



	memset(&pulseWidth, 0, sizeof(struct tPwq));
	memset(&directions, 0, sizeof(struct tTriggerDirections));
	directions.channelA = PS6000_RISING;

	printf("Collect ETS block...\n");
	printf("Collects when value rises past %dmV\n",	adc_to_mv(sourceDetails.thresholdUpper,	unit->channelSettings[PS6000_CHANNEL_A].range));
	printf("Press a key to start...\n");
	_getch();

	SetDefaults(unit);

	//Trigger enabled
	//Rising edge
	//Threshold = 1500mV
	status = SetTrigger(unit->handle, &sourceDetails, 1, &conditions, 1, &directions, &pulseWidth, delay, 0, 0);

	status = ps6000SetEts(unit->handle, PS6000_ETS_FAST, 20, 4, &ets_sampletime);
	printf("ETS Sample Time is: %ld\n", ets_sampletime);

	BlockDataHandler(unit, "Ten readings after trigger\n", BUFFER_SIZE / 10 - 5); // 10% of data is pre-trigger
}

/****************************************************************************
* CollectBlockTriggered
*  this function demonstrates how to collect a single block of data from the
*  unit, when a trigger event occurs.
****************************************************************************/
void CollectBlockTriggered(UNIT * unit)
{
	short	triggerVoltage = mv_to_adc(100, unit->channelSettings[PS6000_CHANNEL_A].range);

	struct tPS6000TriggerChannelProperties sourceDetails = {	triggerVoltage,
																														256 * 10,
																														triggerVoltage,
																														256 * 10,
																														PS6000_CHANNEL_A,
																														PS6000_LEVEL};

	struct tPS6000TriggerConditions conditions = {	PS6000_CONDITION_TRUE,
																									PS6000_CONDITION_DONT_CARE,
																									PS6000_CONDITION_DONT_CARE,
																									PS6000_CONDITION_DONT_CARE,
																									PS6000_CONDITION_DONT_CARE,
																									PS6000_CONDITION_DONT_CARE,
																									PS6000_CONDITION_DONT_CARE};

	struct tPwq pulseWidth;

	struct tTriggerDirections directions = {	PS6000_RISING,
																						PS6000_NONE,
																						PS6000_NONE,
																						PS6000_NONE,
																						PS6000_NONE,
																						PS6000_NONE };

	memset(&pulseWidth, 0, sizeof(struct tPwq));

	printf("Collect block triggered...\n");
	printf("Collects when value rises past %dmV\n",

	adc_to_mv(sourceDetails.thresholdUpper, unit->channelSettings[PS6000_CHANNEL_A].range));

	printf("Press a key to start...\n");
	_getch();

	SetDefaults(unit);

	/* Trigger enabled
	* Rising edge
	* Threshold = 100mV */
	SetTrigger(unit->handle, &sourceDetails, 1, &conditions, 1, &directions, &pulseWidth, 0, 0, 0);

	BlockDataHandler(unit, "Ten readings after trigger\n", 0);
}

/****************************************************************************
* CollectRapidBlock
*  this function demonstrates how to collect a set of captures using 
*  rapid block mode.
****************************************************************************/
void CollectRapidBlock(UNIT * unit)
{
	unsigned short nCaptures;
	unsigned long nMaxSamples, nSamples = 1000;
	long timeIndisposed;
	short capture, channel;
	short ***rapidBuffers;
	short *overflow;
	PICO_STATUS status;
	short i;
	unsigned long nCompletedCaptures;

	short	triggerVoltage = mv_to_adc(100, unit->channelSettings[PS6000_CHANNEL_A].range);

	struct tPS6000TriggerChannelProperties sourceDetails = {	triggerVoltage,
																														256 * 10,
																														triggerVoltage,
																														256 * 10,
																														PS6000_CHANNEL_A,
																														PS6000_LEVEL};

	struct tPS6000TriggerConditions conditions = {	PS6000_CONDITION_TRUE,
																									PS6000_CONDITION_DONT_CARE,
																									PS6000_CONDITION_DONT_CARE,
																									PS6000_CONDITION_DONT_CARE,
																									PS6000_CONDITION_DONT_CARE,
																									PS6000_CONDITION_DONT_CARE,
																									PS6000_CONDITION_DONT_CARE};

	struct tPwq pulseWidth;

	struct tTriggerDirections directions = {	PS6000_RISING,
																						PS6000_NONE,
																						PS6000_NONE,
																						PS6000_NONE,
																						PS6000_NONE,
																						PS6000_NONE };

	memset(&pulseWidth, 0, sizeof(struct tPwq));

	printf("Collect rapid block triggered...\n");
	printf("Collects when value rises past %dmV\n",	
		adc_to_mv(sourceDetails.thresholdUpper, unit->channelSettings[PS6000_CHANNEL_A].range));
	printf("Press any key to abort\n");

	SetDefaults(unit);

	// Trigger enabled
	SetTrigger(unit->handle, &sourceDetails, 1, &conditions, 1, &directions, &pulseWidth, 0, 0, 0);

	//Set the number of captures
	nCaptures = 10;

	//Segment the memory
	status = ps6000MemorySegments(unit->handle, nCaptures, &nMaxSamples);

	//Set the number of captures
	status = ps6000SetNoOfCaptures(unit->handle, nCaptures);

	//Run
	timebase = 160;		//1 MS/s
	status = ps6000RunBlock(unit->handle, 0, nSamples, timebase, 1, &timeIndisposed, 0, CallBackBlock, NULL);

	//Wait until data ready
	g_ready = 0;
	while(!g_ready && !_kbhit())
	{
		Sleep(0);
	}

	if(!g_ready)
	{
		_getch();
		status = ps6000Stop(unit->handle);
		status = ps6000GetNoOfCaptures(unit->handle, &nCompletedCaptures);
		printf("Rapid capture aborted. %d complete blocks were captured\n", nCompletedCaptures);
		printf("\nPress any key...\n\n");
		_getch();

		if(nCompletedCaptures == 0)
			return;
		
		//Only display the blocks that were captured
		nCaptures = (unsigned short)nCompletedCaptures;
	}

	//Allocate memory
	rapidBuffers = calloc(unit->channelCount, sizeof(short*));
	overflow = calloc(unit->channelCount * nCaptures, sizeof(short));

	for (channel = 0; channel < unit->channelCount; channel++) 
	{
		rapidBuffers[channel] = calloc(nCaptures, sizeof(short*));
	}

	for (channel = 0; channel < unit->channelCount; channel++) 
	{	
		if(unit->channelSettings[channel].enabled)
		{
			for (capture = 0; capture < nCaptures; capture++) 
			{
				rapidBuffers[channel][capture] = calloc(nSamples, sizeof(short));
			}
		}
	}

	for (channel = 0; channel < unit->channelCount; channel++) 
	{
		if(unit->channelSettings[channel].enabled)
		{
			for (capture = 0; capture < nCaptures; capture++) 
			{
				status = ps6000SetDataBufferBulk(unit->handle, channel, rapidBuffers[channel][capture], nSamples, capture, PS6000_RATIO_MODE_NONE);
			}
		}
	}

	//Get data
	status = ps6000GetValuesBulk(unit->handle, &nSamples, 0, nCaptures - 1, 1, PS6000_RATIO_MODE_NONE, overflow);

	//Stop
	status = ps6000Stop(unit->handle);

	//print first 10 samples from each capture
	for (capture = 0; capture < nCaptures; capture++)
	{
		printf("Capture %d\n", capture + 1);
		printf("Channel A\tChannel B\tChannel C\tChannel D\n");

		for(i = 0; i < 10; i++)
		{
			printf("%d\t\t%d\t\t%d\t\t%d\n", rapidBuffers[0][capture][i], rapidBuffers[1][capture][i], rapidBuffers[2][capture][i], rapidBuffers[3][capture][i]);
		}
		printf("\n");
	}

	//Free memory
	free(overflow);

	for (channel = 0; channel < unit->channelCount; channel++) 
	{	
		if(unit->channelSettings[channel].enabled)
		{
			for (capture = 0; capture < nCaptures; capture++) 
			{
				free(rapidBuffers[channel][capture]);
			}
		}
	}

	for (channel = 0; channel < unit->channelCount; channel++) 
	{
		free(rapidBuffers[channel]);
	}
	free(rapidBuffers);
}

/****************************************************************************
* Initialise unit' structure with Variant specific defaults
****************************************************************************/
void get_info(UNIT * unit)
{
	char description [6][25]= { "Driver Version",
															"USB Version",
															"Hardware Version",
															"Variant Info",
															"Serial",
															"Error Code" };
	short i, r = 0;
	char line [80];
	int variant;
	PICO_STATUS status = PICO_OK;

	if (unit->handle) 
	{
		for (i = 0; i < 5; i++) 
		{
			status = ps6000GetUnitInfo(unit->handle, line, sizeof (line), &r, i);
			if (i == 3) 
			{
				variant = atoi(line);
			}
			printf("%s: %s\n", description[i], line);
		}
		unit->firstRange = PS6000_50MV;
		unit->lastRange = PS6000_20V;
		unit->channelCount = 4;
	}
}

/****************************************************************************
* Select input voltage ranges for channels
****************************************************************************/
void SetVoltages(UNIT * unit)
{
	int i, ch;

	/* See what ranges are available... */
	for (i = unit->firstRange; i <= unit->lastRange; i++) 
	{
		printf("%d -> %d mV\n", i, inputRanges[i]);
	}

	/* Ask the user to select a range */
	printf("Specify voltage range (%d..%d)\n", unit->firstRange, unit->lastRange);
	printf("99 - switches channel off\n");
	for (ch = 0; ch < unit->channelCount; ch++) 
	{
		printf("\n");
		do 
		{
			printf("Channel %c: ", 'A' + ch);
			fflush(stdin);
			scanf_s("%hd", &unit->channelSettings[ch].range);
		} while (unit->channelSettings[ch].range != 99 && (unit->channelSettings[ch].range < unit->firstRange || unit->channelSettings[ch].range > unit->lastRange));

		if (unit->channelSettings[ch].range != 99) 
		{
			printf(" - %d mV\n", inputRanges[unit->channelSettings[ch].range]);
			unit->channelSettings[ch].enabled = TRUE;
		} 
		else 
		{
			printf("Channel Switched off\n");
			unit->channelSettings[ch].enabled = FALSE;
		}
	}
}

/****************************************************************************
*
* Select timebase, set oversample to on and time units as nano seconds
*
****************************************************************************/
void SetTimebase(UNIT unit)
{
	long timeInterval;
	long maxSamples;

	printf("Specify timebase: ");
	fflush(stdin);
	scanf_s("%lud", &timebase);

	ps6000GetTimebase(unit.handle, timebase, BUFFER_SIZE, &timeInterval, 1, &maxSamples, 0);
	printf("Timebase %lud - %ld ns\n", timebase, timeInterval);
	oversample = TRUE;
}

/****************************************************************************
* Sets the signal generator
* - allows user to set frequency and waveform
* - allows for custom waveform (values 0..4192) of up to 8192 samples long
***************************************************************************/
void SetSignalGenerator(UNIT unit)
{
	PICO_STATUS status;
	short waveform;
	long frequency;
	char fileName [128];
	FILE * fp;
	short arbitraryWaveform [8192];
	short waveformSize = 0;

	memset(&arbitraryWaveform, 0, 8192);

	printf("Enter frequency in Hz: "); // Ask user to enter signal frequency;
	do 
	{
		scanf_s("%lu", &frequency);
	} while (frequency <= 0 || frequency > 10000000);

	if(frequency > 0) // Ask user to enter type of signal
	{
		printf("Signal generator On");
		printf("Enter type of waveform (0..9 or 99)\n");
		printf("0:\tSINE\n");
		printf("1:\tSQUARE\n");
		printf("2:\tTRIANGLE\n");
		printf("99:\tUSER WAVEFORM\n");
		printf("  \t(see manual for more)\n");

		do 
		{
			scanf_s("%hd", &waveform);
		} while (waveform != 99 && (waveform < 0 || waveform >= PS6000_MAX_WAVE_TYPES));

		if (waveform == 99) // custom waveform selected - user needs to select file
		{
			waveformSize = 0;

			printf("Select a waveform file to load: ");
			scanf_s("%s", fileName, 128);
			if (fopen_s(&fp, fileName, "r") == 0) 
			{ // Having opened file, read in data - one number per line (at most 8192 lines), with values in (0..4095)
				while (EOF != fscanf_s(fp, "%hi", (arbitraryWaveform + waveformSize))&& waveformSize++ < 8192);
				fclose(fp);
				printf("File successfully loaded\n");
			} 
			else 
			{
				printf("Invalid filename\n");
				return;
			}
		}
	} 
	else 
	{
		waveform = 0;
		printf("Signal generator Off");
	}

	if (waveformSize > 0) 
	{
		double delta = ((frequency * waveformSize) / 8192.0) * 4294967296.0 * 8e-9; // delta >= 10
		
		status = ps6000SetSigGenArbitrary(	unit.handle, 
																				0, 
																				1000000, 
																				(unsigned long)delta, 
																				(unsigned long)delta, 
																				0, 
																				0, 
																				arbitraryWaveform, 
																				waveformSize, 
																				0,
																				0, 
																				PS6000_SINGLE, 
																				0, 
																				0, 
																				PS6000_SIGGEN_RISING,
																				PS6000_SIGGEN_NONE, 
																				0);

		printf("Status of Arbitrary Gen: %x \n", (unsigned int)status);
	} 
	else 
	{
		status = ps6000SetSigGenBuiltIn(unit.handle, 0, 1000000, waveform, (float)frequency, (float)frequency, 0, 0, 0, 0, 0, 0, 0, 0, 0);
	}
}

/****************************************************************************
* CollectStreamingImmediate
*  this function demonstrates how to collect a stream of data
*  from the unit (start collecting immediately)
***************************************************************************/
void CollectStreamingImmediate(UNIT * unit)
{
	struct tPwq pulseWidth;
	struct tTriggerDirections directions;

	memset(&pulseWidth, 0, sizeof(struct tPwq));
	memset(&directions, 0, sizeof(struct tTriggerDirections));

	SetDefaults(unit);

	printf("Collect streaming...\n");
	printf("Data is written to disk file (data.txt)\n");
	printf("Press a key to start\n");
	_getch();

	/* Trigger disabled	*/
	SetTrigger(unit->handle, NULL, 0, NULL, 0, &directions, &pulseWidth, 0, 0, 0);

	StreamDataHandler(unit, 0);
}

/****************************************************************************
* CollectStreamingTriggered
*  this function demonstrates how to collect a stream of data
*  from the unit (start collecting on trigger)
***************************************************************************/
void CollectStreamingTriggered(UNIT * unit)
{
	short triggerVoltage = mv_to_adc(100,	unit->channelSettings[PS6000_CHANNEL_A].range); // ChannelInfo stores ADC counts
	struct tPwq pulseWidth;
	
	struct tPS6000TriggerChannelProperties sourceDetails = {	triggerVoltage,
																														triggerVoltage,
																														256 * 10,
																														PS6000_CHANNEL_A,
																														PS6000_LEVEL };
	
	struct tPS6000TriggerConditions conditions = {	PS6000_CONDITION_TRUE,
																									PS6000_CONDITION_DONT_CARE,
																									PS6000_CONDITION_DONT_CARE,
																									PS6000_CONDITION_DONT_CARE,
																									PS6000_CONDITION_DONT_CARE,
																									PS6000_CONDITION_DONT_CARE,
																									PS6000_CONDITION_DONT_CARE };

	struct tTriggerDirections directions = {	PS6000_RISING,
																						PS6000_NONE,
																						PS6000_NONE,
																						PS6000_NONE,
																						PS6000_NONE,
																						PS6000_NONE };

	memset(&pulseWidth, 0, sizeof(struct tPwq));

	printf("Collect streaming triggered...\n");
	printf("Data is written to disk file (data.txt)\n");
	printf("Press a key to start\n");
	_getch();
	SetDefaults(unit);

	/* Trigger enabled
	* Rising edge
	* Threshold = 100mV */
	SetTrigger(unit->handle, &sourceDetails, 1, &conditions, 1, &directions, &pulseWidth, 0, 0, 0);

	StreamDataHandler(unit, 100000);
}


int main(void)
{
	char ch;
	int i;
	PICO_STATUS status;
	UNIT unit;
	struct tPwq pulseWidth;
	struct tTriggerDirections directions;

	printf("PS6000 driver example program\n");
	printf("Version 1.1\n\n");
	printf("\n\nOpening the device...\n");

	status = ps6000OpenUnit(&(unit.handle), NULL);
	printf("Handle: %d\n", unit.handle);
	if (status != PICO_OK) 
	{
		printf("Unable to open device\n");
		printf("Error code : %d\n", (int)status);
		while(!_kbhit());
		exit(99); // exit program
	}

	printf("Device opened successfully, cycle %d\n\n", ++cycles);

	// setup devices
	get_info(&unit);
	timebase = 1;

	for (i = 0; i < PS6000_MAX_CHANNELS; i++) 
	{
		unit.channelSettings[i].enabled = TRUE;
		unit.channelSettings[i].DCcoupled = TRUE;
		unit.channelSettings[i].range = PS6000_5V;
	}

	memset(&directions, 0, sizeof(struct tTriggerDirections));
	memset(&pulseWidth, 0, sizeof(struct tPwq));

	SetDefaults(&unit);

	/* Trigger disabled	*/
	SetTrigger(unit.handle, NULL, 0, NULL, 0, &directions, &pulseWidth, 0, 0, 0);

	ch = ' ';
	while (ch != 'X')
	{
		printf("\n");
		printf("B - Immediate block                           V - Set voltages\n");
		printf("T - Triggered block                           I - Set timebase\n");
		printf("E - Collect a block of data using ETS         A - ADC counts/mV\n");
		printf("R - Collect set of rapid captures\n");
		printf("S - Immediate streaming\n");
		printf("W - Triggered streaming\n");
		printf("G - Signal generator\n");
		printf("                                              X - Exit\n");
		printf("Operation:");

		ch = toupper(_getch());

		printf("\n\n");
		switch (ch) 
		{
		case 'B':
			CollectBlockImmediate(&unit);
			break;

		case 'T':
			CollectBlockTriggered(&unit);
			break;

		case 'R':
			CollectRapidBlock(&unit);
			break;

		case 'S':
			CollectStreamingImmediate(&unit);
			break;

		case 'W':
			CollectStreamingTriggered(&unit);
			break;

		case 'E':
			CollectBlockEts(&unit);
			break;

		case 'G':
			SetSignalGenerator(unit);
			break;

		case 'V':
			SetVoltages(&unit);
			break;

		case 'I':
			SetTimebase(unit);
			break;

		case 'A':
			scaleVoltages = !scaleVoltages;
			if (scaleVoltages) 
				printf("Readings will be scaled in mV\n");
			else 
				printf("Readings will be scaled in ADC counts\n");
			break;

		case 'X':
			break;

		default:
			printf("Invalid operation\n");
			break;
		}
	}
	ps6000CloseUnit(unit.handle);

	return 1;
}
