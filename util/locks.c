#include "locks.h"
#include <errno.h>
#include <assert.h>
#include "logger.h"

errcode_t rwlock_init(rwlock_t* rwlock)
{
//	DEBUG1()
	return POSIX_ERRCODE(pthread_rwlock_init(rwlock, NULL));
}

errcode_t rwlock_acquire_read(rwlock_t* rwlock)
{
	return POSIX_ERRCODE(pthread_rwlock_rdlock(rwlock));
}

errcode_t rwlock_acquire_write(rwlock_t* rwlock)
{
	return POSIX_ERRCODE(pthread_rwlock_wrlock(rwlock));
}

errcode_t rwlock_release_read(rwlock_t* rwlock)
{
	return POSIX_ERRCODE(pthread_rwlock_unlock(rwlock));
}

errcode_t rwlock_release_write(rwlock_t* rwlock)
{
	return POSIX_ERRCODE(pthread_rwlock_unlock(rwlock));
}

void rwlock_destroy(rwlock_t* rwlock)
{
	int retcode = pthread_rwlock_destroy(rwlock);

	assert(retcode != EBUSY); //rwlock is locked when trying to release it.
	assert(retcode == 0);
}

errcode_t mutex_lock(pthread_mutex_t* mutex)
{
	return POSIX_ERRCODE(pthread_mutex_lock(mutex));
}

errcode_t mutex_unlock(pthread_mutex_t* mutex)
{
	return POSIX_ERRCODE(pthread_mutex_unlock(mutex));
}

void mutex_destroy(pthread_mutex_t* mutex)
{
	int retcode = pthread_mutex_destroy(mutex);

	assert(retcode != EBUSY);
	assert(retcode == 0);
}

errcode_t cond_wait(pthread_cond_t* cond, pthread_mutex_t* mutex)
{
	return POSIX_ERRCODE(pthread_cond_wait(cond, mutex));
}

errcode_t cond_signal(pthread_cond_t* cond)
{
	return POSIX_ERRCODE(pthread_cond_signal(cond));
}

void cond_destroy(pthread_cond_t* cond)
{
	int retcode = pthread_cond_destroy(cond);

	assert(retcode != EBUSY);
	assert(retcode == 0);
}



//this is a very thin wrapper around pthread's rwlock, so no tests needed.
