
#include "util/queue.h"
#include "cunit/cunit.h"
#include <stdio.h>


typedef struct{
	int value;
}test_data_t;

cunit_err_t test_creation()
{
	queue_t* t_qu = queue_init();

	ASSERT_EQUALS (QUEUE_HEAD(t_qu), NULL);
	ASSERT_EQUALS (QUEUE_TAIL(t_qu), NULL);
	ASSERT_EQUALS (QUEUE_SIZE(t_qu), 0);
	ASSERT_EQUALS (queue_destroy(t_qu), 0);

	return ceSuccess;
}

cunit_err_t test_queue_pop()
{
	queue_t* t_qu = queue_init();
	node_t* t_nd1 = node_init();
	node_t* t_nd2 = node_init();
	node_t* t_nd3 = node_init();

	//push testing1
	queue_push(t_qu,t_nd1);

	ASSERT_EQUALS (NODE_DATA(QUEUE_HEAD(t_qu)), t_nd1);
	ASSERT_EQUALS (QUEUE_SIZE(t_qu), 1);

	//push testing2
	queue_push(t_qu,t_nd2);

	ASSERT_EQUALS (NODE_DATA(QUEUE_HEAD(t_qu)), t_nd1);
	ASSERT_EQUALS (NODE_DATA(QUEUE_TAIL(t_qu)), t_nd2);
	ASSERT_EQUALS (QUEUE_SIZE(t_qu), 2);
	ASSERT_EQUALS (NODE_DATA(NODE_NEXT(QUEUE_HEAD(t_qu))), t_nd2);

	//push testing3
	queue_push(t_qu,t_nd3);

	ASSERT_EQUALS (NODE_DATA(QUEUE_HEAD(t_qu)), t_nd1);
	ASSERT_EQUALS (NODE_DATA(NODE_NEXT(QUEUE_HEAD(t_qu))), t_nd2);
	ASSERT_EQUALS (NODE_DATA(QUEUE_TAIL(t_qu)), t_nd3);
	ASSERT_EQUALS (QUEUE_SIZE(t_qu), 3);
	ASSERT_EQUALS (NODE_DATA(QUEUE_HEAD(t_qu)), t_nd1);
	ASSERT_EQUALS (NODE_DATA(NODE_NEXT(QUEUE_HEAD(t_qu))), t_nd2);
	ASSERT_EQUALS (NODE_DATA(NODE_NEXT(NODE_NEXT(QUEUE_HEAD(t_qu)))), t_nd3);

	//pop testing1
	queue_pop(t_qu);

	ASSERT_EQUALS (NODE_DATA(QUEUE_HEAD(t_qu)), t_nd2);
	ASSERT_EQUALS (NODE_DATA(QUEUE_TAIL(t_qu)), t_nd3);
	ASSERT_EQUALS (QUEUE_SIZE(t_qu), 2)

	//pop testing2
	queue_pop(t_qu);

	ASSERT_EQUALS (NODE_DATA(QUEUE_HEAD(t_qu)), t_nd3);
	ASSERT_EQUALS (NODE_DATA(QUEUE_TAIL(t_qu)), t_nd3);
	ASSERT_EQUALS (QUEUE_SIZE(t_qu), 1)

	//pop testing3
	queue_pop(t_qu);

	ASSERT_EQUALS (QUEUE_HEAD(t_qu), NULL);
	ASSERT_EQUALS (QUEUE_TAIL(t_qu), NULL);
	ASSERT_EQUALS (QUEUE_SIZE(t_qu), 0)

	ASSERT_EQUALS(0, queue_destroy(t_qu));
	ASSERT_EQUALS(1, node_destroy(t_nd1,NULL));
	ASSERT_EQUALS(1, node_destroy(t_nd2,NULL));
	ASSERT_EQUALS(1, node_destroy(t_nd3,NULL));

	return ceSuccess;
}


cunit_err_t test_node_creation()
{
	int toReturn = 0;
	node_t* t_nd = node_init();

	ASSERT_EQUALS (NODE_DATA(t_nd), NULL)
	ASSERT_EQUALS (NODE_NEXT(t_nd), NULL)

	ASSERT_EQUALS(1, node_destroy(t_nd,NULL));

	return toReturn;
}

cunit_err_t test_node_delete()
{
	queue_t* t_toDestroy = queue_init();
	node_t* t_toAdd = node_init();
	node_t* t_nd = node_init();

	node_set_data(t_nd,t_toDestroy);

	ASSERT_EQUALS(1, queue_push(t_toDestroy, t_toAdd));
	ASSERT_EQUALS(1, node_destroy(queue_pop(t_toDestroy), NULL));
	ASSERT_EQUALS(0, node_destroy(t_nd, &queue_destroy));

	return ceSuccess;
}

void add_queue_tests ()
{
	ADD_TEST(test_node_creation);
	ADD_TEST(test_node_delete);
	ADD_TEST(test_creation);
	ADD_TEST(test_queue_pop);
}
