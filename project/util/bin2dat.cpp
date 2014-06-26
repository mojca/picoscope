#include <cstdio>
#include <vector>
#include <iostream>
#include <fstream>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

using namespace std;

void print_usage()
{
	cout <<
		"USAGE: bin2dat <type> <filename> <length>\n" <<
		"       bin2dat <type> <filename> <length> <start>\n\n" <<
		"where <type> can be { -1 | +1 | -2 | +2 } for\n";
}

// filename_in [+-][12] start stop
// filename_in len
int main(int argc, char **argv)
{
	FILE *f_in;
	// short *buffer;
	char *buffer;
	char filename_in[1000];
	long int i_start, i_len;
	long int i;
	vector<int16_t>  buffer_int16_t;
	vector<uint16_t> buffer_uint16_t;
	vector<int8_t>   buffer_int8_t;
	vector<uint8_t>  buffer_uint8_t;


	if(argc < 3) {
		print_usage();
		return 0;
	} else if(argc > 2 && strcmp(argv[1],"-2")==0) {
		// int16_t *buffer_int16_t;
		strcpy(filename_in,argv[2]);

		if(argc == 4) {
			i_len   = atoi(argv[3]);
			i_start = 0;
		} else if(argc == 5) {
			i_len   = atoi(argv[3]);
			i_start = atoi(argv[2]);
		} else {
			print_usage();
			exit(0);
		}
		ifstream file;
		file.open(filename_in, ios::binary | ios::in);
		if (!file) {
			std::cerr << "Could not open file " << filename_in;
			throw;
		}
		file.seekg (0, ios::end);
		int length = file.tellg();
		length = length/sizeof(buffer_int16_t[0]);
		if(i_len < length) {
			length = i_len;
		}
		buffer_int16_t.resize(i_len);
		file.seekg (0, ios::beg); // TODO
		file.read ((char *)&buffer_int16_t[0], sizeof(buffer_int16_t[0])*i_len);
		file.close();

		for(i=0; i<i_len; i++) {
			printf("%d\n", buffer_int16_t[i]);
		}

		buffer_int16_t.clear();
		file.close();
	} else if(argc > 2 && strcmp(argv[1],"-1")==0) {
		// int16_t *buffer_int16_t;
		strcpy(filename_in,argv[2]);

		if(argc == 4) {
			i_len   = atoi(argv[3]);
			i_start = 0;
		} else if(argc == 5) {
			i_len   = atoi(argv[3]);
			i_start = atoi(argv[2]);
		} else {
			print_usage();
			exit(0);
		}
		ifstream file;
		file.open(filename_in, ios::binary | ios::in);
		if (!file) {
			std::cerr << "Could not open file " << filename_in;
			throw;
		}
		file.seekg (0, ios::end);
		int length = file.tellg();
		length = length/sizeof(buffer_int8_t[0]);
		if(i_len < length) {
			length = i_len;
		}
		buffer_int8_t.resize(i_len);
		file.seekg (0, ios::beg); // TODO
		file.read ((char *)&buffer_int8_t[0], sizeof(buffer_int8_t[0])*i_len);
		file.close();

		for(i=0; i<i_len; i++) {
			printf("%d\n", buffer_int8_t[i]);
		}

		buffer_int8_t.clear();
		file.close();
	} else {

	strcpy(filename_in,argv[1]);
	if(argc == 3) {
		i_len   = atoi(argv[2]);
		i_start = 0;
	} else if(argc == 4) {
		i_len   = atoi(argv[2]);
		i_start = atoi(argv[3]);
	} else {
		print_usage();
		exit(0);
	}

	// printf("%d arguments (%s)\n", argc, filename_in);

	f_in = fopen(filename_in, "rb");
	// buffer = (short *)malloc(sizeof(short)*i_len);
	buffer = (char *)malloc(sizeof(buffer[0])*i_len);

	fseek(f_in,sizeof(buffer[0])*i_start,SEEK_SET);
	fread(buffer, sizeof(buffer[0]), i_len, f_in);

	for(i=0; i<i_len; i++) {
		printf("%d\n", buffer[i]);
	}

	free(buffer);
	fclose(f_in);
	}

	return 0;
}
