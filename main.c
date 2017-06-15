/*
 * main.c
 *
 *  Created on: Apr 25, 2017
 *      Author: evan
 *
 *      main.c is the main program for the challenge.
 *      main.c primarily handles IO between the test files provided
 *      and the actual data processing handled in "data_proc.c/h".
 *
 *
 */


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <time.h>

#include "uint8vec.h"
#include "data_proc.h"

int main(int argc, char** argv) {
	// print proper usage and return
	if (argc != 3) {
		printf("./process 'data'.bin 'output'.txt\n");
		return 0;
	}

	// create a dynamic 1-byte vector for file IO
	uint8vec_t* uintv = uint8vec_init();

	// open specified file
	FILE* infile = fopen(argv[1], "rb");
	if (infile == NULL) {
		printf("Unable to open file!\n");
		return 0;
	}
	// placeholder values for parsing file data
	int c1, c2, c3;

	//srand(time(NULL));

	//int count = 0;

	/* loop for file input
	 * Most significant 4 bits of sample are parsed into byte 1
	 * Lease significant 8 bits of sample are parsed into byte 2
	 * Repeat
	 *
	 * Bytes are pushed in order into dynamic uint8_t vector in library "uint8vec.h"
	 * This allows variable storage without over-declaring a large array on stack.
	 * Also allows the vector object to contain the number of bytes/samples it holds.
	 * I do this to load the data locally and NOT run operations with an open file pointer
	 *
	 * The byte order I push into the vector may seem reversed as Intel machine stores
	 * arrays in little Endian format. Needed to reverse the byte order for proper handling.
	 */
	while(1) {

		// grab a byte from the file
		c1 = fgetc(infile);
		if (c1 == EOF)		// If the corresponds to EOF, then break loop
			break;

		// grab next byte
		c2 = fgetc(infile);
		if (c2 == EOF)
			break;


		/* parse the first complete word.
		 *
		 * byte 1: bxxxx0101
		 * byte 2: b01010101
		 */
		uint8_t temp = ((uint8_t)c1 << 4) | ((uint8_t)c2 >> 4);
		push_back_uint8(uintv, temp);

		temp = (uint8_t)c1 >> 4;
		push_back_uint8(uintv, temp);

		// grab next byte
		c3 = fgetc(infile);
		if (c3 == EOF)
			break;

		// parse the second complete word
		temp = (uint8_t)c3;
		push_back_uint8(uintv, temp);

		temp = ((uint8_t)c2 & 0x0F);
		push_back_uint8(uintv, temp);

		//uint16_t num = (uint16_t)rand();
		//push_back_uint8(uintv, (uint8_t)num);
		//push_back_uint8(uintv, num & 0x0F00);


		//count++;
	}
	fclose(infile);




	// declare and initialize output buffers
	uint16_t max_list[OUTPUT_SIZE];
	uint16_t last_list[OUTPUT_SIZE];
	memset(max_list, 0, OUTPUT_SIZE * sizeof(accel_sample_t));
	memset(last_list, 0, OUTPUT_SIZE * sizeof(accel_sample_t));

	// run data processing challenge
	find_maxs(uintv->data, uintv->size, max_list, last_list);
	// open specified output file
	FILE* outfile = fopen(argv[2], "w");
	if (infile == NULL) {
		printf("Unable to open file!\n");
		return 0;
	}

	// write data to specified output file
	fprintf(outfile, "--Sorted Max 32 Values--\n");
	uint32_t size = OUTPUT_SIZE;
	if (uintv->size / sizeof(accel_sample_t) < OUTPUT_SIZE)
		size = uintv->size / sizeof(accel_sample_t);
	for (uint32_t i = OUTPUT_SIZE - size; i < OUTPUT_SIZE; i++)
		fprintf(outfile, "%d\n", max_list[i]);
	fprintf(outfile, "--Last 32 Values--\n");
	for (uint32_t i = OUTPUT_SIZE - size; i < OUTPUT_SIZE; i++)
		fprintf(outfile, "%d\n", last_list[i]);
	fclose(outfile);

	free_uint8vec(uintv);

	return 0;
}
