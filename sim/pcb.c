/*
 * pcb.c
 *
 *  Created on: May 31, 2010
 *      Author: Freifeld Royi
 */

#include "pcb.h"
#include <assert.h>
#include <malloc.h>

proc_cont_t init_proc_cont(int nprocs, mmu_t* mmu)
{
	proc_cont_t to_return;

	process_t* processes = (process_t*)calloc(0, nprocs * sizeof(process_t));
	assert(processes != NULL);

	PROC_CONT_N_PROC(&to_return) = nprocs;
	PROC_CONT_PRC(&to_return) = processes;
	PROC_CONT_MMU(&to_return) = mmu;
	rwlock_init(&(to_return.lock));

	return to_return;
}

int init_process(proc_cont_t* proc_cont)
{
	int i = 0;
	process_t* prc;
	errcode_t errs;
	int* page = NULL;
	pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
	pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

	assert(proc_cont != NULL);

	//WRITE_START(&PROC_CONT_LOCK(proc_cont));

	while (i < PROC_CONT_N_PROC(proc_cont) && !PROC_CONT_PRC(proc_cont)[i].junk)
	{
		++i;
	}

	if (i >= PROC_CONT_N_PROC(proc_cont))
	{
	//	WRITE_END(&PROC_CONT_LOCK(proc_cont));
		return -1;
	}

	prc = (process_t *)calloc(0,sizeof(process_t));

	//init lock
	PROC_LOCK(prc) = mutex;

	//init disk info
	assert(PROC_CONT_MMU(proc_cont) -> disk != NULL);

	errs = disk_alloc_process_block(PROC_CONT_MMU(proc_cont) -> disk, page);

	if (errs == ecFail)
	{
	//	WRITE_END(&PROC_CONT_LOCK(proc_cont));
		free(prc);
		return -1;
	}

	PROC_STRT(prc) = *page;
	prc -> block_size = PROC_CONT_MMU(proc_cont) -> disk -> process_block_size;
	PROC_JUNK(prc) = FALSE;

	PROC_MAIL(prc) = queue_init();
	PROC_COND(prc) = cond;
	PROC_PID(prc) = i;

	//concurrent object init
	errs = worker_thread_create(PROC_THRD(prc),&process_func);

	if (errs == ecFail)
	{
	//	WRITE_END(&PROC_CONT_LOCK(proc_cont));
		free(prc);
		return -1;
	}

//	WRITE_END(&PROC_CONT_LOCK(proc_cont));
	return i;
}

errcode_t process_destroy(proc_cont_t* proc_cont, int id)
{
	post_err_t err;
	assert(proc_cont != NULL);

	READ_START(&PROC_CONT_LOCK(proc_cont));

	if (id >= PROC_CONT_N_PROC(proc_cont))
	{
		READ_END(&PROC_CONT_LOCK(proc_cont));
		return ecFail;
	}
	else if (PROC_CONT_PRC(proc_cont)[id].junk)
	{
		READ_END(&PROC_CONT_LOCK(proc_cont));
		return ecNotFound;
	}

	post_t* post = create_post(fcDel,NULL, 0);
	err = compose_mail(&PROC_CONT_PRC(proc_cont)[id], post);

	if (err != peSuccess)
	{
		free(post); //post->args is NULL
	}
	PROC_CONT_PRC(proc_cont)[id].del = TRUE;

	READ_END(&PROC_CONT_LOCK(proc_cont));
	return ecSuccess;
}

errcode_t proc_cont_destroy(proc_cont_t* proc_cont)
{
	//TODO implement
	// man don't forget to delete pages from MM and DISK
}

/**
 * We send a struct containing
 * proc_cont_t* and the current thread id
 */
BOOL process_func(void* arg)
{
	func_arg* arguments = ARG(arg);

	proc_cont_t* cont = ARG_PROC(arguments);
	process_t* this_proc = &PROC_CONT_SPEC_PROC(cont,ARG_PID(arguments));


	BOOL exit = PROC_DEL(this_proc);

	struct _queue_t* mail_box = PROC_MAIL(this_proc);
	post_t* post = NULL;

	while (!exit)
	{
		pthread_mutex_lock(&PROC_LOCK(this_proc));
		if (queue_size(mail_box) == 0)
			pthread_cond_wait(&PROC_COND(this_proc),&PROC_LOCK(this_proc));
		else
		{
			post = queue_pop(mail_box);
			pthread_mutex_unlock(&PROC_LOCK(this_proc));
		}

		switch (post->func)
		{
			case fcRead:
				//TODO smth
				break;

			case fcLoopRead:
				//TODO smth
				break;

			case fcReadToFile:
				//TODO smth
				break;

			case fcLoopReadToFile:
				//TODO smth
				break;

			case fcWrite:
				//TODO smth
				break;

			case fcLoopWrite:
				//TODO smth
				break;

			case fcDel:
				exit = TRUE;
				break;

			default:
				break;
		}

		post_destroy(post);
		post = NULL;
	}
	//TODO implement
}
