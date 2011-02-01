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
public:
	Picoscope(PICO_SERIES);
	~Picoscope();

	PICO_STATUS Open();
	PICO_STATUS Close();
	bool        isReady()  const { return var_is_ready; };
	PICO_SERIES getSeries() const { return series; };
	short       getHandle() const { return handle; };

	// void CALLBACK CallBackBlock (short handle, PICO_STATUS status, void *pParameter);

	/* temporary: remove after testing is done */
	void do_something_nasty();
	void DoSomething(unsigned long trace_length);
	short** Picoscope::GetData();
	void MyFunction(unsigned long trace_length);

	/* handling exceptions */
	class PicoscopeException {
	public:
		PicoscopeException(int number):errorNumber(number) {}
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
		PicoscopeUserException(const char* message):errorMessage(message) {}
		~PicoscopeUserException() {}
		const char* GetErrorMessage() const;
	private:
		const char* errorMessage;
	};

private:
	PICO_SERIES series;
	// if picoscope is open or not (for book-keeping; not really needed)
	bool var_is_open;
	// 
	bool var_is_ready;
	PICO_STATUS return_status;
	// the number that gets assigned to unit
	// -2: initial value, before we even assign anything
	// -----------------
	// -1: fails to open
	//  0: no unit found
	// >0: value is the handle to the open device
	short handle;
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
