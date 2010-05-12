#include "queue.h"
#include "list.h"

#include <stdlib.h>
#include <assert.h>

/* queue_t macros*/
#define QUEUE(x) ((queue_t *) (x))
#define QUEUE_HEAD(x) QUEUE((x)) -> head
#define QUEUE_TAIL(x) QUEUE((x)) -> tail
#define QUEUE_SIZE(x) QUEUE((x)) -> size


typedef struct _queue_t
{
	node_t* head;
	node_t* tail;
	int size;
} queue_t;


queue_t* queue_init()
{
	queue_t* toReturn = (queue_t*)malloc(sizeof(queue_t));
	assert(toReturn != NULL);
	QUEUE_HEAD(toReturn) = NULL;
	QUEUE_TAIL(toReturn) = NULL;
	QUEUE_SIZE(toReturn) = 0;

	return toReturn;
}

void* queue_pop(queue_t* item)
{
	node_t* nodeToPop = NULL;
	void* toReturn = NULL;

	if (QUEUE_SIZE(item) > 0)
	{
		nodeToPop = QUEUE_HEAD(item);

		if (nodeToPop == QUEUE_TAIL(item))
			QUEUE_TAIL(item) = NULL;

		QUEUE_HEAD(item) = NODE_NEXT(QUEUE_HEAD(item));
		toReturn = NODE_DATA(nodeToPop);
		node_destroy(nodeToPop,NULL);
		--QUEUE_SIZE(item);
	}

	return toReturn;
}

int queue_push(queue_t* queue, void* item)
{
	int toReturn = 0;
	node_t* toPush = node_init();
	node_set_data(toPush, item);

	toReturn = node_set_next(QUEUE_TAIL(queue), toPush);
	if (toReturn == 1)
	{
		QUEUE_HEAD(queue) = toPush;
	}
	QUEUE_TAIL(queue) = toPush;
	++QUEUE_SIZE(queue);

	return toReturn;
}

void queue_for_each(struct _queue_t* queue, element_func_t func)
{
	node_t *cur_node = QUEUE_HEAD(queue);

	while (cur_node != NULL)
	{
		func(NODE_DATA(cur_node));
		cur_node = NODE_NEXT(cur_node);
	}
}

int queue_size(queue_t* item)
{
	return QUEUE_SIZE(item);
}

int queue_destroy(queue_t* dest)
{
	int toReturn = -1;

	if (QUEUE_SIZE(dest) == 0)
	{
		free(dest);
		toReturn = 0;
	}

	return toReturn;
}

#include "tests/queue_tests.c"
