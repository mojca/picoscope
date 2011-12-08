#include <iostream>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include "windows.h"

#include "picoscope.h"
#include "measurement.h"
#include "channel.h"
#include "trigger.h"
#include "args.h"

#include "log.h"
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
	int i;

	// TODO: debug should be enabled with a command-line option
	// FILELog::ReportingLevel() = FILELog::FromString("DEBUG4");
	// FILELog::ReportingLevel() = FILELog::FromString("DEBUG1");
	FILELog::ReportingLevel() = FILELog::FromString("INFO");
	FILE_LOG(logDEBUG4) << "starting";

	t.Start();
	try {

		Picoscope6000 *pico = new Picoscope6000();
		Measurement   *meas = new Measurement(pico);
		Channel       *ch[4];

		meas->SetTimebaseInPs(400);
		meas->EnableChannels(true,false,false,false);
		for(i=0; i<PICOSCOPE_N_CHANNELS; i++) {
			ch[i] = meas->GetChannel(i);
			FILE_LOG(logDEBUG4) << "main - Channel " << (char)('A'+i) << " has index " << ch[i]->GetIndex();
		}

		Args x;
		x.parse_options(argc, argv, meas);
		if(x.IsJustHelp()) {
			return 0;
		}

		if(x.GetFilename() == NULL) { // TODO: maybe we want to use just text file
			throw("You have to provide some filename using '--name <filename>'.\n");
		}

		// meas->SetTimebaseInPs(10000);

		// TODO: fixme
		for(i=0; i<PICOSCOPE_N_CHANNELS; i++) {
			ch[i]->SetVoltage(x.GetVoltage());
		}

		// a->SetVoltage(U_100mV);
		// a[0]->SetVoltage(x.GetVoltage());
		// meas->SetLength(GIGA(1));
		meas->SetLength(x.GetLength());

		if(x.GetNTraces() > 1) {
			meas->SetNTraces(x.GetNTraces());
			// TODO: fix trigger
			FILE_LOG(logDEBUG4) << "main - checking for triggered events";
			if(x.IsTriggered()) {
				for(i=0; i<PICOSCOPE_N_CHANNELS && !(ch[i]->IsEnabled()); i++);
				FILE_LOG(logDEBUG4) << "main - will trigger on channel " << (char)('A'+i);
				meas->SetTrigger(x.GetTrigger(ch[i]));
			}
			meas->AllocateMemoryRapidBlock(MEGA(200));
			// meas->AllocateMemoryRapidBlock(20000);
		} else {
			meas->AllocateMemoryBlock(MEGA(200));
		}

		// std::cerr << "test w5\n";

		// it only makes sense to measure if we decided to use some positive number of samples
		if(x.GetLength()>0) {

			FILE *f = NULL;
			FILE *fb[4] = {NULL,NULL,NULL,NULL}, *ft[4] = {NULL,NULL,NULL,NULL};

			struct tm *current;
			time_t now;
			time(&now);
			current = localtime(&now);

			for(i=0; i<PICOSCOPE_N_CHANNELS; i++) {
				if(ch[i]->IsEnabled()) {
					if(x.IsTextOutput()) {
						ft[i] = fopen(x.GetFilenameText(i), "wt");
						if(ft[i] == NULL) {
							throw("Unable to open text file.\n"); // TODO: write filename
						}
					}
					if(x.IsBinaryOutput()) {
						fb[i] = fopen(x.GetFilenameBinary(i), "wb");
						if(fb[i] == NULL) {
							throw("Unable to open binary file.\n"); // TODO: write filename
						}
					}
				}
			}

			/************************************************************/
			double tmp_dbl;
			short  tmp_short;
			pico->Open();
			meas->InitializeSignalGenerator();
			meas->RunBlock();

			/* metadata */
			f = fopen(x.GetFilenameMeta(), "wt");
			if(f == NULL) {
				throw("Unable to open file with metadata.\n");
			}
			fprintf(f, "command:   ");
			for(i=0; i<argc; i++) {
				fprintf(f, " %s", argv[i]);
			}
			fprintf(f, "\n");
			fprintf(f, "timestamp:  %d-%02d-%02d %02d:%02d:%02d\n\n",
				current->tm_year+1900, current->tm_mon+1, current->tm_mday,
				current->tm_hour, current->tm_min, current->tm_sec);
			fprintf(f, "channels:   ");
			for(i=0; i<PICOSCOPE_N_CHANNELS; i++) {
				if(ch[i]->IsEnabled()) {
					fprintf(f, "%c", 'A'+i);
				}
			}
			fprintf(f, "\n");
			fprintf(f, "length:     %ld\n", x.GetLength());
			fprintf(f, "samples:    %ld\n", x.GetNTraces());
			// fprintf(f, "unit_x:    %.1lf ns | %.1lf ns\n", meas->GetTimebaseInNs(), meas->GetReportedTimebaseInNs());
			tmp_dbl = meas->GetTimebaseInNs();
			fprintf(f, "unit_x:     %.1lf ns\n", tmp_dbl);
			fprintf(f, "range_x:    %.1lf ns\n", x.GetLength()*tmp_dbl);
			tmp_dbl = x.GetVoltageDouble();
			fprintf(f, "unit_y:     %.6le V\n", tmp_dbl*3.0517578125e-5); // 2^(-15) since it goes from [-2^(15),2^(15)-1]
			fprintf(f, "range_y:    %g V\n", tmp_dbl);

			if(x.GetNTraces() > 1) {
				if(x.IsTriggered()) {
					fprintf(f, "trigger_ch: %c\n", (char)(meas->GetTrigger()->GetChannel()->GetIndex()+'A'));
					// fprintf(f, "trigger_xfrac: %g\n", meas->GetTrigger()->GetXFraction());
					// fprintf(f, "trigger_yfrac: %g\n", meas->GetTrigger()->GetYFraction());
					fprintf(f, "trigger_dx: %d (%g %%  of  %ld)\n", meas->GetLengthBeforeTrigger(), meas->GetLengthBeforeTrigger()*100.0/x.GetLength(), x.GetLength());
					tmp_short =  meas->GetTrigger()->GetThreshold();
					fprintf(f, "trigger_dy: %d (%g %%  or  %g V)\n", tmp_short, tmp_short*3.0517578125e-3, tmp_short*3.0517578125e-3*x.GetVoltageDouble());
				}
			}

			// fprintf(f, "unit_x:    %.1lf ns | %.1lf ns\n", meas->GetTimebaseInNs(), meas->GetReportedTimebaseInNs());
			fprintf(f, "out_bin:    %s\n", x.IsBinaryOutput() ? "yes" : "no");
			fprintf(f, "out_dat:    %s\n", x.IsTextOutput()   ? "yes" : "no");
			
			// FILE *g = fopen("C:\\Temp\\Data\\x.dat", "wt");
			if(x.GetNTraces() > 1) {
				while(meas->GetNextDataBulk() > 0) {
					for(i=0; i<PICOSCOPE_N_CHANNELS; i++) {
						if(ch[i]->IsEnabled()) {
							if(x.IsTextOutput()) {
								meas->WriteDataTxt(ft[i], i); // zero for channel A
							}
							if(x.IsBinaryOutput()) {
								meas->WriteDataBin(fb[i], i); // zero for channel A
							}
						}
					}
				}
				tmp_dbl = meas->GetRatePerSecond();
				fprintf(f, "\nrate:       \n");
				if(fabs(tmp_dbl) > 1e6) {
					fprintf(f, "%.3f MS/s\n", tmp_dbl*1e-6);
				} else if(fabs(tmp_dbl) > 1e3) {
					fprintf(f, "%.3f kS/s\n", tmp_dbl*1e-3);
				} else {
					fprintf(f, "%f S/s\n", tmp_dbl);
				}
			} else {
				while(meas->GetNextData() > 0) {
					for(i=0; i<PICOSCOPE_N_CHANNELS; i++) {
						if(ch[i]->IsEnabled()) {
							if(x.IsTextOutput()) {
								meas->WriteDataTxt(ft[i], i); // zero for channel A
							}
							if(x.IsBinaryOutput()) {
								meas->WriteDataBin(fb[i], i); // zero for channel A
							}
						}
					}
				}
			}

			fclose(f);

			for(i=0; i<PICOSCOPE_N_CHANNELS; i++) {
				if(ft[i] != NULL) {
					fclose(ft[i]);
				}
				if(fb[i] != NULL) {
					fclose(fb[i]);
				}
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
