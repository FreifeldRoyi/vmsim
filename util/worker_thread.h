#ifndef WORKER_THREAD_H_
#define WORKER_THREAD_H_

#include "vmsim_types.h"
#include <pthread.h>

typedef BOOL (*worker_func_t)(void* arg);

typedef struct {
	worker_func_t 	func;
	void* 			arg;
	volatile BOOL	stop;
	volatile BOOL	running;

	pthread_t		tid;

	//used for debugging
	const char* file_started;
	int line_started;
}worker_thread_t;

errcode_t worker_thread_create(worker_thread_t* thread, worker_func_t func);

#define worker_thread_start(_t, _a)worker_thread_start_impl(_t, _a, __FILE__, __LINE__)
errcode_t worker_thread_start_impl(worker_thread_t* thread, void* arg, const char* file, int line);
errcode_t worker_thread_stop(worker_thread_t* thread);

BOOL worker_thread_is_running(worker_thread_t* thread);

void worker_thread_destroy(worker_thread_t* thread);

#endif /* WORKER_THREAD_H_ */
