#include "cunit/cunit.h"
#include <unistd.h> //for sleep()

static const int THREAD_COUNT=10;

BOOL worker_thread_counter_func(void* arg)
{
	++(* (int*) arg);
	return FALSE;
}

cunit_err_t test_worker_thread_start_stop()
{
	worker_thread_t thread;
	int runcount = 0;

	ASSERT_EQUALS(ecSuccess, worker_thread_create(&thread, worker_thread_counter_func));
	ASSERT_EQUALS(ecSuccess, worker_thread_start(&thread, &runcount));
	sleep(1);
	ASSERT_EQUALS(ecSuccess, worker_thread_stop(&thread));
	worker_thread_destroy(&thread);

	ASSERT_TRUE(runcount > 0);
	return ceSuccess;
}

cunit_err_t test_worker_thread_multiple()
{
	worker_thread_t threads[THREAD_COUNT];
	int runcount[THREAD_COUNT];
	int i;

	for (i=0; i<THREAD_COUNT; ++i)
	{
		runcount[i] = 0;
		ASSERT_EQUALS(ecSuccess, worker_thread_create(&threads[i], worker_thread_counter_func));
		ASSERT_EQUALS(ecSuccess, worker_thread_start(&threads[i], &runcount[i]));
	}
	sleep(2);

	for (i=0; i<THREAD_COUNT; ++i)
	{
		ASSERT_EQUALS(ecSuccess, worker_thread_stop(&threads[i]));
		worker_thread_destroy(&threads[i]);
		ASSERT_TRUE(runcount[i] > 0);
	}
	return ceSuccess;
}

void add_worker_thread_tests()
{
	ADD_TEST(test_worker_thread_start_stop);
	ADD_TEST(test_worker_thread_multiple);
}
