#include "worker_thread.h"
#include "logger.h"

#define READ_START(_thread) rwlock_acquire_read(&(_thread)->lock)
#define READ_END(_thread) rwlock_release_read(&(_thread)->lock)
#define WRITE_START(_thread) rwlock_acquire_write(&(_thread)->lock)
#define WRITE_END(_thread) rwlock_release_write(&(_thread)->lock)


void* worker_thread_func(void* arg)
{
	worker_thread_t *thread = (worker_thread_t *)arg;
	BOOL stop = FALSE;

	WRITE_START(thread);
	thread->running = TRUE;
	WRITE_END(thread);

	while (!stop)
	{
		stop = thread->func(thread->arg);
		READ_START(thread);
		if (thread->stop)
			stop = TRUE;
		READ_END(thread);
	}

	WRITE_START(thread);
	thread->running = FALSE;
	WRITE_END(thread);

	DEBUG1("Thread with id %d will now exit\n", thread->tid);
	//pthread_exit(thread->arg);
	return thread->arg;
}

errcode_t worker_thread_create(worker_thread_t* thread, worker_func_t func)
{
	rwlock_init(&thread->lock);

	WRITE_START(thread);
	thread->running = FALSE;
	thread->stop = FALSE;
	thread->arg = NULL;
	thread->func = func;

	thread->file_started = NULL;
	thread->line_started = -1;
	WRITE_END(thread);

	return ecSuccess;
}

errcode_t worker_thread_start_impl(worker_thread_t* thread, void* arg, const char* file, int line)
{
	errcode_t errcode;
	WRITE_START(thread);
	thread->arg = arg;
	thread->file_started = file;
	thread->line_started = line;
	WRITE_END(thread);
	errcode = POSIX_ERRCODE(pthread_create(&thread->tid, NULL, worker_thread_func, thread));
	if (errcode != ecSuccess)
		return errcode;

	while(!worker_thread_is_running(thread))
		;//wait

	return errcode;
}

errcode_t worker_thread_stop(worker_thread_t* thread)
{
	WRITE_START(thread);
	thread->stop = TRUE;
	WRITE_END(thread);

	while (worker_thread_is_running(thread))
		;//wait

	return ecSuccess;
}

BOOL worker_thread_is_running(worker_thread_t* thread)
{
	BOOL running;
	READ_START(thread);
	running = thread->running;
	READ_END(thread);
	return running;
}

BOOL worker_thread_should_stop(worker_thread_t* thread)
{
	BOOL stop;
	READ_START(thread);
	stop = thread->stop;
	READ_END(thread);
	return stop;
}

void worker_thread_destroy(worker_thread_t* thread)
{
	rwlock_destroy(&thread->lock);
}

#include "tests/worker_thread_tests.c"

