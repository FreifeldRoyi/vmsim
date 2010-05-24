#include "prm.h"
#include "util/queue.h"
#include "util/worker_thread.h"

#include <pthread.h>

#include <stdlib.h>
#include <assert.h>

static pthread_mutex_t prm_mutex;
static pthread_mutex_t prm_queue_mutex;
static struct _queue_t* prm_queue;
static worker_thread_t prm_thread;

//we use a condvar per command, which is a bit heavyweight but saves us from
//dealing with multiple threads that sleep on the same var.
typedef struct
{
	virt_addr_t addr;
	pthread_cond_t cond_var;
	pthread_mutex_t mutex;
}prm_command_t;

static prm_command_t* new_prm_command(virt_addr_t addr)
{
	prm_command_t* cmd = malloc(sizeof(prm_command_t));
	cmd->addr = addr;
	pthread_cond_init(&cmd->cond_var, NULL);
	pthread_mutex_init(&cmd->mutex, NULL);

	pthread_mutex_lock(&cmd->mutex);

	return cmd;
}

static void wait_for_completion(prm_command_t* cmd)
{
	pthread_cond_wait(&cmd->cond_var, &cmd->mutex);
	pthread_mutex_unlock(&cmd->mutex);
}

static void delete_prm_command(prm_command_t* cmd)
{
	pthread_cond_destroy(&cmd->cond_var);
	pthread_mutex_destroy(&cmd->mutex);

	free(cmd);
}

static int oldest_page_idx = -1;
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
	}
}

static int get_page_to_swap(mmu_t* mmu)
{
	oldest_page_idx = -1;
	mmu_for_each_mem_page(mmu, get_oldest_page);
	return oldest_page_idx;
}

static void push_command(prm_command_t* cmd)
{
	pthread_mutex_lock(&prm_queue_mutex);
	queue_push(prm_queue, cmd);
	pthread_mutex_unlock(&prm_queue_mutex);
}

static void prm_swap_out(mmu_t* mmu, phys_addr_t page)
{
	virt_addr_t vaddr = mmu_phys_to_virt(mmu, page);

	mmu_sync_to_backing_page(mmu, vaddr);
	mmu_unmap_page_unlocked(mmu, vaddr);
}

static BOOL prm_thread_func(void* arg)
{
	prm_command_t* cmd;
	virt_addr_t addr;
	mmu_t* mmu = arg;
	unsigned mem_page_to_swap;
	errcode_t errcode;

	pthread_mutex_lock(&prm_queue_mutex);
	cmd = queue_pop(prm_queue);
	pthread_mutex_unlock(&prm_queue_mutex);
	if (cmd == NULL)
	{
		return FALSE;
	}
	addr = cmd->addr;

	mmu_acquire(mmu);
	errcode = mmu_map_page_unlocked(mmu, addr);
	if (errcode != ecSuccess) //no free pages in memory, swap a page out.
	{
		mem_page_to_swap = get_page_to_swap(mmu);
		prm_swap_out(mmu, mem_page_to_swap);
		errcode = mmu_map_page_unlocked(mmu, addr);
		assert(errcode == ecSuccess);//if after swapping out we have no free pages, something's wrong.
	}

	mmu_sync_from_backing_page(mmu, addr);
	mmu_release(mmu);


	pthread_cond_signal(&cmd->cond_var);

	return FALSE;
}

errcode_t prm_init(mmu_t* mmu)
{
	errcode_t errcode;
	pthread_mutex_init(&prm_mutex,NULL);
	pthread_mutex_init(&prm_queue_mutex,NULL);
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

	assert(worker_thread_is_running(&prm_thread));

	push_command(cmd);
	wait_for_completion(cmd); //blocks
	delete_prm_command(cmd);

	return ecSuccess;
}

void prm_destroy()
{
	worker_thread_stop(&prm_thread);
	worker_thread_destroy(&prm_thread);
	pthread_mutex_destroy(&prm_mutex);
	pthread_mutex_destroy(&prm_queue_mutex);
	queue_destroy(prm_queue);
}
