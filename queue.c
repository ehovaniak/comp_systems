#include <stdlib.h>
#include <assert.h>
#include <semaphore.h>
#include <pthread.h>
#include <stdio.h>

#include "queue.h"


/*
 * This function initializes a queue structure
 */
queue* 
make_queue()
{
    queue* qq = malloc(sizeof(queue));
    qq->head = 0;
    qq->tail = 0;

    // open the queue
    qq->closed = false;

    // initialize queue mutex and condition variables
    pthread_mutex_init(&qq->mutex, NULL);
    pthread_cond_init(&qq->condv, NULL);
    return qq;
}

/*
 * "free_queue" destroys the queues associated mutex and condition variables
 * before freeing the queue structure.
 */
void 
free_queue(queue* qq)
{
    assert(qq->head == 0 && qq->tail == 0);
    pthread_mutex_destroy(&qq->mutex);
    pthread_cond_destroy(&qq->condv);
    free(qq);
}

/*
 * "queue_put" adds data to a queue structure. Supports multi-threading
 */
void 
queue_put(queue* qq, void* msg)
{
	// create a node for data storage
    qnode* node = malloc(sizeof(qnode));
    node->data = msg;
    node->prev = 0;
    node->next = 0;
    

    // lock the mutex to prevent R/W collisions with other threads
    pthread_mutex_lock(&qq->mutex);

    // Append the new node to the linked list
    node->next = qq->head;
    qq->head = node;

    if (node->next) {
        node->next->prev = node;
    } 
    else {
        qq->tail = node;
    }

    // broadcast to all other nodes that the mutex is free
    pthread_cond_broadcast(&qq->condv);
    // release the mutex
    pthread_mutex_unlock(&qq->mutex);
}

/*
 * "queue_get" pops the first element from the queue. Supports multi-threading.
 */
void* 
queue_get(queue* qq)
{

	// Lock the mutex to prevent R/W collisions with other threads
	pthread_mutex_lock(&qq->mutex);
	// check if current node is last in queue
	while(!qq->tail) {
		if (qq->closed == true) {
			// release the mutex
			pthread_mutex_unlock(&qq->mutex);
			return 0;
		}
		// all threads should wait for the mutex
		pthread_cond_wait(&qq->condv, &qq->mutex);
	}

	// pop current node
    qnode* node = qq->tail;

    // adjust tail node
    if (node->prev) {
        qq->tail = node->prev;
        node->prev->next = 0;
    }
    else {
        qq->head = 0;
        qq->tail = 0;
    }

    // save node data and free node
    void* msg = node->data;
    free(node);

    // release the mutex
    pthread_mutex_unlock(&qq->mutex);
    return msg;
}

/*
 * "queue_close" sets a flag within the queue structure to signal
 * the queue is closed and no more data may be entered
 */
void queue_close(queue* qq) {
	// Lock the mutex to prevent R/W collisions with other threads
	pthread_mutex_lock(&qq->mutex);
	// set flag
	qq->closed = true;
	// broadcast to all other nodes that the mutex is free
	pthread_cond_broadcast(&qq->condv);
	// release the mutex
	pthread_mutex_unlock(&qq->mutex);
}

/*
 * "queue_open" sets a flag within the queue structure to signal
 * the queue is open and data may be entered
 */
void queue_open(queue* qq) {
	// Lock the mutex to prevent R/W collisions with other threads
	pthread_mutex_lock(&qq->mutex);
	// set flag
	qq->closed = false;
	// broadcast to all other nodes that the mutex is free
	pthread_cond_broadcast(&qq->condv);
	// release the mutex
	pthread_mutex_unlock(&qq->mutex);
}
