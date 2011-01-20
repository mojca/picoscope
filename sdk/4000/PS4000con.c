/**************************************************************************
*
* Filename:    PS4000con.c
*
* Copyright:   Pico Technology Limited 2010
*
* Author:      PT
*
* Description:
*   This is a console-mode program that demonstrates how to use the
*   PS4000 driver.
*
* Examples:
*    Collect a block of samples immediately
*    Collect a block of samples when a trigger event occurs
*    Collect a stream of data immediately
*    Collect a stream of data when a trigger event occurs
*    Set Signal Generator, using built in or custom signals
*             For PicoScope 4223, 4224, 4423, 4424, 4226, 4227
*
*	To build this application
*		Windows: set up a project for a 32-bit console mode application
*			 add this file to the project
*			 add PS4000.lib to the project (Microsoft C only)
*			 add ps4000Api.h and picoStatus.h to the project
*			 then build the project
*
*		Linux: gcc -lps4000 PS4000con.c -oPS4000con
*		       ./PS4000con
*
*	Hisotry
*		Jun 2010:	RPM Modified to include PicoScope 4226 and 4227
*
***************************************************************************/

#ifdef WIN32
/* Headers for Windows */
#include "windows.h"
#include <conio.h>
#include <stdio.h>

/* Definitions of PS4000 driver routines on Windows*/
#include "ps4000Api.h"

#define PREF4 __stdcall

#else
/* Headers for Linux*/
#include <stdlib.h> 	
#include <string.h>
#include <stdio.h>
#include <ctype.h>

/* For _kbhit / _getch */
#include <termios.h>
#include <unistd.h>
#include <sys/select.h>
#include <sys/ioctl.h>

/* Definition of PS3000 driver routines on Linux */
#include <libps4000/ps4000Api.h>

enum BOOL {FALSE,TRUE};

#define PREF4
#define __min(a,b) (((a) < (b)) ? (a) : (b))
#define __max(a,b) (((a) > (b)) ? (a) : (b))
#define Sleep(x) usleep(1000*(x))

/* This example was originally intended for Windows. Simulate the Win32 _kbhit
* and _getch console IO functions */
int _kbhit()
{
	struct termios oldt, newt;
	int bytesWaiting;
	tcgetattr(STDIN_FILENO, &oldt);
	newt = oldt;
	newt.c_lflag &= ~( ICANON | ECHO );
	tcsetattr(STDIN_FILENO, TCSANOW, &newt);
	setbuf(stdin, NULL);
	ioctl(STDIN_FILENO, FIONREAD, &bytesWaiting);

	tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
	return bytesWaiting;
}

int _getch()
{
	struct termios oldt, newt;
	int ch;
	int bytesWaiting;
	tcgetattr(STDIN_FILENO, &oldt);
	newt = oldt;
	newt.c_lflag &= ~( ICANON | ECHO );
	tcsetattr(STDIN_FILENO, TCSANOW, &newt);
	setbuf(stdin, NULL);
	do {
		ioctl(STDIN_FILENO, FIONREAD, &bytesWaiting);
		if (bytesWaiting)
			getchar();
	} while (bytesWaiting);

	ch = getchar();

	tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
	return ch;
}

#endif

int cycles = 0;

#define BUFFER_SIZE 	1024
#define MAX_CHANNELS 4
#define QUAD_SCOPE 4
#define TRIPLE_SCOPE 3
#define DUAL_SCOPE 2
#define SEGMEM		10

typedef struct
{
	short DCcoupled;
	short range;
	short enabled;
}CHANNEL_SETTINGS;

typedef enum
{
	MODEL_NONE = 0,
	MODEL_PS4223 = 4223,
	MODEL_PS4224 = 4224,
	MODEL_PS4423 = 4423,
	MODEL_PS4424 = 4424,
	MODEL_PS4226 = 4226, 
	MODEL_PS4227 = 4227
} MODEL_TYPE;

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
	struct tPwqConditions * conditions;
	short nConditions;
	enum enThresholdDirection direction;
	unsigned long lower;
	unsigned long upper;
	enum enPulseWidthType type;
}PWQ;

typedef struct
{
	short handle;
	MODEL_TYPE model;
	PS4000_RANGE firstRange;
	PS4000_RANGE lastRange;
	unsigned char signalGenerator;
	unsigned char ETS;
	short ChannelCount;
	CHANNEL_SETTINGS channelSettings[MAX_CHANNELS];
	PS4000_RANGE triggerRange;
}UNIT_MODEL;

unsigned long timebase = 8;
short					oversample = 1;
int						scaleVoltages = TRUE;

unsigned short inputRanges [PS4000_MAX_RANGES] = { 
10,
20,
50,
100,
200,
500,
1000,
2000,
5000,
10000,
20000,
50000 };

short     		g_ready = FALSE;
long long 		g_times [PS4000_MAX_CHANNELS];
short     		g_timeUnit;
long      		g_sampleCount;
unsigned long g_startIndex;
short         g_autoStop;

/****************************************************************************
* Callback
* used by PS4000 data streaimng collection calls, on receipt of data.
* used to set global flags etc checked by user routines
****************************************************************************/
void PREF4 CallBackStreaming
(
 short handle,
 long noOfSamples,
 unsigned long startIndex,
 short overflow,
 unsigned long triggerAt,
 short triggered,
 short autoStop,
 void * pParameter
 )
{
	// used for streaming
	g_sampleCount = noOfSamples;
	g_startIndex = startIndex;
	g_autoStop = autoStop;

	// flag to say done reading data
	g_ready = TRUE;
}

