/*
 * data_proc.c
 *
 *  Created on: Apr 25, 2017
 *      Author: evan
 */


#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <pthread.h>
#include <time.h>
#include <stdatomic.h>
#include <unistd.h>

#include "data_proc.h"

#define FS					400

static int stop = 0;
static int interrupt = 0;

// global buffer for data transaction
uint8_t raw_buf[BUFFER_SIZE * sizeof(accel_sample_t)];
// global buffer for temporary sample storage and processing
accel_sample_t accel_samples[BUFFER_SIZE + OUTPUT_SIZE];

/*
 * Interrupt function.
 *
 * Toggles a global variable to act as a simulated interrupt
 * line from a virtual sensor. Determines a 'dt' based on
 * sampling frequency defined above. When new time reaches just
 * above that 'dt', the interrupt variable is set and the timer reset.
 * It is up to 'MCU' to clear the interrupt line.
 *
 * CALLED BY A CHILD THREAD!
 *
 * I wrote this to simulate a typical sensor configuration. A sensor
 * will sample and will toggle an interrupt line when a sample(s) are
 * ready to be read. The main processor will see this, wake up, and pull
 * the samples from the sensor. The interrupt is cleared and the process
 * repeats.
 */
static void* toggle_interrupt(void) {

	// calculate dt
	double time_diff = 1.0 / FS;

	// initalize start time
	clock_t start, end;
	start = clock();

	/*
	 * Continuous loop while there is still data to be read
	 * Global variables are read and written atomically. This
	 * ensures continuity between this interrupt thread and the
	 * main processor execution.
	 */
	while(!atomic_load(&stop)) {
		//pull current clock time
		end = clock();

		// calculate dt
		double diff = (double)(end - start) / CLOCKS_PER_SEC;
		//toggle interrupt if 'dt' has passed and reset timer
		if ( diff >= time_diff) {
			atomic_store(&interrupt, 1);
			start = clock();
		}
	}

	return NULL;

}

/*
 * 'read_data()' is simply a virtual SPI/I2C/other comms transfer function.
 *
 * Function clears the output buffer then copies data from the input buffer
 * to the output buffer, byte for byte.
 *
 * This is not strictly necessary, but is part of my interrupt simulation scheme.
 * As an interrupt is triggered, data is copied from the sensor (heap memory) to
 * a local stack buffer. This ensures data is locally stored on system prior to data
 * processing. This is done for two reasons:
 *
 * 1) In life, it is not a good idea to be processing data while communication between
 *    peripherals is open. Copying to a local buffer prevents this.
 *
 * 2) Read/Write operations are faster on stack pointers than locations in heap memory.
 *    For both my simulation and in practice, operating on data stored on the stack is more
 *    efficient.
 */
static void read_data(uint8_t* in, uint8_t* out, uint32_t num_bytes) {

	/*
	 * Clear the buffer of previous data
	 *
	 * This step may be unnecessary in practice. I would usually copy fixed 'BUFFER_SIZE'
	 * with each transaction, but the provided test files do not contain a multiple of BUFFER_SIZE.
	 * Copying a fixed amount with each transaction would simply overwrite any old data. Clearing
	 * would not be necessary.
	 */
	memset(out, 0, BUFFER_SIZE * sizeof(accel_sample_t));

	// copy byte for byte. I chose to do this as a for loop to explicitly show the virtual communication
	// between a processor and its peripheral. This is done byte for byte on a hardware level.
	for(uint32_t i = 0; i < num_bytes; i++)
		out[i] = in[i];
}


/*
 * This callback function takes a pointer to raw bytes and stores them into another buffer.
 * In this case, this callback copies all raw bytes from the virtual sensor and stores them
 * into a accel_sample_t stack buffer. Now the raw bytes are discrete 12-bit samples.
 */
static void accel_sample_buffer_callback(uint8_t* raw,
							accel_sample_t* samples,
							uint32_t num_samples) {

	// Clear the buffer of previous data
	memset(samples, 0, BUFFER_SIZE * sizeof(accel_sample_t));

	/* Copy bytes from the raw buffer into the accel_sample_t array.
	 *
	 * 'accel_sample_t' is a packed structure allowing easy byte for byte
	 * transfers while retaining structural meaning and significance. This
	 * saves space in storage while allowing flexibility in function.
	 */

	memcpy((uint8_t*)samples, raw, num_samples * sizeof(accel_sample_t));

}

/*
 * Insertion Sort algorithm for sorting data sets.
 *
 * I chose to use a simple Insertion Sort algorithm for a few reasons:
 *
 * 1) Sort in place. Insertion Sort can sort in place. This can be useful
 *    in an embedded system as the stack/heap doesn't need to grow for
 *    successive calls, array copies, etc. that many other algorithms employ.
 *
 * 2) Simplicity. Insertion Sort is a very simple algorithm with few comparisons
 *    and arithmetic. On small data sets it can be more or as efficient as other
 *    algorithms. Data sets I am operating on are <= 64 samples, depending on
 *    size of buffer.
 *
 */
