#ifndef WORKER_THREAD_H_
#define WORKER_THREAD_H_

/**Worker thread implementation. A worker thread is a thread that repeatedly runs the
 * same function until it's stopped.*/

#include "vmsim_types.h"
#include "locks.h"
#include <pthread.h>

/*a worker-thread function.*/
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

	rwlock_t lock;
}worker_thread_t;

/**Initialize a new worker thread, but do not run it.
 * @param thread the thread to initialize
 * @param func the thread's function. When the thread runs this function will be called
 * 				repeatedly with the same argument until it returns TRUE or the thread is
 * 				stopped.
 * @return ecSuccess on success, some other value on failure.
 * */
errcode_t worker_thread_create(worker_thread_t* thread, worker_func_t func);

/**Run a worker thread.
 * @param _t the thread to run
 * @param _a the argument to pass to the thread's function on every call.
 * @return ecSuccess on success, some other value on failure.
 * */
#define worker_thread_start(_t, _a)worker_thread_start_impl(_t, _a, __FILE__, __LINE__)
errcode_t worker_thread_start_impl(worker_thread_t* thread, void* arg, const char* file, int line);

/**Stop a running worker thread.
 * This function does not return until the thread is stopped.
 * @param thread the thread to use
 * @return ecSuccess on success, some other value on failure.
 * */
errcode_t worker_thread_stop(worker_thread_t* thread);

/**Check if a worker thread is currently running
 * @param thread the thread to use
 * @return TRUE if the thread is running, FALSE otherwise.
 * */
BOOL worker_thread_is_running(worker_thread_t* thread);

/**Check if a stop request was sent to a thread.
 * @param thread the thread to use
 * @return TRUE if the thread should stop, FALSE otherwise.
 * */
BOOL worker_thread_should_stop(worker_thread_t* thread);


/**Finalize a worker thread.
 * This function must not be called on a running thread.
 * @param thread the thread to use
 * */
void worker_thread_destroy(worker_thread_t* thread);

#endif /* WORKER_THREAD_H_ */
