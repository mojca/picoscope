#include <iostream>
#include <string>
#include <conio.h>
#include "windows.h"
#include "picoscope.h"

#include "ps4000Api.h"
#include "ps6000Api.h"
#include "picoStatus.h"

// using namespace std;

bool Picoscope::var_is_ready;

/* constructor */
Picoscope::Picoscope(PICO_SERIES s) {
	int i;

	series  = s;
	var_is_open  = false;
	SetReady(false);
	handle = PICOSCOPE_HANDLE_UNITIALIZED;
	return_status = PICO_OK;

//	EnableChannels(false,false,false,false);
	// should perhaps already the constructor open the picoscope?
	// open();
	// make sure that all the values are initialized to NULL
	// for(i=0; i<PICOSCOPE_N_CHANNELS; i++) {
	// 	data[i]=NULL;
	// }
}

// /* constructor */
// Picoscope4000::Picoscope4000() {
// 	Picoscope(4000);
// }
// /* constructor */
// Picoscope6000::Picoscope6000() {
// 	Picoscope(6000);
// }


/* destructor */
Picoscope::~Picoscope() {
	// close();
	int i;
	for(i=0; i<PICOSCOPE_N_CHANNELS; i++) {
		// if(data[i]!=NULL) {
		// 	// delete [] data[i];
		// }
	}
	// delete [] data; - if it was not initialized
}

PICO_STATUS Picoscope::Open() {
	// previous operation has failed: do we really want to try to proceed? (maybe we could remove this statement)
	if(return_status != PICO_OK) {
		throw PicoscopeException(return_status);
	}
	// first check if Picoscope has already been opened
	if(var_is_open == true) {
		// TODO: this doesn't need to be fatal ...
		throw PicoscopeUserException("Are you trying to open an already opened Picoscope?");
		// TODO: this is wrong!!! (but the code doesn't come that far anyway)
		return_status = PICO_OK;
	} else {
		// finally: open the unit
		if(GetSeries() == PICO_4000) {
			std::cerr << "Open Picoscope 4000 ... ";
			return_status = ps4000OpenUnit(&handle);
		} else {
			std::cerr << "Open Picoscope 6000 ... ";
			return_status = ps6000OpenUnit(&handle, NULL);
		}
	}

	// if unit has been opened successfully ...
	if(return_status == PICO_OK) {
		// if proper handle has been assigned ...
		if(handle > 0) {
			var_is_open = true;
		} else {
			throw PicoscopeUserException("This should not happen. Picoscope reports to open successfully, but there is no proper handle returned. Please report to developers.");
		}
	// else report the problem
	} else {
		throw PicoscopeException(return_status);
	}
	// throw PicoscopeUserException("this is not supposed to happen (just testing).");

	std::cerr << "OK\n";
	return return_status;
}

PICO_STATUS Picoscope::Close()
{
	std::cerr << "Close Picoscope ... ";

	// previous operation has failed: do we really want to try to proceed? (maybe we could remove this statement)
	if(return_status != PICO_OK) {
		throw PicoscopeException(return_status);
	}
	if(var_is_open == false) {
		throw PicoscopeUserException("Are you trying to close an already closed Picoscope?");
	}
	if(handle <= 0) {
		throw PicoscopeUserException("This should not happen. Picoscope should be open, but the handle is not valid. Please report to developers.");
	}

	if(GetSeries() == PICO_4000) {
		return_status = ps4000Stop(handle);
	} else {
		return_status = ps6000Stop(handle);
	}

	if(return_status == PICO_OK) {
		var_is_open = false;
	} else {
		throw PicoscopeException(return_status);
	}

	std::cerr << "OK\n";
	return return_status;
}

void Picoscope::SetStatus(PICO_STATUS status)
{
	return_status = status;
}


void CALLBACK CallBackBlock (short handle, PICO_STATUS status, void *pParameter)
{
	// flag to say done reading data
	Picoscope::SetReady(true);
}

// void CALLBACK Picoscope::CallBackBlock (short handle, PICO_STATUS status, void *pParameter)
// {
// 	// flag to say done reading data
// 	var_is_ready = true;
// }


