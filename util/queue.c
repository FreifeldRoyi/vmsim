#include "queue.h"

#include <stdlib.h>
#include <assert.h>

/* queue_t macros*/
#define QUEUE(x) ((queue_t *) (x))
#define QUEUE_HEAD(x) QUEUE((x)) -> head
#define QUEUE_TAIL(x) QUEUE((x)) -> tail
#define QUEUE_SIZE(x) QUEUE((x)) -> size

/* node_t macros*/
#define NODE(x) ((node_t *) (x))
#define NODE_DATA(x) NODE((x)) -> data
#define NODE_NEXT(x) NODE((x)) -> next

typedef struct _node_t
{
	void* data;
	struct _node_t* next;
} node_t;

typedef struct _queue_t
{
	node_t* head;
	node_t* tail;
	int size;
} queue_t;

/**
 * Node initialization
 * returns a pointer the new node with all fields nullified
 */
node_t* node_init()
{
	node_t* toReturn = (node_t*)malloc(sizeof(node_t));
	assert(toReturn != NULL);
	NODE_DATA(toReturn) = NULL;
	NODE_NEXT(toReturn) = NULL;

	return toReturn;
}

/**
 * Setting node's data. Use this method in order to set the node's
 * data. DON'T DO IT MANUALLY!
 * if node is null, nothing will happen and -1
 * 0 will be returned if everything is o.k
 */
int node_set_data(node_t* nd, void* data)
{
	int toReturn = -1;

	if (nd != NULL)
	{
		NODE_DATA(nd) = data;
		toReturn = 0;
	}

	return toReturn;
}

/**
 * Setting node's next. Use this method in order to set the node's
 * next. DON'T DO IT MANUALLY!
 * if node is null, nothing will happen and 1 will be returned
 * 0 will be returned if everything is o.k
 * -1 if insert fails
 */
int node_set_next(node_t* nd, node_t* node_next)
{
	int toReturn = -1;

	if (nd != NULL)
	{
		NODE_NEXT(nd) = node_next;
		toReturn = 0;
	}
	else
		toReturn = 1;

	return toReturn;
}

/**
 * will free all memory allocated to the node
 * where destfunc is the data's free function.
 * if destfunc is NULL, the data won't be deleted,
 * and it's up to the user to delete it and 1 will be returned.
 * in case of successful deletion 0 will be returned
 * PAY ATTENTION! it's recommended to delete the data
 * before using this function
 */
int node_destroy(node_t* nd, int(*destfunc)(void*))
{
	int toReturn = 0;

	if (destfunc != NULL)
		(*destfunc)(NODE_DATA(nd));
	else
		toReturn = 1;

	free(nd);

	return toReturn;
}

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
