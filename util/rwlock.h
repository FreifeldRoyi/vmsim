#ifndef RWLOCK_H_
#define RWLOCK_H_

#include "vmsim_types.h"
#include <pthread.h>

typedef pthread_rwlock_t rwlock_t;

errcode_t rwlock_init(rwlock_t* rwlock);

errcode_t rwlock_acquire_read(rwlock_t* rwlock);
errcode_t rwlock_acquire_write(rwlock_t* rwlock);
errcode_t rwlock_release_read(rwlock_t* rwlock);
errcode_t rwlock_release_write(rwlock_t* rwlock);

void rwlock_destroy(rwlock_t* rwlock);


#endif /* RWLOCK_H_ */
