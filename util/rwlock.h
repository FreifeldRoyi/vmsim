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


#endif /* RWLOCK_H_ */
