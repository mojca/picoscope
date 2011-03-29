#include <iostream>
#include <stdio.h>
#include <time.h>
#include "windows.h"

#include "picoscope.h"
#include "measurement.h"
#include "channel.h"
#include "trigger.h"
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

void test(short *x)
{
	x[0] = 12;
	x[1] = 1000;
	x[2] = 1000;
}

int main(int argc, char** argv)
{
	Timing t;

	// #define XYMAX 4200000
	// std::cerr << "test start\n";
	// short *xy[4];
	// xy[0] = new short[XYMAX];
	// test(&xy[0][XYMAX-5]);
	// delete [] xy[0];
	// std::cerr << "test stop\n";

	t.Start();
	try {
		// short **data;
		// unsigned long length = (unsigned long)1e7;
		// length = 4200610;

		// std::cerr << "test w0\n";

		Picoscope6000 *pico = new Picoscope6000();
		Measurement   *meas = new Measurement(pico);
		Channel       *a    = meas->GetChannel(0);

		// std::cerr << "test w1\n";

		// meas->AddSimpleTrigger(a,0.5,-0.3);

		// std::cerr << "test w2\n";

		Args x;
		x.parse_options(argc, argv);
		if(x.IsJustHelp()) {
			return 0;
		}

		if(x.GetFilename() == NULL) { // TODO: maybe we want to use just text file
			throw("You have to provide some filename using '--name <filename>'.\n");
		}

		// std::cerr << "test w3\n";

		a->Enable();
		meas->SetTimebaseInPs(200);
		// meas->SetTimebaseInPs(10000);

		// a->SetVoltage(U_100mV);
		a->SetVoltage(x.GetVoltage());
		// meas->SetLength(GIGA(1));
		meas->SetLength(x.GetLength());

		// std::cerr << "test w4\n";

		if(x.GetNTraces() > 1) {
			meas->SetNTraces(x.GetNTraces());
			// meas->SetTrigger(x.GetTrigger(a));
			meas->AllocateMemoryRapidBlock(MEGA(200));
			// meas->AllocateMemoryRapidBlock(20000);
		} else {
			meas->AllocateMemoryBlock(MEGA(200));
		}

		// std::cerr << "test w5\n";

		// it only makes sense to measure if we decided to use some positive number of samples
		if(x.GetLength()>0) {

			FILE *f = NULL;
			FILE *fb = NULL, *ft = NULL;

			struct tm *current;
			time_t now;
			time(&now);
			current = localtime(&now);

			/* metadata */
			f = fopen(x.GetFilenameMeta(), "wt");
			if(f == NULL) {
				throw("Unable to open file with metadata.\n");
			}
			fprintf(f, "timestamp: %d-%02d-%02d %02d:%02d:%02d\n\n",
				current->tm_year+1900, current->tm_mon+1, current->tm_mday,
				current->tm_hour, current->tm_min, current->tm_sec);
			fprintf(f, "length:    %ld\n", x.GetLength());
			fprintf(f, "range:     %g V\n", x.GetVoltageDouble());
			fprintf(f, "unit_x:    %.1lf ns\n", meas->GetTimebaseInNs());
			fprintf(f, "unit_y:    %.6le V\n", x.GetVoltageDouble()*3.0517578125e-5);
			fprintf(f, "out_bin:   %s\n", x.IsTextOutput() ? "yes" : "no");
			fprintf(f, "out_dat:   %s\n", x.IsBinaryOutput() ? "yes" : "no");
			
			fclose(f);

			if(x.IsTextOutput()) {
				ft = fopen(x.GetFilenameText(), "wt");
				if(ft == NULL) {
					throw("Unable to open text file.\n"); // TODO: write filename
				}
			}
			if(x.IsBinaryOutput()) {
				fb = fopen(x.GetFilenameBinary(), "wb");
				if(fb == NULL) {
					throw("Unable to open binary file.\n"); // TODO: write filename
				}
			}


			/************************************************************/
			pico->Open();
			meas->RunBlock();

			// FILE *g = fopen("C:\\Temp\\Data\\x.dat", "wt");
			if(x.GetNTraces() > 1) {
				while(meas->GetNextDataBulk() > 0) {
					if(x.IsTextOutput()) {
						meas->WriteDataTxt(ft, 0); // zero for channel A
					}
					if(x.IsBinaryOutput()) {
						meas->WriteDataBin(fb, 0); // zero for channel A
					}
				}
			} else {
				while(meas->GetNextData() > 0) {
					if(x.IsTextOutput()) {
						meas->WriteDataTxt(ft, 0); // zero for channel A
					}
					if(x.IsBinaryOutput()) {
						meas->WriteDataBin(fb, 0); // zero for channel A
					}
				}
			}

			if(ft != NULL) {
				fclose(ft);
			}
			if(fb != NULL) {
				fclose(fb);
			}

			// apparently this doesn't work for some weird reason
			// meas->RunBlock(); meas->GetNextData();
			// meas->RunBlock(); meas->GetNextData();
			// meas->RunBlock(); meas->GetNextData();
			// meas->RunBlock(); meas->GetNextData();

			pico->Close();
			t.Stop();

			cerr << "Timing: " << t.GetSecondsDouble() << "s\n";
		}

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
