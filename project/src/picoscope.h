#ifndef __PICOSCOPE_H__
#define __PICOSCOPE_H__

#include <iostream>
#include <string>
#include "picoStatus.h"

enum PICOSCOPE_STATE  {OPEN, CLOSED};

typedef enum picoscopeSeries {
	PICO_4000,
	PICO_6000
} PICO_SERIES;

#define PICOSCOPE_N_CHANNELS 4

#define PICOSCOPE_HANDLE_UNITIALIZED  -2
#define PICOSCOPE_HANDLE_FAIL_TO_OPEN -1
#define PICOSCOPE_HANDLE_NO_UNIT_FOUND 0

#define CALLBACK __stdcall

void CALLBACK CallBackBlock (short handle, PICO_STATUS status, void *pParameter);

class Picoscope {
private:
	PICO_SERIES series;
	// if picoscope is open or not (for book-keeping; not really needed)
	bool var_is_open;
	// 
	static bool var_is_ready;
	PICO_STATUS return_status; // WATCH OUT: another one is set by a measurement!!! TODO
	// the number that gets assigned to unit
	// -2: initial value, before we even assign anything
	// -----------------
	// -1: fails to open
	//  0: no unit found
	// >0: value is the handle to the open device
	short handle;

public:
	Picoscope(PICO_SERIES);
	~Picoscope();

	PICO_STATUS Open();
	PICO_STATUS Close();
	// bool        IsReady()   const { return var_is_ready; };
	static bool IsReady()            { return var_is_ready; };
	static void SetReady(bool ready) { var_is_ready=ready; };
	PICO_SERIES GetSeries() const    { return series; };
	short       GetHandle() const    { return handle; };
	void        SetStatus(PICO_STATUS);
	PICO_STATUS GetStatus() const    { return return_status; };

	// void CALLBACK CallBackBlock (short handle, PICO_STATUS status, void *pParameter);

	/* temporary: remove after testing is done */
	void do_something_nasty();
	void DoSomething(unsigned long trace_length);
	short** Picoscope::GetData();
	void MyFunction(unsigned long trace_length);
	// in theory this should be configurable, but we have all picoscopes with 4 channels
	int GetNumberOfChannels() const { return PICOSCOPE_N_CHANNELS; };

	/* handling exceptions */
	class PicoscopeException {
	public:
		PicoscopeException(int number):errorNumber(number) {};
		~PicoscopeException() {}
		PICO_STATUS GetErrorNumber() const;
		const char* GetErrorMessage() const;
		const char* GetVerboseErrorMessage() const;
	private:
		PICO_STATUS errorNumber;
	};
	/* handling exceptions */
	class PicoscopeUserException {
	public:
		PicoscopeUserException(const char* message):errorMessage(message) {};
		~PicoscopeUserException() {};
		const char* GetErrorMessage() const;
	private:
		const char* errorMessage;
	};
};

class Picoscope4000 : public Picoscope {
public:
	Picoscope4000() : Picoscope(PICO_4000) {};
};

class Picoscope6000 : public Picoscope {
public:
	Picoscope6000() : Picoscope(PICO_6000) {};
};

#endif
