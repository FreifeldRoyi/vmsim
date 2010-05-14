#include "worker_thread.h"

void* worker_thread_func(void* arg)
{
	worker_thread_t *thread = (worker_thread_t *)arg;
	thread->running = TRUE;

	while (!thread->stop)
	{
		thread->func(thread->arg);
	}

	thread->running = FALSE;

	return thread->arg;
}

errcode_t worker_thread_create(worker_thread_t* thread, worker_func_t func)
{
	thread->running = FALSE;
	thread->stop = FALSE;
	thread->arg = NULL;
	thread->func = func;

	return ecSuccess;
}

errcode_t worker_thread_start(worker_thread_t* thread, void* arg)
{
	thread->arg = arg;
	if (pthread_create(&thread->tid, NULL, worker_thread_func, thread) != 0)
	{
		return ecFail;
	}
	return ecSuccess;
}

errcode_t worker_thread_stop(worker_thread_t* thread)
{
	thread->stop = TRUE;
	while (thread->running)
		;//wait

	return ecSuccess;
}

void worker_thread_destroy(worker_thread_t* thread)
{
///nothing to do
}

#include "tests/worker_thread_tests.c"

