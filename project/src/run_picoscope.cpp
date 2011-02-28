#include <iostream>
#include <stdio.h>
#include "windows.h"

#include "picoscope.h"
#include "measurement.h"
#include "channel.h"
#include "args.h"

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

		Args x;
		x.parse_options(argc, argv);
		if(x.GetFilename() == NULL) { // TODO: maybe we want to use just text file
			throw("You have to provide some filename using '--name <filename>'.\n");
		}

		a->Enable();
		// a->SetVoltage(U_100mV);
		a->SetVoltage(x.GetVoltage());
		meas->SetTimebaseInPs(200);
		// meas->SetLength(GIGA(1));
		meas->SetLength(x.GetLength());
		meas->AllocateMemory(MEGA(200));

		FILE *f = fopen(x.GetFilenameBinary(), "wb");
		if(f == NULL) {
			throw("Unable to open file.\n"); // TODO: write filename
		}

		pico->Open();
		meas->RunBlock();

		// FILE *g = fopen("C:\\Temp\\Data\\x.dat", "wt");
		while(meas->GetNextData() > 0) {
			meas->WriteDataBin(f, 0); // zero for channel A
			// meas->WriteDataTxt(g, 0); // zero for channel A
		}
		fclose(f);
		// fclose(g);

		// apparently this doesn't work for some weird reason
		// meas->RunBlock(); meas->GetNextData();
		// meas->RunBlock(); meas->GetNextData();
		// meas->RunBlock(); meas->GetNextData();
		// meas->RunBlock(); meas->GetNextData();

		pico->Close();
		t.Stop();

		cerr << "Timing: " << t.GetSecondsDouble() << "s\n";
		delete pico;
		delete meas;


	} catch(Picoscope::PicoscopeException& ex) {
		cerr << "Some picoscope exception:" << endl
		     << "Error number " << ex.GetErrorNumber() << ": " << ex.GetErrorMessage() << endl
		     << '(' << ex.GetVerboseErrorMessage() << ')' << endl;
		try {
			// pico.Close();
		} catch(...) {}
	} catch(Picoscope::PicoscopeUserException& ex) {
		cerr << "Some exception:" << endl
		     << ex.GetErrorMessage() << endl;
	// catch any exceptions
	} catch(const char* s) {
		cerr << "Some exception has occurred:\n" << s << endl;
	} catch(...) {
		cerr << "Some exception has occurred" << endl;
	}
	return 0;
}
