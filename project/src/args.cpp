#include "args.h"
#include "measurement.h"

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
	filename        = NULL;
	filename_binary = NULL;
}

Args::~Args()
{
	if(filename        != NULL) free(filename);
	if(filename_binary != NULL) free(filename_binary);
}

void Args::parse_options(int argc, char** argv)
{
	int i;
	unsigned long ul;

	for(i=1; i<argc; i++) {
		// printf("%s\n", argv[i]);
		switch (lookup_table(&PICO_arguments[0], argv[i])) {
			case PICO_ARG_FILENAME:
				// fprintf(stderr, "  (filename recognized in '%s' '%s')\n", argv[i], argv[i+1]);
				SetFilename(argv[++i]);
				// fprintf(stderr, "  (name: '%s')\n", GetFilename());
				break;
			case PICO_ARG_LENGTH:
				// fprintf(stderr, "  (length recognized in '%s' '%s')\n", argv[i], argv[i+1]);
				ParseAndSetLength(argv[++i]);
				break;
			case PICO_ARG_VOLTAGE:
				// fprintf(stderr, "  (voltage recognized in '%s' '%s')\n", argv[i], argv[i+1]);
				ParseAndSetVoltage(argv[++i]);
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
}

void Args::SetFilename(char *name)
{
	if(filename        != NULL) free(filename);
	if(filename_binary != NULL) free(filename_binary);

	filename        = (char *)malloc(strlen(name)+1);
	filename_binary = (char *)malloc(strlen(name)+4);

	if((filename != NULL) && (filename_binary != NULL)) {
		strcpy(filename, name);
		sprintf(filename_binary, "%s.bin", filename);
	} else {
		throw("Unable to allocate memory.\n");
	}
}

void Args::ParseAndSetVoltage(char *str)
{
	if     (strcmp(str,  "50mV")==0) { voltage = U_50mV;  }
	else if(strcmp(str, "100mV")==0) { voltage = U_100mV; }
	else if(strcmp(str, "200mV")==0) { voltage = U_200mV; }
	else if(strcmp(str, "500mV")==0) { voltage = U_500mV; }
	else if(strcmp(str,    "1V")==0) { voltage = U_1V;    }
	else if(strcmp(str,    "2V")==0) { voltage = U_2V;    }
	else if(strcmp(str,    "5V")==0) { voltage = U_5V;    }
	else if(strcmp(str,   "10V")==0) { voltage = U_10V;   }
	else if(strcmp(str,   "20V")==0) { voltage = U_20V;   }
	else if(strcmp(str,   "50V")==0) { voltage = U_50V;   }
	else if(strcmp(str,  "100V")==0) { voltage = U_100V;  }
	else {
		fprintf(stderr, "WARNING: unknown voltage '%s'\n", str);
		voltage = U_MAX;
	}
}

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

void Args::SetLength(unsigned long l)
{
	length = l;
}

