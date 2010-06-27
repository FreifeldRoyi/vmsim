#include "aging_daemon.h"

#include "util/worker_thread.h"

#include "util/logger.h"

#include <assert.h>
#include <sys/time.h>
#include <time.h>
#include <errno.h>

static worker_thread_t daemon_thread;
static pthread_cond_t should_update_condvar;
static pthread_cond_t done_updating_condvar;
static volatile BOOL should_update;
static volatile BOOL done_updating;
static pthread_mutex_t daemon_mutex;

static struct timespec
abs_time_from_delta(int delta)
{
	struct timeval tp;
	struct timespec ts;

	gettimeofday(&tp, NULL);

	/* Convert from timeval to timespec */
	ts.tv_sec  = tp.tv_sec;
	ts.tv_nsec = tp.tv_usec * 1000;
	ts.tv_sec += delta;
	return ts;
}

void age_page(phys_addr_t mem_page, page_data_t* page)
{
	DEBUG2("old page age: 0x%x, referenced: %d\n", page->page_age, page->referenced);
	page->page_age >>= 1;
	SET_MSB(page->page_age, (page->referenced)?1:0);
	page->referenced = FALSE;
	DEBUG1("new page age: 0x%x\n", page->page_age);
}

BOOL daemon_func(void* arg)
{
	struct timespec waittime = abs_time_from_delta(1);
	mmu_t* mmu = arg;

	pthread_mutex_lock(&daemon_mutex);
	pthread_cond_timedwait(&should_update_condvar, &daemon_mutex, &waittime);
	if (!should_update)
	{
		pthread_mutex_unlock(&daemon_mutex);
		return FALSE;
	}
	mmu_acquire_read(mmu);

	mmu_for_each_mem_page(mmu, age_page);

	mmu_release_read(mmu);

	should_update = FALSE;
	done_updating = TRUE;
	pthread_cond_broadcast(&done_updating_condvar);

	pthread_mutex_unlock(&daemon_mutex);

	return FALSE;
}


errcode_t aging_daemon_update_pages()
{
	assert(worker_thread_is_running(&daemon_thread));
	pthread_mutex_lock(&daemon_mutex);
	done_updating = FALSE;
	should_update = TRUE;
	pthread_cond_broadcast(&should_update_condvar);

	while (!done_updating){
		struct timespec waittime = abs_time_from_delta(1);
		pthread_cond_timedwait(&done_updating_condvar, &daemon_mutex, &waittime);
	}
	pthread_mutex_unlock(&daemon_mutex);
	return ecSuccess;
}

errcode_t aging_daemon_start(mmu_t* mmu)
{
	errcode_t errcode;

	errcode = worker_thread_create(&daemon_thread, &daemon_func);
	if (errcode != ecSuccess)
	{
		return errcode;
	}

	pthread_mutex_init(&daemon_mutex, NULL);
	pthread_cond_init(&should_update_condvar, NULL);
	pthread_cond_init(&done_updating_condvar, NULL);

	should_update = FALSE;

	errcode = worker_thread_start(&daemon_thread, mmu);
	if (errcode != ecSuccess)
	{
		return errcode;
	}

	return ecSuccess;
}

void aging_daemon_stop()
{
	worker_thread_stop(&daemon_thread);
	pthread_cond_destroy(&should_update_condvar);
	pthread_cond_destroy(&done_updating_condvar);
	pthread_mutex_destroy(&daemon_mutex);
}
