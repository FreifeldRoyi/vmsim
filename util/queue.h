#ifndef QUEUE_H_
#define QUEUE_H_

#include "list.h"

struct _queue_t;

/**
 * Queue initialization.
 * Returns a pointer to the head of the queue.
 */
struct _queue_t* queue_init();

/**
 * Returns the head of the queue's data and dequeues the node
 */
void* queue_pop(struct _queue_t* item);

/**
 * Inserts a new queue item to the end of the queue
 * Returns 1 if item is NULL
 * 0 if insert succeeds or -1 if insert fails
 */
int queue_push(struct _queue_t* queue, void* item);

void queue_for_each(struct _queue_t* queue, element_func_t func);

/**
 * Returns the number of items in the queue
 *
 */
int queue_size(struct _queue_t* item);

/**
 * Will free all memory allocated in the queue.
 * Returns 0 if the queue is empty and all memory was free
 * or -1 if freeing the memory fails or the queue isn't empty
 */
int queue_destroy(struct _queue_t* dest);

#endif /* QUEUE_H_ */
