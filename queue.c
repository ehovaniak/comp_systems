#include <stdlib.h>
#include <assert.h>
#include <sys/mman.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdatomic.h>

#include "queue.h"

// TODO: Make this an interprocess queue.

#define PAGE_SIZE	4096;

/*
 * "make_queue" initializes a queue structure and allocates an
 * interprocess shared memory space for the queue
 */
queue*
make_queue()
{
	// calculate required number of pages
	int page_size = PAGE_SIZE;
    int pages = 1 + sizeof(queue) / page_size;
    // FIXME: Queue should be shared.

    // map shared memory space for queue directly from OS
    queue* qq = mmap(0, pages * page_size, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    qq->qii = 0;
    qq->qjj = 0;

    // initialize input and output queue semiphores
    sem_init(&(qq->isem), 1, QUEUE_SIZE);
    sem_init(&(qq->osem), 1, 0);
    return qq;
}

/*
 * "free_queue" releases the mapped memory space back to the OS
 */
void
free_queue(queue* qq)
{
    assert(qq->qii == qq->qjj);

    // determine number of pages to be released
    int page_size = PAGE_SIZE;
    int pages = 1 + sizeof(queue) / page_size;

    // release memory space to OS
    munmap(qq, pages * page_size);
}

/*
 * "queue_put" places data "msg" into queue "qq".
 * Inter-process safe
 */
void
queue_put(queue* qq, job msg)
{
	// wait for the input semiphore to be free
    int rv = sem_wait(&(qq->isem));
    assert(rv == 0);

    // append "msg" to the queue.
    // Atomic operation ensures concurrency in presence of race condition
	unsigned int ii = atomic_fetch_add(&(qq->qii), 1);
    qq->jobs[ii % QUEUE_SIZE] = msg;

    // alert other processes they may proceed on "qq->osem"
    rv = sem_post(&(qq->osem));
    assert(rv == 0);

    return;
}

/*
 * "queue_get" retrieves data from queue "qq"
 * Inter-process safe
 */
job
queue_get(queue* qq)
{
	// wait for output semiphore to be free
	int rv = sem_wait(&(qq->osem));
	assert(rv == 0);

	// retrieve data from queue
	// Atomic operation ensures concurrency in presence of race condition
    unsigned int jj = atomic_fetch_add(&(qq->qjj), 1);
    job pop = qq->jobs[jj % QUEUE_SIZE];

    // alert other processes they proceed on "qq->isem"
    rv = sem_post(&(qq->isem));
    assert(rv == 0);

    return pop;
}

