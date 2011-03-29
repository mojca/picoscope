#include "args.h"
#include "measurement.h"
#include "trigger.h"

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <cstddef>

int lookup_table(const struct gen_table *tbl, const char *opt)
{
	if(opt[0]=='-' && opt[1]=='-') {
		while (tbl->key) {
			if (strcmp(opt+2, tbl->key)==0) {
				return tbl->value;
			}
			tbl++;
		}
		return tbl->value;
	} else {
		return PICO_NOT_ARG;
	}
}

Args::Args()
{
	SetLength(0);
	SetVoltage(U_MAX);
	ntraces          = 1;
	filename         = NULL;
	filename_binary  = NULL;
	is_just_help     = false;
	is_binary_output = false;
	is_text_output   = false;
}

Args::~Args()
{
	if(filename        != NULL) free(filename);
	if(filename_binary != NULL) free(filename_binary);
}

void Args::PrintUsage()
{
	std::cout << "run_picoscope.exe [--help | <options>]\n";
	std::cout << "  (obligatory options)\n";
	std::cout << "    --name <str>  # filename without extention (ext. is added automatically)\n";
	std::cout << "    --l <number> | --length <number>   # length of single trace\n";
	std::cout << "    --U <str> | --voltage <str>        # voltage range\n";
	std::cout << "      allowed values: 50mV, 100mV, 200mV, 500mV, 1V, 2V, 5V, 10V, 20V\n";
	std::cout << "\n";
	std::cout << "    --bin | --binary                   # save <name>.bin in binary format\n";
	std::cout << "    --dat | --text                     # save <name>.dat in text format\n";
//	std::cout << "    --ch <str> | --channel <str>       # list of channels, example: acd\n";
	std::cout << "\n";
	std::cout << "  only for triggered events:\n";
	std::cout << "    --trig <x> <y> | --trigger <x> <y> # real number for x and y\n";
	std::cout << "      # x between (0,1) represents trigger point on x axis\n";
	std::cout << "      # y between (-1,1) represents trigger point on y axis and implies direction\n";
	std::cout << "      # y < 0 triggers on falling signal; y > 0 on raising signal\n";
	std::cout << "    --n <number>                       # number of traces\n";
}

void Args::parse_options(int argc, char** argv)
{
	int i;
	unsigned long ul;

	if(argc==1) {
		is_just_help = true;
		PrintUsage();
	}

	for(i=1; i<argc; i++) {
		// printf("%s\n", argv[i]);
		switch (lookup_table(&PICO_arguments[0], argv[i])) {
			case PICO_ARG_HELP:
				is_just_help = true;
				PrintUsage();
				i = argc;
				break;
			case PICO_ARG_BINARY:
				is_binary_output = true;
				break;
			case PICO_ARG_TEXT:
				is_text_output = true;
				break;
			case PICO_ARG_FILENAME:
				// fprintf(stderr, "  (filename recognized in '%s' '%s')\n", argv[i], argv[i+1]);
				SetFilename(argv[++i]);
				// fprintf(stderr, "  (name: '%s')\n", GetFilename());
				break;
			case PICO_ARG_LENGTH:
				// fprintf(stderr, "  (length recognized in '%s' '%s')\n", argv[i], argv[i+1]);
				ParseAndSetLength(argv[++i]);
				break;
			case PICO_ARG_NTRACES:
				ParseAndSetNTraces(argv[++i]);
				break;
			case PICO_ARG_VOLTAGE:
				// fprintf(stderr, "  (voltage recognized in '%s' '%s')\n", argv[i], argv[i+1]);
				ParseAndSetVoltage(argv[++i]);
				break;
			case PICO_ARG_TRIGGER:
				ParseAndSetTrigger(argv[i+1], argv[i+2]);
				i+=2;
				break;
			case PICO_ARG_SIGNAL_SQUARE:
				// ParseAndSetSignalGeneratorVoltage(argv[i+1]);
				// ParseAndSetSignalGeneratorTime(argv[i+2]);
				i+=2;
				break;
			case PICO_NOT_ARG:
				fprintf(stderr, "WARNING: this is not an argument '%s'\n", argv[i]);
				throw;
				break;
			default:
				fprintf(stderr, "  (invalid option '%s')\n", argv[i]);
				throw;
				break;
		}
	}

	// default output is text
	if(!IsBinaryOutput() && !IsTextOutput()) {
		is_text_output = true;
	}
}

