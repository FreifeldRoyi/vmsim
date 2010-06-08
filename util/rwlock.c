#include "rwlock.h"
#include <errno.h>
#include <assert.h>

errcode_t rwlock_init(rwlock_t* rwlock)
{
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


//this is a very thin wrapper around pthread's rwlock, so no tests needed.