void Picoscope::MyFunction(unsigned long trace_length)
{
	int i;
	unsigned long max_samples;
	long time_in_ms = 0;
	int segment = 0;

	/* ps6000SetChannel */
	// handle, channel, enabled, coupling (TRUE: DC, FALSE: AC), range (for voltage; 8 = 5V), analogueOffset, bandwidth
	// for(i=0; i<PICOSCOPE_N_CHANNELS; i++) {
	// 	return_status = ps6000SetChannel(handle, (PS4000_CHANNEL)i, channels[i].IsEnabled(), TRUE, PS6000_5V, 0, PS6000_BW_FULL);
	// 	if(return_status != PICO_OK) {
	// 		throw PicoscopeException(return_status);
	// 	}
	// }
	return_status = ps6000SetChannel(handle, (PS6000_CHANNEL)0, TRUE,  PS6000_DC_1M, PS6000_5V, 0, PS6000_BW_FULL);
	return_status = ps6000SetChannel(handle, (PS6000_CHANNEL)1, FALSE, PS6000_DC_1M, PS6000_5V, 0, PS6000_BW_FULL);
	return_status = ps6000SetChannel(handle, (PS6000_CHANNEL)2, FALSE, PS6000_DC_1M, PS6000_5V, 0, PS6000_BW_FULL);
	return_status = ps6000SetChannel(handle, (PS6000_CHANNEL)3, FALSE, PS6000_DC_1M, PS6000_5V, 0, PS6000_BW_FULL);
	if(return_status != PICO_OK) {
		throw PicoscopeException(return_status);
	}

	#define PS6000_TIMEBASE_5GS    0UL
	#define PS6000_TIMEBASE_2_5GS  1UL
	#define PS6000_TIMEBASE_1_25GS 2UL

	#define PS6000_TIMEBASE_200ps  0UL
	#define PS6000_TIMEBASE_400ps  1UL
	#define PS6000_TIMEBASE_800ps  2UL


	#define PS6000_TIMEBASE PS6000_TIMEBASE_5GS

	printf("get timebase\n");
	return_status = ps6000GetTimebase(handle, PS6000_TIMEBASE, trace_length, NULL, 1, &max_samples, segment);
	printf("max samples: %lu, trace length: %lu\n", max_samples, trace_length);

	if(return_status != PICO_OK) {
		throw PicoscopeException(return_status);
	}

	// TODO: check if everything was ok
	short *data = new short[trace_length];
	// data[i] = new short[1000000000];
	// std::cerr << "size of data(i):" << sizeof(data[i]) << std::endl;
	// fprintf(stderr, "> setting data buffers\n");
	// handle, channel, short *buffer, long buffer_length
	printf("set data buffer (length of buffer: %ld %ld)\n", sizeof(data), sizeof(data[0]));
	return_status = ps6000SetDataBuffer(handle, (PS6000_CHANNEL)0, data, trace_length, PS6000_RATIO_MODE_NONE);
	if(return_status != PICO_OK) {
		throw PicoscopeException(return_status);
	}

	Picoscope::SetReady(false);
	printf("run block\n");
	return_status = ps6000RunBlock(handle, 0, trace_length, PS6000_TIMEBASE, 1, &time_in_ms, segment, CallBackBlock, NULL);
	printf("time in ms: %ld\n", time_in_ms);
	if(return_status != PICO_OK) {
		throw PicoscopeException(return_status);
	}
	while (!Picoscope::IsReady() && !_kbhit()) {
		Sleep(0);
	}
	unsigned long N_of_samples;
	short overflow;
	printf("get values\n");
	N_of_samples = trace_length;
	return_status = ps6000GetValues(handle, 0, &N_of_samples, 1, PS6000_RATIO_MODE_NONE, segment, &overflow);
	printf("end of get values\n  number of samples: %lu\n", N_of_samples);
	if(return_status != PICO_OK) {
		printf("unable to get values\n");
		throw PicoscopeException(return_status);
	}
	printf("write data\n");
	FILE *g = fopen("data.txt", "wt");
	for(i=0; i<N_of_samples; i++) {
		fprintf(g, "%d\n", data[i]);
	}
	fclose(g);
	delete [] data;
}