void Args::SetFilename(char *name)
{
	if(filename        != NULL) free(filename);
	if(filename_binary != NULL) free(filename_binary);
	if(filename_text   != NULL) free(filename_text);
	if(filename_meta   != NULL) free(filename_meta);

	filename        = (char *)malloc(strlen(name)+1);
	filename_binary = (char *)malloc(strlen(name)+4);
	filename_text   = (char *)malloc(strlen(name)+4);
	filename_meta   = (char *)malloc(strlen(name)+4);

	if((filename != NULL) && (filename_binary != NULL) && (filename_text != NULL) && (filename_meta != NULL)) {
		strcpy(filename, name);
		sprintf(filename_binary, "%s.bin", filename);
		sprintf(filename_text,   "%s.dat", filename);
		sprintf(filename_meta,   "%s.txt", filename);
	} else {
		throw("Unable to allocate memory.\n");
	}
}

PICO_VOLTAGE Args::ParseVoltage(char *str)
{
	if     (strcmp(str,  "50mV")==0) { return U_50mV;  }
	else if(strcmp(str, "100mV")==0) { return U_100mV; }
	else if(strcmp(str, "200mV")==0) { return U_200mV; }
	else if(strcmp(str, "500mV")==0) { return U_500mV; }
	else if(strcmp(str,    "1V")==0) { return U_1V;    }
	else if(strcmp(str,    "2V")==0) { return U_2V;    }
	else if(strcmp(str,    "5V")==0) { return U_5V;    }
	else if(strcmp(str,   "10V")==0) { return U_10V;   }
	else if(strcmp(str,   "20V")==0) { return U_20V;   }
	else if(strcmp(str,   "50V")==0) { return U_50V;   }
	else if(strcmp(str,  "100V")==0) { return U_100V;  }
	else {
		fprintf(stderr, "WARNING: unknown voltage '%s'\n", str);
		return U_MAX;
	}
}

void Args::ParseAndSetVoltage(char *str)
{
	voltage = ParseVoltage(str);
}

double Args::GetVoltageDouble()
{
	switch(voltage) {
		case U_50mV:  return   0.05;
		case U_100mV: return   0.1;
		case U_200mV: return   0.2;
		case U_500mV: return   0.5;
		case U_1V:    return   1;
		case U_2V:    return   2;
		case U_5V:    return   5;
		case U_10V:   return  10;
		case U_20V:   return  20;
		case U_50V:   return  50;
		case U_100V:  return 100;
	}
	return 0;
}


// void Args::ParseAndSetSignalGeneratorVoltage(char *str)
// {
// 	generator_voltage = ParseVoltage(str);
// }


// ParseAndSetSignalGeneratorTime(argv[i+2]);

// TODO: fix this parser
void Args::ParseAndSetLength(char *str_orig)
{
	int i, str_len;
	char str[32];
	unsigned long len_u;
	double len_d, multiply = 1;

	strncpy(str, str_orig, 31);

	str_len = strlen(str);
	for(i=0; ((str[i]>='0' && str[i]<='9') || (str[i]=='.') || (str[i]=='-') || str[i]=='e' || str[i]=='E'); i++);

	if(str[i]=='k') {
		multiply = 1000;
	} else if(str[i]=='M') {
		multiply = 1000000;
	} else if(str[i]=='G') {
		multiply = 1000000000;
	}
	if(multiply > 10) {
		str[i]=0;
		if(sscanf(str, "%lf", &len_d)==1) {
			length = (unsigned long)lround(multiply*len_d);
			fprintf(stderr, "number is: %lu\n", length);
		} else {
			length = 0;
			fprintf(stderr, "unable to read number (%s)\n", str_orig);
		}
	} else {
		if(sscanf(str, "%lu", &length)==1) {
			fprintf(stderr, "number is: %lu\n", length);
		} else {
			length = 0;
			fprintf(stderr, "unable to read number (%s)\n", str_orig);
		}
	}
}

void Args::ParseAndSetNTraces(char *str)
{
	ntraces = (unsigned long)atoi(str);
	if(ntraces == 0) {
		throw "something is wrong; there are no traces to fetch.\n";
	} else {
		std::cerr << "    (fetching multiple traces: " << ntraces << ")\n";
	}
}

void Args::ParseAndSetTrigger(char *strx, char *stry)
{
	is_triggered = true;
	x_frac = atof(strx);
	y_frac = atof(stry);

	if(x_frac < 0 || x_frac > 1) {
		throw "--trigger <xfrac> <yfrac>: xfrac has to be between 0 and 1";
	}
	if(y_frac < -1 || y_frac > 1) {
		throw "--trigger <xfrac> <yfrac>: yfrac has to be between -1 and 1";
	}
}

Trigger* Args::GetTrigger(Channel *ch)
{
	Trigger *tr = new Trigger(ch,GetTriggerXFraction(), GetTriggerYFraction());
	return tr;
}

void Args::SetLength(unsigned long l)
{
	length = l;
}
