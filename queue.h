#ifndef QUEUE_H
#define QUEUE_H

#include <pthread.h>
#include <stdbool.h>

typedef struct qnode {
    void*         data;
    struct qnode* prev;
    struct qnode* next;
} qnode;

typedef struct queue {
    qnode* head;
    qnode* tail;
    bool closed;			// flag to open or close the queue to new data
    pthread_mutex_t mutex;	// queue specific mutex variable
    pthread_cond_t condv;	// queue specific condv variable
} queue;

queue* make_queue();
void free_queue(queue* qq);

void queue_put(queue* qq, void* msg);
void* queue_get(queue* qq);

void queue_close(queue* qq);
void queue_open(queue* qq);

#endif
