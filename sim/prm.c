#include "prm.h"
#include "util/queue.h"
#include "util/worker_thread.h"
#include "util/logger.h"

#include <pthread.h>
#include <time.h>
#include <sys/time.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

static pthread_mutex_t prm_mutex;
static pthread_cond_t prm_condvar;
static pthread_mutex_t prm_queue_mutex;
static pthread_cond_t prm_queue_condvar;
static struct _queue_t* prm_queue;
static worker_thread_t prm_thread;

typedef struct
{
	virt_addr_t addr;
	BOOL done;
}prm_command_t;

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

static prm_command_t* new_prm_command(virt_addr_t addr)
{
	prm_command_t* cmd = malloc(sizeof(prm_command_t));
	cmd->addr = addr;
	cmd->done = FALSE;
	return cmd;
}

static void wait_for_completion(prm_command_t* cmd)
{
	struct timespec ts;
	pthread_mutex_lock(&prm_mutex);
	while (!cmd->done)
	{
		ts = abs_time_from_delta(1);
		pthread_cond_timedwait(&prm_condvar, &prm_mutex, &ts);
	}
	pthread_mutex_unlock(&prm_mutex);

}

static void delete_prm_command(prm_command_t* cmd)
{
	free(cmd);
}

static int oldest_page_idx = -1;
static virt_addr_t oldest_page_vaddr;
static unsigned oldest_page_age = 0;

static void get_oldest_page(phys_addr_t mem_page, page_data_t* page)
{
	static int cur_idx;
	unsigned age = page->extra_data;

	if (oldest_page_idx == -1)
	{
		cur_idx = 0;
	}

	if ((age > oldest_page_idx) || (oldest_page_idx == -1))
	{
		oldest_page_idx = cur_idx;
		oldest_page_age = age;
		oldest_page_vaddr = page->addr;
	}
}

static virt_addr_t get_page_to_swap(mmu_t* mmu)
{
	oldest_page_idx = -1;
	mmu_for_each_mem_page(mmu, get_oldest_page);
	return oldest_page_vaddr;
}

static void push_command(prm_command_t* cmd)
{
	pthread_mutex_lock(&prm_queue_mutex);
	DEBUG1("Pushing %p\n", cmd);
	queue_push(prm_queue, cmd);
	if (queue_size(prm_queue) == 1)
	{
		DEBUG("Signaling\n");
		pthread_cond_broadcast(&prm_queue_condvar);
	}
	pthread_mutex_unlock(&prm_queue_mutex);
}

static void prm_swap_out(mmu_t* mmu, virt_addr_t page)
{
	phys_addr_t mem_page;

	mmu_pin_page(mmu, page, &mem_page);

	mmu_sync_to_backing_page_unlocked(mmu, page);
	mmu_unmap_page_unlocked(mmu, page);

//	mmu_unpin_page(mmu, vaddr); no need. unmapping the page also unpins it.
}

static BOOL prm_thread_func(void* arg)
{
	prm_command_t* cmd = NULL;
	virt_addr_t addr;
	mmu_t* mmu = arg;
	virt_addr_t vaddr_to_swap;
	errcode_t errcode;
	struct timespec wait_time;

	pthread_mutex_lock(&prm_queue_mutex);
	cmd = queue_pop(prm_queue);
	while (cmd == NULL){
		if (worker_thread_should_stop(&prm_thread))
		{
			pthread_mutex_unlock(&prm_queue_mutex);
			return FALSE;
		}
		wait_time = abs_time_from_delta(1);
		pthread_cond_timedwait(&prm_queue_condvar, &prm_queue_mutex, &wait_time);
		cmd = queue_pop(prm_queue);
	}
	pthread_mutex_unlock(&prm_queue_mutex);

	DEBUG1("Popped %p\n", cmd);

	addr = cmd->addr;

//	DEBUG("Locking MMU\n");
//	mmu_block_alloc_free(mmu); pagefaults happen from read/write context where allocation is blocked anyway
	errcode = mmu_map_page(mmu, addr);
	if (errcode != ecSuccess) //no free pages in memory, swap a page out.
	{
		vaddr_to_swap = get_page_to_swap(mmu);
		prm_swap_out(mmu, vaddr_to_swap);
		errcode = mmu_map_page(mmu, addr);
		assert(errcode == ecSuccess);//if after swapping out we have no free pages, something's wrong.
	}

	mmu_sync_from_backing_page_unlocked(mmu, addr);
//	mmu_release_alloc_free(mmu);
	DEBUG("Released MMU\n");

	pthread_mutex_lock(&prm_mutex);
	cmd->done = TRUE;
	pthread_cond_broadcast(&prm_condvar);
	pthread_mutex_unlock(&prm_mutex);

	return FALSE;
}

errcode_t prm_init(mmu_t* mmu)
{
	errcode_t errcode;
	INFO("PRM starting\n");
	pthread_mutex_init(&prm_mutex,NULL);
	pthread_mutex_init(&prm_queue_mutex,NULL);
	pthread_cond_init(&prm_queue_condvar, NULL);

	pthread_cond_init(&prm_condvar, NULL);
	prm_queue = queue_init();
	if (prm_queue == NULL)
		return ecFail;

	errcode = worker_thread_create(&prm_thread, prm_thread_func);
	if (errcode != ecSuccess)
	{
		return errcode;
	}

	errcode = worker_thread_start(&prm_thread, mmu);
	if (errcode != ecSuccess)
	{
		return errcode;
	}

	return ecSuccess;
}

errcode_t prm_pagefault(virt_addr_t addr)
{
	prm_command_t* cmd = new_prm_command(addr);

	DEBUG2("(%d:%d)\n", VIRT_ADDR_PID(addr), VIRT_ADDR_PAGE(addr));

	assert(worker_thread_is_running(&prm_thread));

	push_command(cmd);
	wait_for_completion(cmd); //blocks
	delete_prm_command(cmd);

	return ecSuccess;
}

void prm_destroy()
{
	INFO("PRM exiting\n");
	worker_thread_stop(&prm_thread);
	worker_thread_destroy(&prm_thread);
	assert(queue_size(prm_queue) == 0);
	pthread_mutex_destroy(&prm_mutex);
	pthread_mutex_destroy(&prm_queue_mutex);
	pthread_cond_destroy(&prm_queue_condvar);
	pthread_cond_destroy(&prm_condvar);
	queue_destroy(prm_queue);
	INFO("PRM exited\n");
}