void Picoscope::DoSomething(unsigned long trace_length) {
	// int i;
	// /* ps4000SetChannel */
	// // handle, channel, enabled, coupling (TRUE: DC, FALSE: AC), range (for voltage; 8 = 5V)
	// for(i=0; i<PICOSCOPE_N_CHANNELS; i++) {
	// 	return_status = ps4000SetChannel(handle, (PS4000_CHANNEL)i, channels[i].IsEnabled(), TRUE, PS4000_5V);
	// 	if(return_status != PICO_OK) {
	// 		throw PicoscopeException(return_status);
	// 	}
	// }
	// 
	// #define PS4000_TIMEBASE_80MS 0UL
	// #define PS4000_TIMEBASE_40MS 1UL
	// #define PS4000_TIMEBASE_20MS 2UL
	// #define PS4000_TIMEBASE_12_5ns 0UL
	// #define PS4000_TIMEBASE_25ns   1UL
	// #define PS4000_TIMEBASE_50ns   2UL
	// 
	// #define PS4000_TIMEBASE PS4000_TIMEBASE_80MS
	// 
	// 
	// /* ps4000GetTimebase */
	// // handle, timebase, noSomples, *timeIntevalNanoseconds (NULL), oversample, *maxSamples (NULL), segmentIndex
	// long max_samples, time_in_ms;
	// int segment = 0;
	// return_status = ps4000GetTimebase(handle, PS4000_TIMEBASE, trace_length, NULL, 1, &max_samples, segment);
	// // fprintf(stderr, "max samples: %d\n", max_samples);
	// if(return_status != PICO_OK) {
	// 	throw PicoscopeException(return_status);
	// }
	// 
	// // handle, channel, short *buffer, long, buffer_length
	// for(i=0; i<4; i+=1) {
	// 	// TODO: check if everything was ok
	// 	data[i] = new short[trace_length];
	// 	// data[i] = new short[1000000000];
	// 	// std::cerr << "size of data(i):" << sizeof(data[i]) << std::endl;
	// 	// fprintf(stderr, "> setting data buffers\n");
	// 	// handle, channel, short *buffer, long buffer_length
	// 	return_status = ps4000SetDataBuffer(handle, (PS4000_CHANNEL)i, data[i], trace_length);
	// 	if(return_status != PICO_OK) {
	// 		throw PicoscopeException(return_status);
	// 	}
	// }
	// var_is_ready = false;
	// global_is_ready = false;
	// return_status = ps4000RunBlock(handle, 0, trace_length, PS4000_TIMEBASE, 1, &time_in_ms, segment, CallBackBlock, NULL);
	// if(return_status != PICO_OK) {
	// 	throw PicoscopeException(return_status);
	// }
	// while (!global_is_ready && !_kbhit()) {
	// 	Sleep(0);
	// }
	// unsigned long N_of_samples;
	// short overflow;
	// return_status = ps4000GetValues(handle, 0, &N_of_samples, 1, RATIO_MODE_NONE, segment, &overflow);
	// if(return_status != PICO_OK) {
	// 	throw PicoscopeException(return_status);
	// }
}

short** Picoscope::GetData() {
	// return data;
	return NULL;
}

// int Picoscope::GetNumberOfHitsAboveThreshold(double value, )
// {
// 	return 0;
// }

/* temporary: remove after testing is done */
void Picoscope::do_something_nasty () {
	throw PicoscopeException(PICO_KERNEL_DRIVER_TOO_OLD);
}

/**********************************************************************
 * Handling Exceptions
 *********************************************************************/
// Picoscope::PicoscopeUserException(const  message)
// {
// 	errorNumber  = PICO_OTHER_ERROR;
// 	errorMessage = message;
// }

const char* Picoscope::PicoscopeUserException::GetErrorMessage() const
{
	return errorMessage;
}

PICO_STATUS Picoscope::PicoscopeException::GetErrorNumber() const
{
	return errorNumber;
}

