This branch contains code for a process safe data queue. Queue was tested using 128bit factoring method provided.

Implimentation uses process handled by the OS direclty for parallelism. Upon read/write operations, a process will attempt to grab the semiphore associated with a queue. If successful, the thread will continue with execution. If the semiphore is not free, then the process wait for the semiphore to become free. Atomic operations are used to prevent race conditions and ensure data concurrency.

CODE I WROTE/MODIFIED:
- queue.c
- queue.h
- factor.c (PARTS!)

ALL ELSE PROVIDED!
