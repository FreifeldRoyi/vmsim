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
static virt_addr_t swap_target;

static void get_oldest_page(phys_addr_t mem_page, page_data_t* page)
{
	static int cur_idx;
	unsigned age = page->page_age;

	if (oldest_page_idx == -1)
	{
		cur_idx = 0;
	}

	///actually age works in reverse - the more leading '0's it has, the older it is.
	if (((age < oldest_page_age) || (oldest_page_idx == -1))&&
		(!VIRT_ADDR_PAGE_EQ(swap_target, page->addr)))
	{
		oldest_page_idx = cur_idx;
		oldest_page_age = age;
		oldest_page_vaddr = page->addr;
	}
}

static virt_addr_t get_page_to_swap(mmu_t* mmu, virt_addr_t page_to_load)
{
	oldest_page_idx = -1;
	swap_target = page_to_load;
	mmu_for_each_mem_page(mmu, get_oldest_page);
	assert (!VIRT_ADDR_PAGE_EQ(page_to_load, oldest_page_vaddr));\
	DEBUG1("age of oldest page is 0x%X\n", oldest_page_age);
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
	errcode_t errcode;
	errcode = mmu_sync_to_backing_page(mmu, page);
	assert( errcode == ecSuccess);
	errcode = mmu_unmap_page(mmu, page);
	assert( errcode == ecSuccess);
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

	mmu_acquire_write(mmu);
	DEBUG2("swap in (%d:%d)\n", VIRT_ADDR_PID(addr), VIRT_ADDR_PAGE(addr));

	errcode = mmu_map_page(mmu, addr); //the vaddr to swap in is already locked
	while (errcode != ecSuccess) //no free pages in memory, swap a page out.
	{
		phys_addr_t mem_page;
		DEBUG("Swapping in required.\n");
		vaddr_to_swap = get_page_to_swap(mmu, addr);
		DEBUG2("Chose (%d:%d) to swap out.\n",VIRT_ADDR_PID(vaddr_to_swap), VIRT_ADDR_PAGE(vaddr_to_swap));
		prm_swap_out(mmu, vaddr_to_swap);
		DEBUG("Swapped out.\n");
		errcode = mmu_map_page(mmu, addr);
		/*if after swapping out we have no free pages, maybe a concurrent alloc
		 * caught the page we freed. We have to try again.
		 */

		//FOR DEBUGGING
		ipt_translate(&mmu->mem_ipt, addr, &mem_page);
		DEBUG1("Mapped new page at %d\n", mem_page);
	}

	mmu_sync_from_backing_page(mmu, addr);
	DEBUG("Released MMU\n");

	mmu_release_write(mmu);

	pthread_mutex_lock(&prm_mutex);
	cmd->done = TRUE;
	pthread_cond_broadcast(&prm_condvar);
	pthread_mutex_unlock(&prm_mutex);

	return FALSE;
}

errcode_t prm_init(mmu_t* mmu)
{
	errcode_t errcode;
	DEBUG("PRM starting\n");
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
	DEBUG("PRM exiting\n");
	worker_thread_stop(&prm_thread);
	worker_thread_destroy(&prm_thread);
	assert(queue_size(prm_queue) == 0);
	pthread_mutex_destroy(&prm_mutex);
	pthread_mutex_destroy(&prm_queue_mutex);
	pthread_cond_destroy(&prm_queue_condvar);
	pthread_cond_destroy(&prm_condvar);
	queue_destroy(prm_queue);
	DEBUG("PRM exited\n");
}