const char* Picoscope::PicoscopeException::GetErrorMessage() const
{
	// #define PICO_OK0x00000000UL
	switch(errorNumber) {
		case PICO_OK                               : return "PICO_OK"                               ; // 0x00000000UL
		case PICO_MAX_UNITS_OPENED                 : return "PICO_MAX_UNITS_OPENED"                 ; // 0x00000001UL
		case PICO_MEMORY_FAIL                      : return "PICO_MEMORY_FAIL"                      ; // 0x00000002UL
		case PICO_NOT_FOUND                        : return "PICO_NOT_FOUND"                        ; // 0x00000003UL
		case PICO_FW_FAIL                          : return "PICO_FW_FAIL"                          ; // 0x00000004UL
		case PICO_OPEN_OPERATION_IN_PROGRESS       : return "PICO_OPEN_OPERATION_IN_PROGRESS"       ; // 0x00000005UL
		case PICO_OPERATION_FAILED                 : return "PICO_OPERATION_FAILED"                 ; // 0x00000006UL
		case PICO_NOT_RESPONDING                   : return "PICO_NOT_RESPONDING"                   ; // 0x00000007UL
		case PICO_CONFIG_FAIL                      : return "PICO_CONFIG_FAIL"                      ; // 0x00000008UL
		case PICO_KERNEL_DRIVER_TOO_OLD            : return "PICO_KERNEL_DRIVER_TOO_OLD"            ; // 0x00000009UL
		case PICO_EEPROM_CORRUPT                   : return "PICO_EEPROM_CORRUPT"                   ; // 0x0000000AUL
		case PICO_OS_NOT_SUPPORTED                 : return "PICO_OS_NOT_SUPPORTED"                 ; // 0x0000000BUL
		case PICO_INVALID_HANDLE                   : return "PICO_INVALID_HANDLE"                   ; // 0x0000000CUL
		case PICO_INVALID_PARAMETER                : return "PICO_INVALID_PARAMETER"                ; // 0x0000000DUL
		case PICO_INVALID_TIMEBASE                 : return "PICO_INVALID_TIMEBASE"                 ; // 0x0000000EUL
		case PICO_INVALID_VOLTAGE_RANGE            : return "PICO_INVALID_VOLTAGE_RANGE"            ; // 0x0000000FUL
		case PICO_INVALID_CHANNEL                  : return "PICO_INVALID_CHANNEL"                  ; // 0x00000010UL
		case PICO_INVALID_TRIGGER_CHANNEL          : return "PICO_INVALID_TRIGGER_CHANNEL"          ; // 0x00000011UL
		case PICO_INVALID_CONDITION_CHANNEL        : return "PICO_INVALID_CONDITION_CHANNEL"        ; // 0x00000012UL
		case PICO_NO_SIGNAL_GENERATOR              : return "PICO_NO_SIGNAL_GENERATOR"              ; // 0x00000013UL
		case PICO_STREAMING_FAILED                 : return "PICO_STREAMING_FAILED"                 ; // 0x00000014UL
		case PICO_BLOCK_MODE_FAILED                : return "PICO_BLOCK_MODE_FAILED"                ; // 0x00000015UL
		case PICO_NULL_PARAMETER                   : return "PICO_NULL_PARAMETER"                   ; // 0x00000016UL
		case PICO_ETS_MODE_SET                     : return "PICO_ETS_MODE_SET"                     ; // 0x00000017UL
		case PICO_DATA_NOT_AVAILABLE               : return "PICO_DATA_NOT_AVAILABLE"               ; // 0x00000018UL
		case PICO_STRING_BUFFER_TO_SMALL           : return "PICO_STRING_BUFFER_TO_SMALL"           ; // 0x00000019UL
		case PICO_ETS_NOT_SUPPORTED                : return "PICO_ETS_NOT_SUPPORTED"                ; // 0x0000001AUL
		case PICO_AUTO_TRIGGER_TIME_TO_SHORT       : return "PICO_AUTO_TRIGGER_TIME_TO_SHORT"       ; // 0x0000001BUL
		case PICO_BUFFER_STALL                     : return "PICO_BUFFER_STALL"                     ; // 0x0000001CUL
		case PICO_TOO_MANY_SAMPLES                 : return "PICO_TOO_MANY_SAMPLES"                 ; // 0x0000001DUL
		case PICO_TOO_MANY_SEGMENTS                : return "PICO_TOO_MANY_SEGMENTS"                ; // 0x0000001EUL
		case PICO_PULSE_WIDTH_QUALIFIER            : return "PICO_PULSE_WIDTH_QUALIFIER"            ; // 0x0000001FUL
		case PICO_DELAY                            : return "PICO_DELAY"                            ; // 0x00000020UL
		case PICO_SOURCE_DETAILS                   : return "PICO_SOURCE_DETAILS"                   ; // 0x00000021UL
		case PICO_CONDITIONS                       : return "PICO_CONDITIONS"                       ; // 0x00000022UL
		case PICO_USER_CALLBACK                    : return "PICO_USER_CALLBACK"                    ; // 0x00000023UL
		case PICO_DEVICE_SAMPLING                  : return "PICO_DEVICE_SAMPLING"                  ; // 0x00000024UL
		case PICO_NO_SAMPLES_AVAILABLE             : return "PICO_NO_SAMPLES_AVAILABLE"             ; // 0x00000025UL
		case PICO_SEGMENT_OUT_OF_RANGE             : return "PICO_SEGMENT_OUT_OF_RANGE"             ; // 0x00000026UL
		case PICO_BUSY                             : return "PICO_BUSY"                             ; // 0x00000027UL
		case PICO_STARTINDEX_INVALID               : return "PICO_STARTINDEX_INVALID"               ; // 0x00000028UL
		case PICO_INVALID_INFO                     : return "PICO_INVALID_INFO"                     ; // 0x00000029UL
		case PICO_INFO_UNAVAILABLE                 : return "PICO_INFO_UNAVAILABLE"                 ; // 0x0000002AUL
		case PICO_INVALID_SAMPLE_INTERVAL          : return "PICO_INVALID_SAMPLE_INTERVAL"          ; // 0x0000002BUL
		case PICO_TRIGGER_ERROR                    : return "PICO_TRIGGER_ERROR"                    ; // 0x0000002CUL
		case PICO_MEMORY                           : return "PICO_MEMORY"                           ; // 0x0000002DUL
		case PICO_SIG_GEN_PARAM                    : return "PICO_SIG_GEN_PARAM"                    ; // 0x0000002EUL
		case PICO_SHOTS_SWEEPS_WARNING             : return "PICO_SHOTS_SWEEPS_WARNING"             ; // 0x0000002FUL
		case PICO_SIGGEN_TRIGGER_SOURCE            : return "PICO_SIGGEN_TRIGGER_SOURCE"            ; // 0x00000030UL
		case PICO_AUX_OUTPUT_CONFLICT              : return "PICO_AUX_OUTPUT_CONFLICT"              ; // 0x00000031UL
		case PICO_AUX_OUTPUT_ETS_CONFLICT          : return "PICO_AUX_OUTPUT_ETS_CONFLICT"          ; // 0x00000032UL
		case PICO_WARNING_EXT_THRESHOLD_CONFLICT   : return "PICO_WARNING_EXT_THRESHOLD_CONFLICT"   ; // 0x00000033UL
		case PICO_WARNING_AUX_OUTPUT_CONFLICT      : return "PICO_WARNING_AUX_OUTPUT_CONFLICT"      ; // 0x00000034UL
		case PICO_SIGGEN_OUTPUT_OVER_VOLTAGE       : return "PICO_SIGGEN_OUTPUT_OVER_VOLTAGE"       ; // 0x00000035UL
		case PICO_DELAY_NULL                       : return "PICO_DELAY_NULL"                       ; // 0x00000036UL
		case PICO_INVALID_BUFFER                   : return "PICO_INVALID_BUFFER"                   ; // 0x00000037UL
		case PICO_SIGGEN_OFFSET_VOLTAGE            : return "PICO_SIGGEN_OFFSET_VOLTAGE"            ; // 0x00000038UL
		case PICO_SIGGEN_PK_TO_PK                  : return "PICO_SIGGEN_PK_TO_PK"                  ; // 0x00000039UL
		case PICO_CANCELLED                        : return "PICO_CANCELLED"                        ; // 0x0000003AUL
		case PICO_SEGMENT_NOT_USED                 : return "PICO_SEGMENT_NOT_USED"                 ; // 0x0000003BUL
		case PICO_INVALID_CALL                     : return "PICO_INVALID_CALL"                     ; // 0x0000003CUL
		case PICO_GET_VALUES_INTERRUPTED           : return "PICO_GET_VALUES_INTERRUPTED"           ; // 0x0000003DUL
		case PICO_NOT_USED                         : return "PICO_NOT_USED"                         ; // 0x0000003FUL
		case PICO_INVALID_SAMPLERATIO              : return "PICO_INVALID_SAMPLERATIO"              ; // 0x00000040UL
		// Operation could not be carried out because device was in an invalid state.
		case PICO_INVALID_STATE                    : return "PICO_INVALID_STATE"                    ; // 0x00000041UL
		// Operation could not be carried out as rapid capture no of waveforms are greater than the
		// no of memory segments.
		case PICO_NOT_ENOUGH_SEGMENTS              : return "PICO_NOT_ENOUGH_SEGMENTS"              ; // 0x00000042UL
		// A driver function has already been called and not yet finished
		// only one call to the driver can be made at any one time
		case PICO_DRIVER_FUNCTION                  : return "PICO_DRIVER_FUNCTION"                  ; // 0x00000043UL
		case PICO_RESERVED                         : return "PICO_RESERVED"                         ; // 0x00000044UL
		case PICO_INVALID_COUPLING                 : return "PICO_INVALID_COUPLING"                 ; // 0x00000045UL
		case PICO_BUFFERS_NOT_SET                  : return "PICO_BUFFERS_NOT_SET"                  ; // 0x00000046UL
		case PICO_RATIO_MODE_NOT_SUPPORTED         : return "PICO_RATIO_MODE_NOT_SUPPORTED"         ; // 0x00000047UL
		case PICO_RAPID_NOT_SUPPORT_AGGREGATION    : return "PICO_RAPID_NOT_SUPPORT_AGGREGATION"    ; // 0x00000048UL
		case PICO_INVALID_TRIGGER_PROPERTY         : return "PICO_INVALID_TRIGGER_PROPERTY"         ; // 0x00000049UL
		// manually added
		case PICO_INTERFACE_NOT_CONNECTED          : return "PICO_INTERFACE_NOT_CONNECTED"          ; // 0x0000004AUL
		case PICO_SIGGEN_WAVEFORM_SETUP_FAILED     : return "PICO_SIGGEN_WAVEFORM_SETUP_FAILED"     ; // 0x0000004DUL
		case PICO_FPGA_FAIL                        : return "PICO_FPGA_FAIL"                        ; // 0x0000004EUL
		case PICO_POWER_MANAGER                    : return "PICO_POWER_MANAGER"                    ; // 0x0000004FUL
		case PICO_INVALID_ANALOGUE_OFFSET          : return "PICO_INVALID_ANALOGUE_OFFSET"          ; // 0x00000050UL
		case PICO_PLL_LOCK_FAILED                  : return "PICO_PLL_LOCK_FAILED"                  ; // 0x00000051UL
		case PICO_ANALOG_BOARD                     : return "PICO_ANALOG_BOARD"                     ; // 0x00000052UL
		case PICO_CONFIG_FAIL_AWG                  : return "PICO_CONFIG_FAIL_AWG"                  ; // 0x00000053UL
		case PICO_INITIALIZE_FPGA                  : return "PICO_INITIALIZE_FPGA"                  ; // 0x00000054UL
		case PICO_EXTERNAL_FREQUENCY_INVALID       : return "PICO_EXTERNAL_FREQUENCY_INVALID"       ; // 0x00000056UL
		case PICO_CLOCK_CHANGE_ERROR               : return "PICO_CLOCK_CHANGE_ERROR"               ; // 0x00000057UL
		case PICO_TRIGGER_AND_EXTERNAL_CLOCK_CLASH : return "PICO_TRIGGER_AND_EXTERNAL_CLOCK_CLASH" ; // 0x00000058UL
		case PICO_PWQ_AND_EXTERNAL_CLOCK_CLASH     : return "PICO_PWQ_AND_EXTERNAL_CLOCK_CLASH"     ; // 0x00000059UL
		case PICO_GET_DATA_ACTIVE                  : return "PICO_GET_DATA_ACTIVE"                  ; // 0x00000103UL
		default: return "unknown error.";
		// default:
		// 	a = "abc";
		// 	// sprintf(a,"unknown error (%lu)", errorNumber);
		// 	return (const char *)(a);
	}
}