/****************************************************************************
* Callback
* used by PS4000 data block collection calls, on receipt of data.
* used to set global flags etc checked by user routines
****************************************************************************/
void PREF4 CallBackBlock
(
 short handle,
 PICO_STATUS status,
 void * pParameter
 )
{
	// flag to say done reading data
	g_ready = TRUE;
}

/****************************************************************************
* SetDefaults - restore default settings
****************************************************************************/
void SetDefaults(UNIT_MODEL * unit)
{
	PICO_STATUS status;
	int i;

	status = ps4000SetEts(unit->handle, PS4000_ETS_OFF, 0, 0, NULL); // Turn off ETS
	//printf("SetDefaults: ps4000SetEts ------ %d \n", status);

	for (i = 0; i < unit->ChannelCount; i++) // reset channels to most recent settings
	{
		status = ps4000SetChannel(unit->handle, PS4000_CHANNEL_A + i,
															unit->channelSettings[PS4000_CHANNEL_A + i].enabled,
															unit->channelSettings[PS4000_CHANNEL_A + i].DCcoupled,
															unit->channelSettings[PS4000_CHANNEL_A + i].range);

		//printf("SetDefaults: ps4000SetChannel(channel: %d)------ %d \n", i, status);
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
	return ( scaleVoltages ) ? ( raw * inputRanges[ch] ) / PS4000_MAX_VALUE : raw;
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
	return ((mv * PS4000_MAX_VALUE ) / inputRanges[ch]);
}

/****************************************************************************
* RapidBlockDataHandler
* - Used by Rapid block data routines
* - acquires data (user sets trigger mode before calling), displays 10 items
*   and saves all to data.txt
* Input :
* - unit : the unit to use.
* - text : the text to display before the display of data slice
* - offset : the offset into the data buffer to start the display's slice.
****************************************************************************/
int RapidBlockDataHandler(UNIT_MODEL * unit, char * text, int offset)
{
	int i, j;
	long timeInterval;
	unsigned long sampleCount = 50000;
	FILE * fp;
	long maxSamples;
	short * buffers[PS4000_MAX_CHANNEL_BUFFERS * 2];
	long timeIndisposed;
	long nMaxSamples;
	unsigned long segmentIndex;
	unsigned long noOfSamples;
	PICO_STATUS status;

/*  find the maximum number of samples, the time interval (in timeUnits),
	*		 the most suitable time units, and the maximum oversample at the current timebase*/
	while (ps4000GetTimebase(	unit->handle, timebase, sampleCount, &timeInterval, oversample,	&maxSamples, 0))
	{
		timebase++;
	}
	printf("Rapid Block mode with aggregation:- timebase: %lu\toversample:%hd\n", timebase, oversample);

	// Set the memory segments (must be euqal or more than no of waveforms)
	ps4000MemorySegments(unit->handle, 100, &nMaxSamples);

	// smapleCount must be < nMaxSamples
	sampleCount = 20000;
	printf("Rapid Block Mode with aggregation: memory Max smaples = %ld \n", nMaxSamples);

	// Set the number of waveforms to 100
	ps4000SetNoOfCaptures(unit->handle, 100);

	/* Start it collecting, then wait for completion*/
	g_ready = FALSE;

	status = ps4000RunBlock(unit->handle, 0, sampleCount, timebase, oversample,
													&timeIndisposed, 0, CallBackBlock, NULL);

	printf("RapidBlockDataHandler::Run Block : %i\n", status);
	printf("Waiting for trigger...Press a key to abort\n");

	while (!g_ready && !_kbhit())
	{
		Sleep(0);
	}

	for (i = 0; i < unit->ChannelCount; i++) 
	{
		buffers[i * 2] = (short*)malloc(sampleCount * sizeof(short));
		buffers[i * 2 + 1] = (short*)malloc(sampleCount * sizeof(short));
		ps4000SetDataBuffers(unit->handle, (short)i, buffers[i * 2], buffers[i * 2 + 1], sampleCount);
	}

	ps4000Stop(unit->handle);

	if (g_ready) 
	{
		// retrieve data of 10 Segments 
		// one segment is a bulk of data of one waveform
		// and printf out the first 10 readings
		fp = fopen("data.txt", "w");

		noOfSamples = sampleCount;

		for (segmentIndex = 80; segmentIndex < 90; segmentIndex ++)
		{
			printf("\nRapid Block Mode with aggregation: Reading Segement:-- %lu \n", segmentIndex);
			// Get values of 
			sampleCount = noOfSamples;
			status = ps4000GetValues(unit->handle, 0, (unsigned long*) &sampleCount, 1, RATIO_MODE_NONE, segmentIndex, NULL);
			printf("\nRapid Block Mode with aggregation: Reading Segement:-- ps4000GetValues: %i \n", status);
			/* Print out the first 10 readings, converting the readings to mV if required */
			printf(text);
			printf("Value (%s)\n", ( scaleVoltages ) ? ("mV") : ("ADC Counts"));

			for (i = offset; i < offset+10; i++) 
			{
				for (j = 0; j < unit->ChannelCount; j++) 
				{
					if (unit->channelSettings[j].enabled) 
					{
						printf("%d\t",adc_to_mv(buffers[j * 2][i],unit->channelSettings[PS4000_CHANNEL_A + j].range));
					}
				}
				printf("\n");
			}

			sampleCount = __min(sampleCount, BUFFER_SIZE);
			if (fp != NULL)
			{
				for (i = 0; i < sampleCount; i++) 
				{
					for (j = 0; j < unit->ChannelCount; j++) 
					{
						fprintf(fp, "%lld ", g_times[j] + (long long)(i * timeInterval));
						if (unit->channelSettings[j].enabled) 
						{
							fprintf(fp,", %d, %d", 
								buffers[j * 2][i], 
								adc_to_mv(buffers[j * 2][i], 
								unit->channelSettings[PS4000_CHANNEL_A + j].range));
						}
					}
					fprintf(fp, "\n");
				}
			}
			else
				printf("Cannot open the file data.txt for writing. \nPlease ensure that you have permission to access. \n");
		}
	} 
	else 
	{
		printf("data collection aborted\n");
		_getch();
	}

	if (fp != NULL)
		fclose(fp);

	for (i = 0; i < unit->ChannelCount * 2; i++) 
	{
		free(buffers[i]);
	}
	return 1;
}

/****************************************************************************
* No_Agg_RapidBlockDataHandler
* - Used by Rapid block data routines
* - acquires data (user sets trigger mode before calling), displays 10 items
*   and saves all to data.txt
* Input :
* - unit : the unit to use.
* - text : the text to display before the display of data slice
* - offset : the offset into the data buffer to start the display's slice.
****************************************************************************/
int No_Agg_RapidBlockDataHandler(UNIT_MODEL * unit, char * text, int offset)
{
	int i, j;
	long timeInterval;
	long sampleCount = 50000;
	FILE * fp;
	long maxSamples;
	short * buffers[PS4000_MAX_CHANNEL_BUFFERS][SEGMEM];
	short overflow[SEGMEM] = {1,1,1,1,1,1,1,1,1,1};
	long timeIndisposed;
	long nMaxSamples;
	unsigned long segmentIndex;
	unsigned long noOfSamples;
	PICO_STATUS status;


	/*  find the maximum number of samples, the time interval (in timeUnits),
	*		 the most suitable time units, and the maximum oversample at the current timebase*/
	while (ps4000GetTimebase(unit->handle, timebase, sampleCount,	&timeInterval, oversample, &maxSamples, 0))
	{
		timebase++;
	}
	printf("Rapid Block mode without aggregation:- timebase: %lu\toversample:%hd\n", timebase, oversample);


	// Set the memory segments (must be euqal or more than no of waveforms)
	ps4000MemorySegments(unit->handle, 100, &nMaxSamples);

	// smapleCount must be < nMaxSamples
	sampleCount = 50000;
	printf("Rapid Block Mode without aggregation: memory Max smaples = %ld \n", nMaxSamples);

	// Set the number of waveforms to 100
	ps4000SetNoOfCaptures(unit->handle, 100);

	/* Start it collecting, then wait for completion*/
	g_ready = FALSE;

	// Manually set timebase 
	timebase = 5;

	status = ps4000RunBlock(unit->handle, 0, sampleCount, timebase, oversample, &timeIndisposed, 0, CallBackBlock, NULL);
	/*printf("Run Block : %i", status);*/
	printf("Waiting for trigger...Press a key to abort\n");

	while (!g_ready && !_kbhit())
	{
		Sleep(0);
	}

	// set the data buffer bulk
	// (SEGMEM)10 segments for each channel
	for (i = 0; i < unit->ChannelCount; i++) 
	{
		for (j = 0; j < SEGMEM; j ++)
		{
			buffers[i][j] = (short*)malloc(sampleCount * sizeof(short));
			ps4000SetDataBufferBulk(unit->handle, i, buffers[i][j], sampleCount, j);
		}
	}

	ps4000Stop(unit->handle);

	if (g_ready) 
	{
		// retrieve data of 10 Segments 
		// one segment is a bulk of data of one waveform
		// and printf out the first 10 readings
		fp = fopen("data.txt", "w");

		noOfSamples = sampleCount;

		ps4000GetValuesBulk(unit->handle, (long unsigned *) &sampleCount, 0// start segment index
			, 9 // end segment index (Please refer to Programmer's Guide for more details)
			, overflow);

		for (segmentIndex = 0; segmentIndex < 9; segmentIndex ++)
		{
			printf("\nRapid Block Mode without aggregation: Reading Segement:-- %lu \n", segmentIndex);
			// Get values of 
			sampleCount = noOfSamples;
			ps4000GetValues(unit->handle, 0, (unsigned long*) &sampleCount, 1, RATIO_MODE_NONE, segmentIndex, NULL);

			// Print out the first 10 readings, converting the readings to mV if required 
			printf(text);
			printf("Value (%s)\n", (scaleVoltages) ? ("mV") : ("ADC Counts"));

			for (i = offset; i < offset+10; i++) 
			{
				for (j = 0; j < unit->ChannelCount; j++) 
				{
					if (unit->channelSettings[j].enabled) 
					{
						printf("%d\t",adc_to_mv(buffers[j][segmentIndex][i],unit->channelSettings[PS4000_CHANNEL_A + j].range));
					}
				}
				printf("\n");
			}

			sampleCount = __min(sampleCount, BUFFER_SIZE);
			if (fp != NULL)
			{
				for (i = 0; i < sampleCount; i++) 
				{
					for (j = 0; j < unit->ChannelCount; j++) 
					{
						fprintf(fp, "%lld ", g_times[j] + (long long)(i * timeInterval));
						if (unit->channelSettings[j].enabled) 
						{
							fprintf(fp,", %d, %d", 
								buffers[j][segmentIndex][i], 
								adc_to_mv(buffers[j][segmentIndex][i], 
								unit->channelSettings[PS4000_CHANNEL_A + j].range));
						}
					}
					fprintf(fp, "\n");
				}
			}
			else
				printf("Cannot open the file data.txt for writing. \nPlease ensure that you have permission to access. \n");
		}
	} 
	else 
	{
		printf("data collection aborted\n");
		_getch();
	}

	if (fp != NULL)
		fclose(fp);

	for (i = 0; i < unit->ChannelCount; i++) 
	{
		for (j = 0; j < SEGMEM; j ++)
			free(buffers[i][j]);		
	}
	return 1;
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
void BlockDataHandler(UNIT_MODEL * unit, char * text, int offset)
{
	int i, j;
	long timeInterval;
	long sampleCount= BUFFER_SIZE;
	FILE * fp;
	long maxSamples;
	short * buffers[PS4000_MAX_CHANNEL_BUFFERS * 2];
	long timeIndisposed;
	PICO_STATUS status;

	for (i = 0; i < unit->ChannelCount; i++) 
	{
		buffers[i * 2] = (short*)malloc(sampleCount * sizeof(short));
		buffers[i * 2 + 1] = (short*)malloc(sampleCount * sizeof(short));
		status = ps4000SetDataBuffers(unit->handle, (short)i, buffers[i * 2], buffers[i * 2 + 1], sampleCount);

		printf("BlockDataHandler:ps4000SetDataBuffers(channel %d) ------ %d \n", i, status);
	}

	/*  find the maximum number of samples, the time interval (in timeUnits),
	*		 the most suitable time units, and the maximum oversample at the current timebase*/
	while (ps4000GetTimebase(unit->handle, timebase, sampleCount, &timeInterval, oversample, &maxSamples, 0))
	{
		timebase++;
	}

	timebase = 6;
	printf("timebase: %ld\toversample:%hd\n", timebase, oversample);

	/* Start it collecting, then wait for completion*/
	g_ready = FALSE;
	status = ps4000RunBlock(unit->handle, 0, sampleCount, timebase, oversample,	&timeIndisposed, 0, CallBackBlock, NULL);

	printf("BlockDataHandler:ps4000RunBlock ------ %i \n", status);
	printf("Waiting for trigger...Press a key to abort\n");

	while (!g_ready && !_kbhit())
	{
		Sleep(0);
	}

	if (g_ready) 
	{
		status = ps4000GetValues(unit->handle, 0, (unsigned long*) &sampleCount, 1, RATIO_MODE_NONE, 0, NULL);
		printf("BlockDataHandler:ps4000GetValues ------ %i \n", status);

		/* Print out the first 10 readings, converting the readings to mV if required */
		printf(text);
		printf("Value (%s)\n", ( scaleVoltages ) ? ("mV") : ("ADC Counts"));

		for (i = offset; i < offset+10; i++) 
		{
			for (j = 0; j < unit->ChannelCount; j++) 
			{
				if (unit->channelSettings[j].enabled) 
				{
					printf("%d\t", adc_to_mv(buffers[j * 2][i],	unit->channelSettings[PS4000_CHANNEL_A + j].range));
				}
			}
			printf("\n");
		}

		sampleCount = __min(sampleCount, BUFFER_SIZE);

		fp = fopen("data.txt", "w");
		if (fp != NULL)
		{
			for (i = 0; i < sampleCount; i++) 
			{
				for (j = 0; j < unit->ChannelCount; j++) 
				{
					fprintf(fp, "%lld ", g_times[j] + (long long)(i * timeInterval));
					if (unit->channelSettings[j].enabled) 
					{
						fprintf(fp,", %d, %d", buffers[j * 2][i], adc_to_mv(buffers[j * 2][i], 
										unit->channelSettings[PS4000_CHANNEL_A + j].range));
					}
				}
				fprintf(fp, "\n");
			}
		}
		else
			printf("Cannot open the file data.txt for writing. \nPlease ensure that you have permission to access. \n");
	} 
	else 
	{
		printf("data collection aborted\n");
		_getch();
	}

	status = ps4000Stop(unit->handle);
	printf("BlockDataHandler:ps4000Stop ------ %i \n", status);

	if (fp != NULL)
		fclose(fp);

	for (i = 0; i < unit->ChannelCount * 2; i++) 
	{
		free(buffers[i]);
	}
}

void StreamDataHandler(UNIT_MODEL * unit, unsigned long preTrigger)
{
	long i, j;
	unsigned long sampleCount= BUFFER_SIZE * 10; /*  *10 is to make sure buffer large enough */
	FILE * fp;
	short * buffers[PS4000_MAX_CHANNEL_BUFFERS];
	PICO_STATUS status;
	unsigned long sampleInterval = 1;
	int index = 0;
	int totalSamples;

	for (i = 0; i < unit->ChannelCount; i++) // create data buffers
	{
		buffers[i * 2] = (short*) malloc(sampleCount * sizeof(short));
		buffers[i * 2 + 1] = (short*)malloc(sampleCount * sizeof(short));
		ps4000SetDataBuffers(unit->handle, i, buffers[i * 2],
			buffers[i * 2 + 1], sampleCount);
	}

	printf("Waiting for trigger...Press a key to abort\n");
	g_autoStop = FALSE;
	status = ps4000RunStreaming(unit->handle, &sampleInterval, PS4000_US,
															preTrigger, 1000000 - preTrigger, TRUE, 1000,	sampleCount);

	printf("Streaming data...Press a key to abort\n");

	fp = fopen("data.txt", "w");

	totalSamples = 0;
	while (!_kbhit() && !g_autoStop)
	{
		/* Poll until data is received. Until then, GetStreamingLatestValues wont call the callback */
		Sleep(100);
		g_ready = FALSE;

		status = ps4000GetStreamingLatestValues(unit->handle,	CallBackStreaming, NULL);
		index ++;

		if (g_ready && g_sampleCount > 0) /* can be ready and have no data, if autoStop has fired */
		{
			totalSamples += g_sampleCount;
			printf("Collected %li samples, index = %lu, Total: %li samples\n", g_sampleCount,
							g_startIndex, totalSamples);

			if (fp != NULL)
			{
				for (i = g_startIndex; i < (long)(g_startIndex + g_sampleCount); i++) 
				{
					for (j = 0; j < unit->ChannelCount; j++) 
					{
						if (unit->channelSettings[j].enabled) 
						{
							fprintf(
								fp,
								"%d, %d, %d, %d,",
								buffers[j * 2][i],
								adc_to_mv(
								buffers[j * 2][i],
								unit->channelSettings[PS4000_CHANNEL_A + j].range),
								buffers[j * 2 + 1][i],
								adc_to_mv(
								buffers[j * 2 + 1][i],
								unit->channelSettings[PS4000_CHANNEL_A + j].range));
						}
					}
					fprintf(fp, "\n");
				}
			}
			else
				printf("Cannot open the file data.txt for writing. \nPlease ensure that you have permission to access. \n");
		}
	}

	if (fp != NULL)
		fclose(fp);	

	ps4000Stop(unit->handle);

	if (!g_autoStop) 
	{
		printf("data collection aborted\n");
		_getch();
	}

	for (i = 0; i < unit->ChannelCount * 2; i++) 
	{
		free(buffers[i]);
	}
}

PICO_STATUS SetTrigger(	short handle,
												struct tTriggerChannelProperties * channelProperties,
												short nChannelProperties,
												struct tTriggerConditions * triggerConditions,
												short nTriggerConditions,
												TRIGGER_DIRECTIONS * directions,
												struct tPwq * pwq,
												unsigned long delay,
												short auxOutputEnabled,
												long autoTriggerMs)
{
	PICO_STATUS status;

	if ((status = ps4000SetTriggerChannelProperties(handle,
																									channelProperties,
																									nChannelProperties,
																									auxOutputEnabled,
																									autoTriggerMs)) != PICO_OK)
	{

			printf("SetTrigger:ps4000SetTriggerChannelProperties ------ %d \n", status);
			return status;
	}

	if ((status = ps4000SetTriggerChannelConditions(handle,
																									triggerConditions,
																									nTriggerConditions)) != PICO_OK) 
	{
			printf("SetTrigger:ps4000SetTriggerChannelConditions ------ %d \n", status);
			return status;
	}

	if ((status = ps4000SetTriggerChannelDirections(handle,
																									directions->channelA,
																									directions->channelB,
																									directions->channelC,
																									directions->channelD,
																									directions->ext,
																									directions->aux)) != PICO_OK) 
	{
			printf("SetTrigger:ps4000SetTriggerChannelDirections ------ %d \n", status);
			return status;
	}

	if ((status = ps4000SetTriggerDelay(handle, delay)) != PICO_OK) 
	{
		printf("SetTrigger:ps4000SetTriggerDelay ------ %d \n", status);
		return status;
	}

	if((status = ps4000SetPulseWidthQualifier(handle, pwq->conditions, pwq->nConditions, 
																						pwq->direction, pwq->lower, pwq->upper, pwq->type)) != PICO_OK)
	{
		printf("SetTrigger:ps4000SetPulseWidthQualifier ------ %d \n", status);
		return status;
	}
	return status;
}

/****************************************************************************
* CollectBlockImmediate
*  this function demonstrates how to collect a single block of data
*  from the unit (start collecting immediately)
****************************************************************************/
void CollectBlockImmediate(UNIT_MODEL * unit)
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
* CollectRapidBlockImmediate
*  this function demonstrates how to rapidly collect a single block of data
*  from the unit (start collecting immediately)
****************************************************************************/
int CollectRapidBlockImmediate(UNIT_MODEL * unit)
{
	struct tPwq pulseWidth;
	struct tTriggerDirections directions;

	memset(&directions, 0, sizeof(struct tTriggerDirections));
	memset(&pulseWidth, 0, sizeof(struct tPwq));

	printf("Collect Rapid block immediate with aggregation...\n");
	printf("Press a key to start\n");
	_getch();

	SetDefaults(unit);

	/* Trigger disabled	*/
	SetTrigger(unit->handle, NULL, 0, NULL, 0, &directions, &pulseWidth, 0, 0, 0);


	return RapidBlockDataHandler(unit, "First 10 readings\n", 0);
}

/****************************************************************************
* CollectRapidBlock_No_Agg
*  this function demonstrates how to use rapid block mode without
*  aggregation
****************************************************************************/
int CollectRapidBlock_No_Agg(UNIT_MODEL * unit)
{
	struct tPwq pulseWidth;
	struct tTriggerDirections directions;

	memset(&directions, 0, sizeof(struct tTriggerDirections));
	memset(&pulseWidth, 0, sizeof(struct tPwq));

	printf("Collect Rapid block immediate without aggregation...\n");
	printf("Press a key to start\n");
	_getch();

	SetDefaults(unit);

	/* Trigger disabled	*/
	SetTrigger(unit->handle, NULL, 0, NULL, 0, &directions, &pulseWidth, 0, 0, 0);

	return No_Agg_RapidBlockDataHandler(unit, "First 10 readings\n", 0);
}


/****************************************************************************
* CollectBlockEts
*  this function demonstrates how to collect a block of
*  data using equivalent time sampling (ETS).
****************************************************************************/
void CollectBlockEts(UNIT_MODEL * unit)
{
	PICO_STATUS status;
	long ets_sampletime;
	short
		triggerVoltage = mv_to_adc(	100, unit->channelSettings[PS4000_CHANNEL_A].range); // ChannelInfo stores ADC counts
	
	struct tTriggerChannelProperties sourceDetails = {	triggerVoltage,
																											10,
																											triggerVoltage,
																											10,
																											PS4000_CHANNEL_A,
																											LEVEL };

	struct tTriggerConditions conditions = {	CONDITION_TRUE,
																						CONDITION_DONT_CARE,
																						CONDITION_DONT_CARE,
																						CONDITION_DONT_CARE,
																						CONDITION_DONT_CARE,
																						CONDITION_DONT_CARE,
																						CONDITION_DONT_CARE };
	
	unsigned long delay = 0;
	struct tPwq pulseWidth;
	struct tTriggerDirections directions;

	memset(&pulseWidth, 0, sizeof(struct tPwq));
	memset(&directions, 0, sizeof(struct tTriggerDirections));
	directions.channelA = RISING;

	printf("Collect ETS block...\n");
	printf("Collects when value rises past %dmV\n",
		adc_to_mv(sourceDetails.thresholdUpper,
		unit->channelSettings[PS4000_CHANNEL_A].range));
	printf("Press a key to start...\n");
	_getch();

	SetDefaults(unit);

	/* Trigger enabled
	* Rising edge
	* Threshold = 1500mV
	* 10% pre-trigger  (negative is pre-, positive is post-) */

	status = SetTrigger(unit->handle, &sourceDetails, 1, &conditions, 1,
		&directions, &pulseWidth, delay, 0, 0);

	/* printf("Set Trigger : %x" , status); */

	/* Enable ETS in fast mode, the computer will store 100 cycles but interleave only 10 */
	status = ps4000SetEts(unit->handle, PS4000_ETS_FAST, 20, 4, &ets_sampletime);

	/*printf("Set ETS : %x" , status);*/
	printf("ETS Sample Time is: %ld\n", ets_sampletime);

	BlockDataHandler(unit, "Ten readings after trigger\n", BUFFER_SIZE / 10 - 5); // 10% of data is pre-trigger
}

/****************************************************************************
* CollectBlockTriggered
*  this function demonstrates how to collect a single block of data from the
*  unit, when a trigger event occurs.
****************************************************************************/
void CollectBlockTriggered(UNIT_MODEL * unit)
{
	short	triggerVoltage = mv_to_adc(100,	unit->channelSettings[PS4000_CHANNEL_A].range); // ChannelInfo stores ADC counts

	struct tTriggerChannelProperties sourceDetails = {	triggerVoltage,
																											triggerVoltage,
																											256 * 10,
																											PS4000_CHANNEL_A,
																											LEVEL };

	struct tTriggerConditions conditions = {	CONDITION_TRUE,
																						CONDITION_DONT_CARE,
																						CONDITION_DONT_CARE,
																						CONDITION_DONT_CARE,
																						CONDITION_DONT_CARE,
																						CONDITION_DONT_CARE,
																						CONDITION_DONT_CARE };

	struct tPwq pulseWidth;
	
	struct tTriggerDirections directions = {	RISING,
																						NONE,
																						NONE,
																						NONE,
																						NONE,
																						NONE };
	
	memset(&pulseWidth, 0, sizeof(struct tPwq));

	printf("Collect block triggered...\n");
	printf("Collects when value rises past %dmV\n",
		adc_to_mv(sourceDetails.thresholdUpper,
		unit->channelSettings[PS4000_CHANNEL_A].range));
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
* Initialise unit' structure with Variant specific defaults
****************************************************************************/
void get_info(UNIT_MODEL * unit)
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
			status = ps4000GetUnitInfo(unit->handle, line, sizeof(line), &r, i);
			if (i == 3) 
			{
				variant = atoi(line);
			}
			printf("%s: %s\n", description[i], line);
		}

	switch (variant) 
	{
	case MODEL_PS4223:
		unit->model						= MODEL_PS4223;
		unit->signalGenerator = FALSE;
		unit->ETS							= FALSE;
		unit->firstRange			= PS4000_50MV;
		unit->lastRange				= PS4000_100V;
		unit->ChannelCount		= DUAL_SCOPE;
		break;

	case MODEL_PS4224:
		unit->model						= MODEL_PS4224;
		unit->signalGenerator = FALSE;
		unit->ETS							= FALSE;
		unit->firstRange			= PS4000_50MV;
		unit->lastRange				= PS4000_20V;
		unit->ChannelCount		= DUAL_SCOPE;
		break;

	case MODEL_PS4423:
		unit->model						= MODEL_PS4423;
		unit->signalGenerator = FALSE;
		unit->ETS							= FALSE;
		unit->firstRange			= PS4000_50MV;
		unit->lastRange				= PS4000_100V;
		unit->ChannelCount		= QUAD_SCOPE;
		break;

	case MODEL_PS4424:
		unit->model						= MODEL_PS4424;
		unit->signalGenerator = FALSE;
		unit->ETS							= FALSE;
		unit->firstRange			= PS4000_50MV;
		unit->lastRange				= PS4000_20V;
		unit->ChannelCount		= QUAD_SCOPE;
		break;

	case MODEL_PS4226:
		unit->model						= MODEL_PS4226;
		unit->signalGenerator = TRUE;
		unit->ETS							= TRUE;
		unit->firstRange			= PS4000_50MV;
		unit->lastRange				= PS4000_20V;
		unit->ChannelCount		= DUAL_SCOPE;
		break;

	case MODEL_PS4227:
		unit->model						= MODEL_PS4227;
		unit->signalGenerator = TRUE;
		unit->ETS							= TRUE;
		unit->firstRange			= PS4000_50MV;
		unit->lastRange				= PS4000_20V;
		unit->ChannelCount		= DUAL_SCOPE;
		break;

	default:
		break;
		}
	}
}

