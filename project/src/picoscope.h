#ifndef __PICOSCOPE_H__
#define __PICOSCOPE_H__

#include <iostream>
#include <string>
#include "picoStatus.h"

enum PICOSCOPE_STATE {OPEN, CLOSED};

#define PICOSCOPE_N_CHANNELS 4

#define PICOSCOPE_HANDLE_UNITIALIZED  -2
#define PICOSCOPE_HANDLE_FAIL_TO_OPEN -1
#define PICOSCOPE_HANDLE_NO_UNIT_FOUND 0

#define CALLBACK __stdcall

void CALLBACK CallBackBlock (short handle, PICO_STATUS status, void *pParameter);

class Picoscope {
public:
	Picoscope();
	~Picoscope();

	PICO_STATUS Open();
	PICO_STATUS Close();
	bool is_ready() const;
	void EnableChannels(bool a, bool b, bool c, bool d);

	// void CALLBACK CallBackBlock (short handle, PICO_STATUS status, void *pParameter);

	/* temporary: remove after testing is done */
	void do_something_nasty();
	void DoSomething(unsigned long trace_length);
	short** Picoscope::GetData();

	class Channel {
	public:
		void Enable()    { is_enabled = true;  }
		void Disable()   { is_enabled = false; }
		bool IsEnabled() { return is_enabled; }
	private:
		bool is_enabled;
	};

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
	Channel channels[PICOSCOPE_N_CHANNELS];
	short *data[PICOSCOPE_N_CHANNELS];
};

#endif
