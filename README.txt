This branch contains code for a simulated data collection system with an ISR (Interrupt Service Routine) I wrote for a project.

The "main" program loads byte sample data from the raw .bin files included and parses them. Each file
contains a list of 12-bit sensor sample data. "main" parses this binary string into the sampls and 
pushes each 12-bit sample to a dynamic vector. I used this dynamic vector to handle an arbitrary 
number of input samples.

Within the actual process function is the simulated ISR. I spawn two threads to handle this 
simulation. Thread 1 functions as a simple Interrupt line you would typically see in hardware. I define a sampling frequency "fs" and every "1/fs" seconds a global toggle is set. I use atomic operations to handle this so there is no race condition for the other thread.

Thread 2 functions as the actual processor. When it detects the global interrupt has been set, 
thread 2 begins execution. First, thread 1 grabs each data from the dynamic vector, byte for byte, and stores the data in a global buffer. This simulates pulling data from an external sensor via a SPI or I2C bus when the sensor has triggered an interrupt.

Second, when the data has been loaded locally, thread 2 begins parsing each byte in the buffer into 
discrete 12-bit words, stored as 2 byte words with the most significant 4 bits set to 0. These samples are saved into another buffer declared as a custom "sample_t" through a callback function. This is arguably an extra step, but now the raw btyes have physical meaning and can be manipulated 
appropriately. The "sample_t" structure is declared "packed" to save space as each sample occupies 
only 2 bytes when modern processors are 4 byte machines.

Third, thread 2 sorts the samples stored in the sample buffer from least to greates using a quicksort 
algorithm. I chose quicksort because it is fast on small data sets and space effecient.