const char* Picoscope::PicoscopeException::GetVerboseErrorMessage() const
{
	// #define PICO_OK0x00000000UL
	switch(errorNumber) {
		case PICO_MEMORY_FAIL                    : return "Not enough memory could be allocated on the host machine";
		case PICO_FW_FAIL                        : return "Unable to download firmware";
		case PICO_OPEN_OPERATION_IN_PROGRESS     : return "PICO_OPEN_OPERATION_IN_PROGRESS";
		case PICO_OPERATION_FAILED               : return "PICO_OPERATION_FAILED";
		case PICO_KERNEL_DRIVER_TOO_OLD          : return "The picopp.sys file is too old to be used with the device driver";
		// TODO
		case PICO_EEPROM_CORRUPT                 : return "PICO_EEPROM_CORRUPT"                  ;
		case PICO_OS_NOT_SUPPORTED               : return "PICO_OS_NOT_SUPPORTED"                ;
		case PICO_INVALID_HANDLE                 : return "There is no device with the handle value passed";
		case PICO_INVALID_PARAMETER              : return "PICO_INVALID_PARAMETER"               ;
		case PICO_INVALID_TIMEBASE               : return "PICO_INVALID_TIMEBASE"                ;
		case PICO_INVALID_VOLTAGE_RANGE          : return "PICO_INVALID_VOLTAGE_RANGE"           ;
		case PICO_INVALID_CHANNEL                : return "PICO_INVALID_CHANNEL"                 ;
		case PICO_INVALID_TRIGGER_CHANNEL        : return "PICO_INVALID_TRIGGER_CHANNEL"         ;
		case PICO_INVALID_CONDITION_CHANNEL      : return "PICO_INVALID_CONDITION_CHANNEL"       ;
		case PICO_NO_SIGNAL_GENERATOR            : return "PICO_NO_SIGNAL_GENERATOR"             ;
		case PICO_STREAMING_FAILED               : return "PICO_STREAMING_FAILED"                ;
		case PICO_BLOCK_MODE_FAILED              : return "PICO_BLOCK_MODE_FAILED"               ;
		case PICO_NULL_PARAMETER                 : return "PICO_NULL_PARAMETER"                  ;
		case PICO_ETS_MODE_SET                   : return "PICO_ETS_MODE_SET"                    ;
		case PICO_DATA_NOT_AVAILABLE             : return "PICO_DATA_NOT_AVAILABLE"              ;
		case PICO_STRING_BUFFER_TO_SMALL         : return "PICO_STRING_BUFFER_TO_SMALL"          ;
		case PICO_ETS_NOT_SUPPORTED              : return "PICO_ETS_NOT_SUPPORTED"               ;
		case PICO_AUTO_TRIGGER_TIME_TO_SHORT     : return "PICO_AUTO_TRIGGER_TIME_TO_SHORT"      ;
		case PICO_BUFFER_STALL                   : return "PICO_BUFFER_STALL"                    ;
		case PICO_TOO_MANY_SAMPLES               : return "PICO_TOO_MANY_SAMPLES"                ;
		case PICO_TOO_MANY_SEGMENTS              : return "PICO_TOO_MANY_SEGMENTS"               ;
		case PICO_PULSE_WIDTH_QUALIFIER          : return "PICO_PULSE_WIDTH_QUALIFIER"           ;
		case PICO_DELAY                          : return "PICO_DELAY"                           ;
		case PICO_SOURCE_DETAILS                 : return "PICO_SOURCE_DETAILS"                  ;
		case PICO_CONDITIONS                     : return "PICO_CONDITIONS"                      ;
		case PICO_USER_CALLBACK                  : return "PICO_USER_CALLBACK"                   ;
		case PICO_DEVICE_SAMPLING                : return "PICO_DEVICE_SAMPLING"                 ;
		case PICO_NO_SAMPLES_AVAILABLE           : return "No samples available because a run has not been completed.";
		case PICO_SEGMENT_OUT_OF_RANGE           : return "PICO_SEGMENT_OUT_OF_RANGE"            ;
		case PICO_BUSY                           : return "PICO_BUSY"                            ;
		case PICO_STARTINDEX_INVALID             : return "PICO_STARTINDEX_INVALID"              ;
		case PICO_INVALID_INFO                   : return "PICO_INVALID_INFO"                    ;
		case PICO_INFO_UNAVAILABLE               : return "PICO_INFO_UNAVAILABLE"                ;
		case PICO_INVALID_SAMPLE_INTERVAL        : return "PICO_INVALID_SAMPLE_INTERVAL"         ;
		case PICO_TRIGGER_ERROR                  : return "PICO_TRIGGER_ERROR"                   ;
		case PICO_MEMORY                         : return "PICO_MEMORY"                          ;
		case PICO_SIG_GEN_PARAM                  : return "PICO_SIG_GEN_PARAM"                   ;
		case PICO_SHOTS_SWEEPS_WARNING           : return "PICO_SHOTS_SWEEPS_WARNING"            ;
		case PICO_SIGGEN_TRIGGER_SOURCE          : return "PICO_SIGGEN_TRIGGER_SOURCE"           ;
		case PICO_AUX_OUTPUT_CONFLICT            : return "PICO_AUX_OUTPUT_CONFLICT"             ;
		case PICO_AUX_OUTPUT_ETS_CONFLICT        : return "PICO_AUX_OUTPUT_ETS_CONFLICT"         ;
		case PICO_WARNING_EXT_THRESHOLD_CONFLICT : return "PICO_WARNING_EXT_THRESHOLD_CONFLICT"  ;
		case PICO_WARNING_AUX_OUTPUT_CONFLICT    : return "PICO_WARNING_AUX_OUTPUT_CONFLICT"     ;
		case PICO_SIGGEN_OUTPUT_OVER_VOLTAGE     : return "PICO_SIGGEN_OUTPUT_OVER_VOLTAGE"      ;
		case PICO_DELAY_NULL                     : return "PICO_DELAY_NULL"                      ;
		case PICO_INVALID_BUFFER                 : return "PICO_INVALID_BUFFER"                  ;
		case PICO_SIGGEN_OFFSET_VOLTAGE          : return "PICO_SIGGEN_OFFSET_VOLTAGE"           ;
		case PICO_SIGGEN_PK_TO_PK                : return "PICO_SIGGEN_PK_TO_PK"                 ;
		case PICO_CANCELLED                      : return "PICO_CANCELLED"                       ;
		case PICO_SEGMENT_NOT_USED               : return "PICO_SEGMENT_NOT_USED"                ;
		case PICO_INVALID_CALL                   : return "PICO_INVALID_CALL"                    ;
		case PICO_GET_VALUES_INTERRUPTED         : return "PICO_GET_VALUES_INTERRUPTED"          ;
		case PICO_NOT_USED                       : return "PICO_NOT_USED"                        ;
		case PICO_INVALID_SAMPLERATIO            : return "PICO_INVALID_SAMPLERATIO"             ;
		// Operation could not be carried out because device was in an invalid state.
		case PICO_INVALID_STATE                  : return "PICO_INVALID_STATE"                   ;
		// Operation could not be carried out as rapid capture no of waveforms are greater than the
		// no of memory segments.
		case PICO_NOT_ENOUGH_SEGMENTS              : return "PICO_NOT_ENOUGH_SEGMENTS"              ;
		// A driver function has already been called and not yet finished
		// only one call to the driver can be made at any one time
		case PICO_DRIVER_FUNCTION                  : return "PICO_DRIVER_FUNCTION"                  ;
		case PICO_RESERVED                         : return "PICO_RESERVED"                         ;
		case PICO_INVALID_COUPLING                 : return "PICO_INVALID_COUPLING"                 ;
		case PICO_BUFFERS_NOT_SET                  : return "PICO_BUFFERS_NOT_SET"                  ;
		case PICO_RATIO_MODE_NOT_SUPPORTED         : return "PICO_RATIO_MODE_NOT_SUPPORTED"         ;
		case PICO_RAPID_NOT_SUPPORT_AGGREGATION    : return "PICO_RAPID_NOT_SUPPORT_AGGREGATION"    ;
		case PICO_INVALID_TRIGGER_PROPERTY         : return "PICO_INVALID_TRIGGER_PROPERTY";
		// manually added
		case PICO_INTERFACE_NOT_CONNECTED          : return "The driver was unable to contact the oscilloscope";
		case PICO_SIGGEN_WAVEFORM_SETUP_FAILED     : return "A problem occurred in ps6000SetSigGenBuiltIn or ps6000SetSigGenArbitrary";
		case PICO_FPGA_FAIL                        : return "PICO_FPGA_FAIL";
		case PICO_POWER_MANAGER                    : return "PICO_POWER_MANAGER";
		case PICO_ANALOG_BOARD                     : return "The oscilloscope's analog board is not detected, or is not connected to the digital board";
		case PICO_CONFIG_FAIL_AWG                  : return "Unable to configure the signal generator";
		case PICO_INITIALIZE_FPGA                  : return "The FPGA cannot be initialized, so unit cannot be opened";
		case PICO_EXTERNAL_FREQUENCY_INVALID       : return "The frequency for the external clock is not within +/-5 % of the stated value";
		case PICO_CLOCK_CHANGE_ERROR               : return "The FPGA could not lock the clock signal";
		case PICO_TRIGGER_AND_EXTERNAL_CLOCK_CLASH : return "You are trying to configure the AUX input as both a trigger and a reference clock";
		case PICO_PWQ_AND_EXTERNAL_CLOCK_CLASH     : return "You are trying to congfigure the AUX input as both a pulse width qualifier and a reference clock";
		case PICO_GET_DATA_ACTIVE                  : return "(reserved for future use)";

		// default:
		// 	switch(GetSeries()) {
		// 		case PICO_4000 :
		// 			switch(errorNumber) {
						case PICO_OK                             : return "The PicoScope 4000 is functioning correctly.";
						// TODO: PS4000_MAX_UNITS should resolve to a valid number
						case PICO_MAX_UNITS_OPENED               : return "An attempt has been made to open more than PS4000_MAX_UNITS. (Reserved.)";
						case PICO_NOT_FOUND                      : return "No PicoScope 4000 could be found";
						case PICO_NOT_RESPONDING                 : return "The PicoScope 4000 is not responding to commands from the PC";
						case PICO_CONFIG_FAIL                    : return "The configuration information in the PicoScope 4000 has become corrupt or is missing";
						default: return "unknown error.\n";
			// 		}
			// 	case PICO_6000 :
			// 		switch(errorNumber) {
			// 			case PICO_OK                             : return "The PicoScope 6000 is functioning correctly.";
			// 			// TODO: PS4000_MAX_UNITS should resolve to a valid number
			// 			case PICO_MAX_UNITS_OPENED               : return "An attempt has been made to open more than PS6000_MAX_UNITS. (Reserved.)";
			// 			case PICO_NOT_FOUND                      : return "No PicoScope 6000 could be found";
			// 			case PICO_NOT_RESPONDING                 : return "The PicoScope 6000 is not responding to commands from the PC";
			// 			case PICO_CONFIG_FAIL                    : return "The configuration information in the PicoScope 6000 has become corrupt or is missing";
			// 
			// 			case PICO_INVALID_ANALOGUE_OFFSET        : return "An impossible analogue offset value was specified in ps6000SetChannel";
			// 			case PICO_PLL_LOCK_FAILED                : return "Unable to configure the PicoScope 6000";
			// 
			// 			default: return "unknown error.\n";
			// 		}
			// }
	}
}
