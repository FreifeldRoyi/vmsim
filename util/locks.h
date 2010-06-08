#ifndef RWLOCK_H_
#define RWLOCK_H_

/**A readers-writer lock.*/

#include "vmsim_types.h"
#include <pthread.h>

//currently we're only implementing a thin wrapper over posix RW-lock.
typedef pthread_rwlock_t rwlock_t;

/**Initialize an RW-lock.
 *
 * @param rwlock the lock to initialize.
 *
 * @return ecSuccess on success, some other value on failure.
 * */
errcode_t rwlock_init(rwlock_t* rwlock);

/**Acquire the lock for read.
 * @param rwlock the lock
 *
 * @return ecSuccess on success, some other value on failure.
 * */
errcode_t rwlock_acquire_read(rwlock_t* rwlock);

/**Acquire the lock for write.
 * @param rwlock the lock
 *
 * @return ecSuccess on success, some other value on failure.
 * */
errcode_t rwlock_acquire_write(rwlock_t* rwlock);

/**Release the lock after read.
 * @param rwlock the lock
 *
 * @return ecSuccess on success, some other value on failure.
 * */
errcode_t rwlock_release_read(rwlock_t* rwlock);

/**Release the lock after write.
 * @param rwlock the lock
 *
 * @return ecSuccess on success, some other value on failure.
 * */
errcode_t rwlock_release_write(rwlock_t* rwlock);

void rwlock_destroy(rwlock_t* rwlock);

/**
 * locks the given mutex
 * @param mutex - the mutex to lock
 *
 * @return ecSuccess or some other code on failure
 */
errcode_t mutex_lock(pthread_mutex_t* mutex);

/**
 * unlocks the given mutex
 * @param mutex - the mutex to unlock
 *
 * @return ecSuccess or some other code on failure
 */
errcode_t mutex_unlock(pthread_mutex_t* mutex);

/**
 * destroys the given mutex
 *
 * @param mutex - the mutex to destroy
 */
void mutex_destroy(pthread_mutex_t* mutex);

/**
 * monitor wait with the given condition variable
 * mutex given will be unlocked when thread is asleep
 * but thread will gain lock over the mutex once it was signaled
 *
 * @param cond - the condition variable
 * @param mutex - the mutex
 *
 * @return ecSuccess or some other code on failure
 */
errcode_t cond_wait(pthread_cond_t* cond, pthread_mutex_t* mutex);

/**
 * monitor signal with the given condition variable
 *
 * @return ecSuccess or some other code on failure
 */
errcode_t cond_signal(pthread_cond_t* cond);

/**
 * condition variable destroy
 * @param cond - the condition variable
 */
void cond_destroy(pthread_cond_t* cond);


#endif /* RWLOCK_H_ */
