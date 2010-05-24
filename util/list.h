/*
 * list.h
 *
 *  Created on: 12/05/2010
 *      Author: tom
 */

#ifndef LIST_H_
#define LIST_H_


/* node_t macros*/
#define NODE(x) ((node_t *) (x))
#define NODE_DATA(x) NODE((x)) -> data
#define NODE_NEXT(x) NODE((x)) -> next

#define LIST(x) ((list_t *) (x))
#define LIST_HEAD(x) LIST((x)) -> head
#define LIST_SIZE(x) LIST((x)) -> size

typedef struct _node_t
{
	void* data;
	struct _node_t* next;
} node_t;

typedef struct _list_t
{
	node_t* head;
	int size;
} list_t;

/**
 * Node initialization
 * returns a pointer the new node with all fields nullified
 */
node_t* node_init();

/**
 * Setting node's data. Use this method in order to set the node's
 * data. DON'T DO IT MANUALLY!
 * if node is null, nothing will happen and -1
 * 0 will be returned if everything is o.k
 */
int node_set_data(node_t* nd, void* data);

/**
 * Setting node's next. Use this method in order to set the node's
 * next. DON'T DO IT MANUALLY!
 * if node is null, nothing will happen and 1 will be returned
 * 0 will be returned if everything is o.k
 * -1 if insert fails
 */
int node_set_next(node_t* nd, node_t* node_next);

/**
 * will free all memory allocated to the node
 * where destfunc is the data's free function.
 * if destfunc is NULL, the data won't be deleted,
 * and it's up to the user to delete it and 1 will be returned.
 * in case of successful deletion 0 will be returned
 * PAY ATTENTION! it's recommended to delete the data
 * before using this function
 */
int node_destroy(node_t* nd, int(*destfunc)(void*));

typedef void (*element_func_t) (void*);
void list_for_each(list_t* list, element_func_t func);


#endif /* LIST_H_ */
