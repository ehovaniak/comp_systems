/*
 * uint8vec.h
 *
 *  Created on: Apr 28, 2017
 *      Author: evan
 */

#ifndef UINT8VEC_H_
#define UINT8VEC_H_

#include <stdlib.h>
#include <stdint.h>

#define DEFAULT		128
#define FACTOR		2

typedef struct {
	size_t size;
	size_t cap;

	uint8_t* data;
}uint8vec_t;

uint8vec_t* uint8vec_init(void);

void free_uint8vec(uint8vec_t* uintv);

void push_back_uint8(uint8vec_t* uintv, uint8_t data);

#endif /* UINT8VEC_H_ */
