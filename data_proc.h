/*
 * data_proc.h
 *
 *  Created on: Apr 25, 2017
 *      Author: evan
 */

#ifndef DATA_PROC_H_
#define DATA_PROC_H_

#include <stdint.h>


#define OUTPUT_SIZE			32		// size of desired output list
#define BUFFER_SIZE			32		// size of buffer used for transactions

/* accel_sample_t structure
 *
 * structure used for storing individual accel sample data.
 * In a typical system one accel sample will be composed of XYZ
 * portions. These are themselves a sample each. You could declare
 * more attributes to this sample structure for each X Y Z sample of
 * 12-bits wide for each.
 *
 * EXAMPLE:
 * typedef struct __attribute__((packed)) {
 *     uint16_t x;
 *     uint16_t y;
 *     uint16_t z;
 * } accel_sample_t;
 *
 * This would effectively store all data associated with a simple accel sample
 *
 * This also takes advantage of the 'packed' attribute. Packed will force each
 * attribute to be aligned immediately after another. This would make it efficient
 * and simply to copy data from the sensor to an array of these structs in a callback
 * function, but still able to access each channel individually when desired.
 *
 * Since the test files didn't specify, the struct is only composed of a single uint16_t,
 * since the sample size is 12-bits wide.
 */
typedef struct __attribute__((packed)) {
	uint16_t bytes;
} accel_sample_t;

void find_maxs(uint8_t* input,
		uint32_t num_bytes,
		uint16_t* max_output,
		uint16_t* last_output);

#endif /* DATA_PROC_H_ */
