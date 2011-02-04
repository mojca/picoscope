#include <iostream>
#include <stdio.h>
#include "windows.h"

#include "picoscope.h"
#include "measurement.h"
#include "channel.h"

#include "timing.h"

#include "ps4000Api.h"
#include "ps6000Api.h"
// #include "picoStatus.h"

#include <conio.h>
// #include <stdio.h>

// #include <sys/types.h>
// #include <time.h>

#define MEGA(a) ((unsigned long)(a*1000000UL))
#define GIGA(a) ((unsigned long)(a*1000000000UL))

using namespace std;

int main(int argc, char** argv)
{
	Timing t;
	t.Start();
	try {
		// short **data;
		// unsigned long length = (unsigned long)1e7;
		// length = 4200610;

		Picoscope6000 *pico = new Picoscope6000();
		Measurement   *meas = new Measurement(pico);
		Channel       *a    = meas->GetChannel(0);
		Channel       *d    = meas->GetChannel(3);

		a->Enable();
		a->SetVoltage(U_5V);
		d->Enable();
		d->SetVoltage(U_5V);
		meas->SetTimebaseInPs(400UL);
		meas->SetLength(MEGA(200));
		meas->AllocateMemory(MEGA(100));

		pico->Open();
		meas->RunBlock();

		FILE *f = fopen("C:\\Temp\\Data\\x.dat", "wb");
		while(meas->GetNextData() > 0) {
			meas->WriteDataBin(f, 0); // zero for channel A
		}
		fclose(f);

		pico->Close();
		t.Stop();

		cerr << "Timing: " << t.GetSecondsDouble() << "s\n";
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
		cerr << "Some exception has occurred" << endl;
	}
	return 0;
}
