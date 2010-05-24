#include "worker_thread.h"

void* worker_thread_func(void* arg)
{
	worker_thread_t *thread = (worker_thread_t *)arg;
	BOOL stop = FALSE;
	thread->running = TRUE;

	while ((!thread->stop) && (!stop))
	{
		stop = thread->func(thread->arg);
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

	thread->file_started = NULL;
	thread->line_started = -1;

	return ecSuccess;
}

errcode_t worker_thread_start_impl(worker_thread_t* thread, void* arg, const char* file, int line)
{
	thread->arg = arg;
	thread->file_started = file;
	thread->line_started = line;
	return POSIX_ERRCODE(pthread_create(&thread->tid, NULL, worker_thread_func, thread));

}

errcode_t worker_thread_stop(worker_thread_t* thread)
{
	thread->stop = TRUE;
	while (thread->running)
		;//wait

	return ecSuccess;
}

BOOL worker_thread_is_running(worker_thread_t* thread)
{
	return thread->running;
}

void worker_thread_destroy(worker_thread_t* thread)
{
///nothing to do
}

#include "tests/worker_thread_tests.c"

