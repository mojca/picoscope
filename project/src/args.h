#ifndef __ARGS_H__
#define __ARGS_H__

// TODO: rewrite the code into C++
#include "measurement.h"
#include "channel.h"
#include "trigger.h"

#include <cstddef>

class Args;

struct gen_table {
	const char *key;
	int value;
};

enum PICO_args {
	PICO_ARG_HELP,     // --help
	PICO_ARG_BINARY,   // --bin | --binary
	PICO_ARG_TEXT,     // --dat | --text
	PICO_ARG_LENGTH,   // --length | --l
	PICO_ARG_NTRACES,  // --n
	PICO_ARG_NREPEATS, // --repeat <number of repeats>
	PICO_ARG_CHANNEL,  // --ch
	PICO_ARG_FILENAME, // --name
	PICO_ARG_VOLTAGE,  // --voltage | --U
	PICO_ARG_RATE,     // --dt
	PICO_ARG_TRIGGER,  // --trigger | --trig
	PICO_ARG_SIGNAL_SQUARE, // --square
	PICO_ARG_SPECIAL_N_GAMMA, // --ngamma <i2_offset_from_trigger> <i2_length>
	PICO_ARG_OTHER,
	PICO_ARG_UNKNOWN,
	PICO_NOT_ARG
};

static struct gen_table PICO_arguments[] = {
	{ "help",    PICO_ARG_HELP     }, // --help
	{ "bin",     PICO_ARG_BINARY   }, // --bin | --binary
	{ "binary",  PICO_ARG_BINARY   },
	{ "dat",     PICO_ARG_TEXT     }, // --dat | --text
	{ "text",    PICO_ARG_TEXT     },
	{ "l",       PICO_ARG_LENGTH   },
	{ "length",  PICO_ARG_LENGTH   },
	{ "n",       PICO_ARG_NTRACES  }, // --n <number of traces>
	{ "repeat",  PICO_ARG_NREPEATS }, // --repeat <number of repeats>
	{ "ch",      PICO_ARG_CHANNEL  },
	{ "channel", PICO_ARG_CHANNEL  },
	{ "U",       PICO_ARG_VOLTAGE  }, // --U <XmV>
	{ "voltage", PICO_ARG_VOLTAGE  },
	{ "dt",      PICO_ARG_RATE     }, // --dt <number>ns | <nuber>ps | <number>MHz <number>GHz
	{ "trigger", PICO_ARG_TRIGGER  }, // --trigger <xfrac> <yfrac>
	{ "trig",    PICO_ARG_TRIGGER  },
	{ "name",    PICO_ARG_FILENAME },
	{ "square",  PICO_ARG_SIGNAL_SQUARE}, // --square <XmV> <Xns>
	//
	{ "ngamma",  PICO_ARG_SPECIAL_N_GAMMA}, // --ngamma <i2_offset_from_trigger> <i2_length>
	{ NULL,      PICO_ARG_OTHER    }
};

// TODO: has to go to some other file
// static struct gen_table PICO_voltage[] = {
// 	{ "50mV",  U_50mV  },
// 	{ "100mV", U_100mV },
// 	{ "200mV", U_200mV },
// 	{ "500mV", U_500mV },
// 	{ "1V",    U_1V    },
// 	{ "2V",    U_2V    },
// 	{ "5V",    U_5V    },
// 	{ "10V",   U_10V   },
// 	{ "20V",   U_20V   },
// 	{ "50V",   U_50V   },
// 	{ "100V",  U_100V  },
// 	{ NULL,    U_MAX   }
// };

// void parse_options(int, char **);
int lookup_table(const struct gen_table *, const char *);
// bool almost_equals(const char *, const char *);

class Args {
public:
	Args();
	~Args();

	void parse_options(int, char **, Measurement *m);

	Measurement* GetMeasurement() const { return measurement; };

	void SetFilename(char *);
	char* GetFilename()        const { return filename; }
	char* GetFilenameBinary()  const { return filename_binary[4]; }
	char* GetFilenameText()    const { return filename_text[4]; }
	char* GetFilenameMeta()    const { return filename_meta; }
	char* GetFilenameSpecial() const { return filename_special[4]; }

	// TODO: this should throw an error if i is not between 0 and 3
	char* GetFilenameBinary(int i)  const { return filename_binary[i]; }
	char* GetFilenameText(int i)    const { return filename_text[i]; }
	char* GetFilenameSpecial(int i) const { return filename_special[i]; }


	PICO_VOLTAGE ParseVoltage(char *);
	void ParseAndSetVoltage(char *);
	void SetVoltage(PICO_VOLTAGE v) { voltage = v; };
	PICO_VOLTAGE GetVoltage() const { return voltage; };
	double GetVoltageDouble();

	void ParseAndSetRate(char *);

	void ParseAndSetLength(char *);
	void SetLength(unsigned long);
	unsigned long GetLength() const { return length; };

	void ParseAndSetNTraces(char *);
	unsigned long GetNTraces() const { return ntraces; };

	void ParseAndSetNRepeats(char *);
	unsigned long GetNRepeats() const { return nrepeats; };

	bool IsTriggered() const { return is_triggered; };
	void ParseAndSetTrigger(char *, char *);
	Trigger* GetTrigger(Channel *ch);

	void ParseAndSetSquareSignalGenerator(char *, char *);

	void ParseAndSetSpecialNGamma(char *, char *);
	bool IsSpecialNGamma() const { return is_special_n_gamma; };
	unsigned long GetSpecialNGammaIntegralOffsetFromTrigger() const { return special_n_gamma_i2_offset_from_trigger; };
	unsigned long GetSpecialNGammaIntegralLength() const { return special_n_gamma_i2_length; };

	void ParseAndSetChannels(char *);

	void PrintUsage();
	bool IsJustHelp() const { return is_just_help; };
	bool IsTextOutput() const { return is_text_output; };
	bool IsBinaryOutput() const { return is_binary_output; };
	bool IsSpecialOutput() const { return is_special_output; };

private:
	Measurement *measurement;
	char *filename, *filename_binary[5], *filename_text[5], *filename_meta, *filename_special[5];
	unsigned long length;
	unsigned long ntraces;
	unsigned long nrepeats;
	PICO_VOLTAGE voltage;
	bool is_triggered;
	double x_frac, y_frac;
	bool is_just_help;
	bool is_binary_output, is_text_output, is_special_output;
	// bool channel_enabled[PICOSCOPE_N_CHANNELS];

	PICO_VOLTAGE generator_voltage;
	// unsigned long generator_ns;

	// special functions
	// n-gamma discrimination parameters
	bool is_special_n_gamma;
	unsigned long special_n_gamma_i2_offset_from_trigger;
	unsigned long special_n_gamma_i2_length;

	double GetTriggerXFraction() const { return x_frac; }
	double GetTriggerYFraction() const { return y_frac; }
};

#endif
