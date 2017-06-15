/*
 * uint8vec.c
 *
 *  Created on: Apr 28, 2017
 *      Author: evan
 *
 * This library is for a dynamic vector class in C.
 * User is able to append new entries to an array,
 * increase, and/or decrease the arrays size
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "uint8vec.h"

/*
 * Creates an empty vector with capacity "DEFAULT" defined in "uint8vec.h"
 */
uint8vec_t* uint8vec_init(void) {
	// allocate space for vector
	uint8vec_t* uintv = malloc(sizeof(uint8vec_t));

	// initialize vector
	uintv->size = 0;
	uintv->cap = DEFAULT;
	uintv->data = malloc(uintv->cap * sizeof(uint8_t));

	return uintv;
}

/*
 * releases the dynamic vector back to the heap
 */
void free_uint8vec(uint8vec_t* uintv) {
	free(uintv->data);
	free(uintv);
}

/*
 * appends new data to the vector, dynamically adjusting size if necessary
 */
void push_back_uint8(uint8vec_t* uintv, uint8_t data) {
	size_t curr_cap = uintv->cap;
	size_t curr_samples = uintv->size;

	// reallocate if vector is too small for new entry
	if (curr_samples >= curr_cap) {
		size_t new_cap = curr_cap * FACTOR;

		uint8_t* new_data = (uint8_t*)realloc(uintv->data, new_cap * sizeof(uint8_t));

		if (new_data == NULL)
			printf("REALLOC FAILED!!\n");

		uintv->cap = new_cap;
		uintv->data = new_data;
	}

	// append new entry to end of vector
	uintv->data[uintv->size] = data;
	uintv->size++;

	return;
}
