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
	PICO_ARG_CHANNEL,  // --ch
	PICO_ARG_FILENAME, // --name
	PICO_ARG_VOLTAGE,  // --voltage | --U
	PICO_ARG_TRIGGER,  // --trigger | --trig
	PICO_ARG_SIGNAL_SQUARE, // --square
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
	{ "ch",      PICO_ARG_CHANNEL  },
	{ "channel", PICO_ARG_CHANNEL  },
	{ "U",       PICO_ARG_VOLTAGE  }, // --U <XmV>
	{ "voltage", PICO_ARG_VOLTAGE  },
	{ "trigger", PICO_ARG_TRIGGER  }, // --trigger <xfrac> <yfrac>
	{ "trig",    PICO_ARG_TRIGGER  },
	{ "name",    PICO_ARG_FILENAME },
	{ "square",  PICO_ARG_SIGNAL_SQUARE}, // --square <XmV> <Xns>
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

	void parse_options(int, char **);

	void SetFilename(char *);
	char* GetFilename()       const { return filename; }
	char* GetFilenameBinary() const { return filename_binary; }
	char* GetFilenameText()   const { return filename_text; }
	char* GetFilenameMeta()   const { return filename_meta; }


	PICO_VOLTAGE ParseVoltage(char *);
	void ParseAndSetVoltage(char *);
	void SetVoltage(PICO_VOLTAGE v) { voltage = v; };
	PICO_VOLTAGE GetVoltage() const { return voltage; };
	double GetVoltageDouble();

	void ParseAndSetLength(char *);
	void SetLength(unsigned long);
	unsigned long GetLength() const { return length; };

	void ParseAndSetNTraces(char *);
	unsigned long GetNTraces() const { return ntraces; };

	void ParseAndSetTrigger(char *, char *);
	Trigger* GetTrigger(Channel *ch);

	void PrintUsage();
	bool IsJustHelp() const { return is_just_help; };
	bool IsTextOutput() const { return is_text_output; };
	bool IsBinaryOutput() const { return is_binary_output; };

private:
	char *filename, *filename_binary, *filename_text, *filename_meta;
	unsigned long length;
	unsigned long ntraces;
	PICO_VOLTAGE voltage;
	bool is_triggered;
	double x_frac, y_frac;
	bool is_just_help;
	bool is_binary_output, is_text_output;

	PICO_VOLTAGE generator_voltage;
	// unsigned long generator_ns;

	double GetTriggerXFraction() const { return x_frac; }
	double GetTriggerYFraction() const { return y_frac; }
};

#endif