/****************************************************************************
* Select input voltage ranges for channels A and B
****************************************************************************/
void set_voltages(UNIT_MODEL * unit)
{
	int i;
	int ch;

	/* See what ranges are available... */
	for (i = unit->firstRange; i <= unit->lastRange; i++) 
	{
		printf("%d -> %d mV\n", i, inputRanges[i]);
	}

	/* Ask the user to select a range */
	printf("Specify voltage range (%d..%d)\n", unit->firstRange, unit->lastRange);
	printf("99 - switches channel off\n");
	
	for (ch = 0; ch < unit->ChannelCount; ch++) 
	{
		printf("\n");
		do 
		{
			printf("Channel %c: ", 'A' + ch);
			fflush(stdin);
			scanf("%hd", &unit->channelSettings[ch].range);
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
void SetTimebase(UNIT_MODEL unit)
{
	long timeInterval;
	long maxSamples;
	PICO_STATUS status;

	do 
	{
		printf("Specify timebase: ");

		fflush(stdin);
		scanf("%lud", &timebase);
		status = ps4000GetTimebase(unit.handle, timebase, BUFFER_SIZE, &timeInterval, 1, &maxSamples, 0);

		if(status == PICO_INVALID_TIMEBASE)
			printf("Invalid timebase\n\n");
	
	} while (status == PICO_INVALID_TIMEBASE);
	
	printf("Timebase %lu - %ld ns\n", timebase, timeInterval);
	oversample = TRUE;
}

/****************************************************************************
* Toggles the signal generator
* - allows user to set frequency and waveform
* - allows for custom waveform (values 0..4192) of up to 8192 samples long
***************************************************************************/
void SetSignalGenerator(UNIT_MODEL unit)
{
	PICO_STATUS status;
	short waveform;
	long	frequency;
	char	fileName [128];
	FILE	* fp;
	short arbitraryWaveform [8192];
	short waveformSize = 0;

	memset(&arbitraryWaveform, 0, 8192);

	printf("Enter frequency in Hz: "); // Ask user to enter signal frequency;
	do 
	{
		scanf("%lu", &frequency);
	} while (frequency <= 0 || frequency > 10000000);

	if (frequency > 0) // Ask user to enter type of signal
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
			scanf("%hd", &waveform);
		} while (waveform != 99 && (waveform < 0 || waveform >= MAX_WAVE_TYPES));

		if (waveform == 99) // custom waveform selected - user needs to select file
		{
			waveformSize = 0;

			printf("Select a waveform file to load: ");
			scanf("%s", fileName);
			if ((fp = fopen(fileName, "r"))) // Having opened file, read in data - one number per line (at most 8192 lines), with values in (0..4095)
			{ 
				while (EOF != fscanf(fp, "%hi", (arbitraryWaveform + waveformSize))&& waveformSize++ < 8192);

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
		status = ps4000SetSigGenArbitrary(unit.handle, 
																			0, 
																			4000000, 
																			(unsigned long)delta, 
																			(unsigned long)delta, 
																			0, 
																			0, 
																			arbitraryWaveform, 
																			waveformSize, 
																			0,
																			0, 
																			SINGLE, 
																			0, 
																			0, 
																			SIGGEN_RISING,
																			SIGGEN_NONE, 
																			0);

		printf("Status of Arbitrary Gen: %x \n", (unsigned int)status);
	} 
	else 
	{
		status = ps4000SetSigGenBuiltIn(unit.handle, 
																		0, 
																		4000000, 
																		waveform, 
																		(float)frequency, 
																		(float)frequency, 
																		0, 0, 0, 0, 0, 0, 0, 0, 0);
	}
	/* printf("Set Signal Generator : %d\n", status); */
}

/****************************************************************************
* Signal generator 
* External in signals as triggers
* generate 3 shots after triggering occures
* - allows user to set frequency and arbitrary waveform
* - allows for custom waveform (values 0..4192) of up to 8192 samples long
***************************************************************************/
void SetExtSignalGenerator(UNIT_MODEL unit)
{
	PICO_STATUS status;
	long frequency;
	char fileName [128];
	FILE * fp;
	short arbitraryWaveform [8192];
	short waveformSize = 0;

	memset(&arbitraryWaveform, 0, 8192);

	printf("Enter frequency in Hz: "); // Ask user to enter signal frequency;
	do 
	{
		scanf("%lu", &frequency);
	} while (frequency <= 0 || frequency > 10000000);

	if (frequency > 0) // Ask user to enter type of signal
	{
		printf("Signal generator + External triggering On\n");
		waveformSize = 0;

		printf("Select a waveform file to load: ");
		scanf("%s", fileName);
		if((fp = fopen(fileName, "r"))) // Having opened file, read in data - one number per line (at most 8192 lines), with values in (0..4095)
		{ 
			while (EOF != fscanf(fp, "%hi", (arbitraryWaveform + waveformSize))&& waveformSize++ < 8192);
			fclose(fp);
			printf("File successfully loaded\n");
		} 
		else 
		{
			printf("Invalid filename\n");
			return;
		}
	} 
	else 
	{
		waveformSize = 0;
		printf("Signal generator + External triggering Off");
	}

	if (waveformSize > 0) 
	{
		double delta = ((frequency * waveformSize) / 8192.0) * 4294967296.0 * 8e-9; // delta >= 10

		// SINGLE, 3 shot, 'sweep' -off, rising edge, external triggering source, threshold -- 1.5V
		// more details refer to Programmer's Guide
		status = ps4000SetSigGenArbitrary(unit.handle, 0, 4000000, (unsigned long)delta, (unsigned long)delta, 0, 0, arbitraryWaveform, 						waveformSize, 0, 0, SINGLE, 
			3, /*shots -- how many shots generated after triggering*/
			0, 
			SIGGEN_RISING, SIGGEN_EXT_IN /*set external signal as triggering source*/, 
			2458 /* triggering threshold -- adc count*/);
		printf("Status of Arbitrary Gen: %x \n", (unsigned int)status);

	}
	/* printf("Set Signal Generator : %d\n", status); */
}


/****************************************************************************
* Stream Data Handler
* - Used by the two stream data examples - untriggered and triggered
* Inputs:
* - unit - the unit to sample on
* - preTrigger - the number of samples in the pre-trigger phase 
*					(0 if no trigger has been set)
***************************************************************************/
/****************************************************************************
* CollectStreamingImmediate
*  this function demonstrates how to collect a stream of data
*  from the unit (start collecting immediately)
***************************************************************************/
void CollectStreamingImmediate(UNIT_MODEL * unit)
{
	struct tPwq pulseWidth;
	struct tTriggerDirections directions;

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
void CollectStreamingTriggered(UNIT_MODEL * unit)
{
	short	triggerVoltage = mv_to_adc(100, unit->channelSettings[PS4000_CHANNEL_A].range); // ChannelInfo stores ADC counts

	struct tTriggerChannelProperties sourceDetails = {	triggerVoltage,
																											triggerVoltage,
																											256 * 10,
																											PS4000_CHANNEL_A,
																											LEVEL };

	struct tTriggerConditions conditions = {	CONDITION_TRUE,
																						CONDITION_DONT_CARE,
																						CONDITION_DONT_CARE,
																						CONDITION_DONT_CARE,
																						CONDITION_DONT_CARE,
																						CONDITION_DONT_CARE,
																						CONDITION_DONT_CARE };

	struct tPwq pulseWidth;
	
	struct tTriggerDirections directions = {	RISING,
																						NONE,
																						NONE,
																						NONE,
																						NONE,
																						NONE };			
	
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
	UNIT_MODEL unit;
	struct tPwq pulseWidth;
	struct tTriggerDirections directions;

	printf("PS4000 driver example program\n");
	printf("Version 1.1\n\n");
	printf("\n\nOpening the device...\n");

	status = ps4000OpenUnit(&(unit.handle));
	printf("Handle: %d\n", unit.handle);
	
	if (status != PICO_OK && status != PICO_EEPROM_CORRUPT) 
	{
		printf("Unable to open device\n");
		printf("Error code : %d\n", (int)status);
		while (!_kbhit());
		exit( 99); // exit program - nothing after this executes
	}

	printf("Device opened successfully, cycle %d\n\n", ++cycles);

	// setup devices
	get_info(&unit);
	timebase = 1;

	for (i = 0; i < MAX_CHANNELS; i++) 
	{
		unit.channelSettings[i].enabled = TRUE;
		unit.channelSettings[i].DCcoupled = TRUE;
		unit.channelSettings[i].range = PS4000_5V;
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
		printf("B - immediate block             V - Set voltages\n");
		printf("T - triggered block             I - Set timebase\n");
		printf("F - toggle signal generator on/off\n");
		printf("S - immediate streaming         A - ADC counts/mV\n");
		printf("R - immdeiate rapid block with aggreation\n");
		printf("N - rapid block without aggreation\n");
		printf("Q - collect a block using ETS\n");
		printf("W - triggered streaming\n");
		printf("G - Signal generator\n");
		printf("E - External signal generator\n");
		printf("                                X - exit\n");
		printf("Operation:");

		ch = toupper(_getch());

		printf("\n\n");
		switch (ch) 
		{
		case 'B':
			CollectBlockImmediate(&unit);
			break;

		case 'R':
			CollectRapidBlockImmediate(&unit);
			break;

		case 'N':
			CollectRapidBlock_No_Agg(&unit);
			break;

		case 'T':
			CollectBlockTriggered(&unit);
			break;

		case 'S':
			CollectStreamingImmediate(&unit);
			break;

		case 'W':
			CollectStreamingTriggered(&unit);
			break;

		case 'Q':
			if(unit.ETS == FALSE)
			{
				printf("This model does not have ETS\n\n");
				break;
			}

			CollectBlockEts(&unit);
			break;

		case 'G':
			if(unit.signalGenerator == FALSE)
			{
				printf("This model does not have a signal generator\n\n");
				break;
			}

			SetSignalGenerator(unit);
			break;

		case 'E':
			if(unit.signalGenerator == FALSE)
			{
				printf("This model does not have a signal generator\n\n");
				break;
			}

			SetExtSignalGenerator(unit);
			break;

		case 'V':
			set_voltages(&unit);
			break;

		case 'I':
			SetTimebase(unit);
			break;

		case 'A':
			scaleVoltages = !scaleVoltages;
			if (scaleVoltages) 
			{
				printf("Readings will be scaled in mV\n");
			} 
			else 
			{
				printf("Readings will be scaled in ADC counts\n");
			}
			break;

		case 'X':
			break;

		default:
			printf("Invalid operation\n");
			break;
		}
	}
	
	ps4000CloseUnit(unit.handle);
	return 1;
}
