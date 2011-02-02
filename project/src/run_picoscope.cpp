#include <iostream>
#include "windows.h"

#include "picoscope.h"
#include "measurement.h"
#include "channel.h"

#include "ps4000Api.h"
#include "ps6000Api.h"
// #include "picoStatus.h"

#include <conio.h>
// #include <stdio.h>

// #include <sys/types.h>
// #include <time.h>

using namespace std;

int main(int argc, char** argv)
{
	try {
		// short **data;
		// unsigned long length = (unsigned long)1e7;
		// length = 4200610;

		Picoscope6000 *pico = new Picoscope6000();
		Measurement   *meas = new Measurement(pico);
		Channel       *a    = meas->GetChannel(0);

		a->Enable();
		a->SetVoltage(U_5V);
		meas->SetTimebaseInPs(200UL);
		meas->SetLength(1000000UL);

		cerr << "open picoscope\n";
		pico->Open();
		cerr << "run\n";
		meas->RunBlock();
		cerr << "close\n";
		pico->Close();
		cerr << "end\n";

		delete pico;
		delete meas;

	} catch(Picoscope::PicoscopeException& ex) {
		cerr << "Some picoscope exception:" << endl
		     << ex.GetErrorMessage() << ex.GetErrorNumber() << endl
		     << '(' << ex.GetVerboseErrorMessage() << ')' << endl;
		try {
			// pico.Close();
		} catch(...) {}
	} catch(Picoscope::PicoscopeUserException& ex) {
		cerr << "Some exception:" << endl
		     << ex.GetErrorMessage() << endl;
	// catch any exceptions
	} catch(...) {
		cout << "Some exception has occurred" << endl;
	}
	return 0;
}
