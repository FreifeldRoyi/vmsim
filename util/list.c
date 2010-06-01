#include "list.h"

#include <stdlib.h>
#include <assert.h>

/* node_t macros*/
#define NODE(x) ((node_t *) (x))
#define NODE_DATA(x) NODE((x)) -> data
#define NODE_NEXT(x) NODE((x)) -> next

node_t* node_init()
{
	node_t* toReturn = (node_t*)malloc(sizeof(node_t));
	assert(toReturn != NULL);
	NODE_DATA(toReturn) = NULL;
	NODE_NEXT(toReturn) = NULL;

	return toReturn;
}

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

void list_for_each(list_t* list, element_func_t func)
{
	node_t *cur_node = LIST_HEAD(list);

	while (cur_node != NULL)
	{
		func(NODE_DATA(cur_node));
		cur_node = NODE_NEXT(cur_node);
	}
}
