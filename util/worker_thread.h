#ifndef WORKER_THREAD_H_
#define WORKER_THREAD_H_

#include "vmsim_types.h"
#include <pthread.h>

typedef void (*worker_func_t)(void* arg);

typedef struct {
	worker_func_t 	func;
	void* 			arg;
	BOOL			stop;
	BOOL			running;

	pthread_t		tid;
}worker_thread_t;

errcode_t worker_thread_create(worker_thread_t* thread, worker_func_t func);
errcode_t worker_thread_start(worker_thread_t* thread, void* arg);
errcode_t worker_thread_stop(worker_thread_t* thread);
void worker_thread_destroy(worker_thread_t* thread);

#endif /* WORKER_THREAD_H_ */