static void insertion_sort(uint16_t* list, uint32_t num) {
	uint16_t key;
	uint32_t j;


	for (uint32_t i = 1; i < num; i++) {
		key = list[i];
		j = i - 1;

		while(j < UINT32_MAX && list[j] > key) {
			list[j+1] = list[j];
			j--;
		}
		list[j+1] = key;
	}

}

/*
 * last_update simply shifts old data over by 'samples' and copies the new
 * data to the desired array.
 */
static void last_update(uint16_t* new, uint16_t* old, uint32_t samples) {
	memmove(old, &old[samples], (OUTPUT_SIZE - samples) * sizeof(uint16_t));
	memcpy(&old[OUTPUT_SIZE - samples], new, samples * sizeof(uint16_t));

}

/*
 * find_maxs Implements the virtual sensor interrupt system and determines
 * the max and last values as specified.
 *
 * Flow:
 *
 * 1) Interrupt thread spawned and Interrupt system started
 *    - A child thread is spawned to toggle an interrupt line
 *      every dt = 1.0 / FS seconds.
 *
 * 2) Interrupt checking.
 *    - A loop continuously polls the interrupt line. If set,
 *      then data is pulled from heap.
 *
 * 3) Data transferred from sensor (heap)
 *    - Interrupt cleared so sensor (child thread) can continue operating.
 *    - Data is copied from sensor to global buffer
 *
 * 4) Callback
 *    - Once data is pulled from sensor, it is converted into accel samples
 *      (accel_sample_t) through a callback.
 *
 * 5) Update last read list
 *    - copy newly read samples to list of previously read samples
 *
 * 6) Sort data
 *    - Sort newly transferred samples with previous list of 32. The bottom half
 *      of the buffer is always the 32 greatest values with new data being copied
 *      to the top half. Half of the buffer is therefore always sorted.
 *
 * 7) Check if out of data.
 *    - Set done flag if out of data. This tells the interrupt thread to break.
 *
 * 8) Copy max_list data to output buffer.
 *    - The last 32 samples of the accel_samples buffer is the maximum 32 samples
 *
 * 9) Properly merge child thread
 */
void find_maxs(uint8_t* input,
				uint32_t num_bytes,
				uint16_t* max_output,
				uint16_t* last_output) {

	pthread_t int_thread;

	// spawn and initialize interrupt thread
	int err = pthread_create(&int_thread, 0, toggle_interrupt, NULL);
	if (err) {
		printf("pthread_create failed!\n");
		exit(EXIT_FAILURE);
	}

	// book keeping variables for keeping track of how much data left to load form input
	uint32_t transaction_size;
	uint32_t sample_size;

	/*
	 * Continuously check interrupt status. On a real system, the interrupt actually
	 * triggers an event with the host system. For simplicity I constantly check it.
	 */
	while(1) {
		if (atomic_load(&interrupt)) {

			// calculate how much data to copy from input set
			transaction_size = BUFFER_SIZE * sizeof(accel_sample_t);
			if (transaction_size  > num_bytes)
				transaction_size = num_bytes;

			sample_size = transaction_size / sizeof(accel_sample_t);

			// clear interrupt and read data from sensor peripheral (heap)
			atomic_store(&interrupt, 0);
			/*
			 * These buffers are declared globally, but I prefer to specify them in
			 * when callin these functions. Helps to follow execution flow and allows
			 * for more general function design.
			 */
			read_data(input, raw_buf, transaction_size);

			// book keeping
			input += transaction_size;
			num_bytes -= transaction_size;

			// copy bytes to samples
			accel_sample_buffer_callback(raw_buf, accel_samples, sample_size);

			// update last read list
			last_update((uint16_t*)accel_samples, last_output, sample_size);

			// sort newly copied data with previous list
			insertion_sort((uint16_t*)accel_samples, BUFFER_SIZE + OUTPUT_SIZE);

			// sleep. This was mainly for testing of my interrupt scheme.
			usleep(0);
		}

		// set 'done' flag if out of data
		if (num_bytes == 0) {
			atomic_store(&stop, 1);
			break;
		}
	}

	// copy max data to output buffer
	memcpy(max_output, &accel_samples[BUFFER_SIZE], OUTPUT_SIZE * sizeof(accel_sample_t));

	// properly join child thread
	err = pthread_join(int_thread, NULL);
	if (err) {
		printf("pthread_join failed!\n");
		exit(EXIT_FAILURE);
	}

	return;
}
