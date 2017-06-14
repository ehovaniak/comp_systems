This branch contains code for a thread safe data queue. Queue was tested using 128bit factoring method
provided.

Implimentation uses pthread library to create threads. Upon read/write operations, a thread will
attempt to grab the mutex associated with a queue. If successful, the thread will continue with
execution. If the mutex is not free, then the threads wait for the mutex to become free. This 
mechanism prevents race conditions while allowing pseudo parallel operation.

CODE I WROTE/MODIFIED:
- queue.c
- queue.h

ALL ELSE PROVIDED!
