#ifndef __ARGS_H__
#define __ARGS_H__

// TODO: rewrite the code into C++
#include "measurement.h"

#include <cstddef>

class Args;

struct gen_table {
	const char *key;
	int value;
};

enum PICO_args {
	PICO_ARG_LENGTH,   // --length
	PICO_ARG_CHANNEL,  // --ch
	PICO_ARG_FILENAME, // --name
	PICO_ARG_VOLTAGE,  // --voltage
	PICO_ARG_OTHER,
	PICO_ARG_UNKNOWN,
	PICO_NOT_ARG
};

static struct gen_table PICO_arguments[] = {
	{ "l",       PICO_ARG_LENGTH   },
	{ "length",  PICO_ARG_LENGTH   },
	{ "ch",      PICO_ARG_CHANNEL  },
	{ "channel", PICO_ARG_CHANNEL  },
	{ "U",       PICO_ARG_VOLTAGE  },
	{ "voltage", PICO_ARG_VOLTAGE  },
	{ "name",    PICO_ARG_FILENAME },
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
	char* GetFilename() { return filename; }
	char* GetFilenameBinary() { return filename_binary; }


	void ParseAndSetVoltage(char *);
	void SetVoltage(PICO_VOLTAGE v) { voltage = v; };
	PICO_VOLTAGE GetVoltage() const { return voltage; };

	void ParseAndSetLength(char *);
	void SetLength(unsigned long);
	unsigned long GetLength() const { return length; };

private:
	char *filename, *filename_binary;
	unsigned long length;
	PICO_VOLTAGE voltage;
};

#endif
