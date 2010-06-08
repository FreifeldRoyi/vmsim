/*
 * pcb.c
 *
 *  Created on: May 31, 2010
 *      Author: Freifeld Royi
 */

#include "pcb.h"
#include <assert.h>
#include <malloc.h>
#include <strings.h>

proc_cont_t* init_proc_cont(int nprocs, mmu_t* mmu)
{
	proc_cont_t* to_return;

	to_return = (proc_cont_t *)malloc(sizeof(proc_cont_t));
	assert(to_return != NULL);

	pthread_mutex_init(&PROC_CONT_MTX(to_return), NULL);
	pthread_cond_init(&PROC_CONT_DEL(to_return), NULL);

	process_t* processes = (process_t*)calloc(nprocs, sizeof(process_t));
	assert(processes != NULL);

	PROC_CONT_N_PROC(to_return) = nprocs;
	PROC_CONT_PRC(to_return) = processes;
	PROC_CONT_MMU(to_return) = mmu;

	return to_return;
}

int init_process(proc_cont_t* proc_cont)
{
	int i = 0;
	process_t* prc;
	errcode_t errs;
	int* page = NULL;
	
	assert(proc_cont != NULL);

	pthread_mutex_lock(&PROC_CONT_MTX(proc_cont));

	while (i < PROC_CONT_N_PROC(proc_cont) && !PROC_CONT_SPEC_PROC(proc_cont,i).junk)
	{
		++i;
	}

	if (i >= PROC_CONT_N_PROC(proc_cont))
	{
		pthread_mutex_unlock(&PROC_CONT_MTX(proc_cont));
		return -1;
	}

	prc = &PROC_CONT_SPEC_PROC(proc_cont, i); //area is already allocated on disk. just needs to get the pointer
	bzero(prc, sizeof(process_t)); //make all fields zero

	//init lock
	pthread_mutex_init(&PROC_MAIL_LOCK(prc),NULL);

	//init disk info
	assert(PROC_CONT_MMU(proc_cont) -> disk != NULL);
	errs = disk_alloc_process_block(PROC_CONT_MMU(proc_cont) -> disk, page);

	if (errs == ecFail)
	{
		pthread_mutex_unlock(&PROC_CONT_MTX(proc_cont));
		free(prc);
		return -2;
	}

	PROC_STRT(prc) = *page;
	prc -> block_size = PROC_CONT_MMU(proc_cont) -> disk -> process_block_size;
	PROC_JUNK(prc) = FALSE;

	PROC_MAIL(prc) = queue_init();
	pthread_cond_init(&PROC_COND(prc), NULL);
	PROC_PID(prc) = i;

	//concurrent object init
	errs = worker_thread_create(PROC_THRD(prc),&process_func);

	if (errs == ecFail)
	{
		pthread_mutex_unlock(&PROC_CONT_MTX(proc_cont));
		free(prc);
		return -3;
	}

	pthread_mutex_unlock(&PROC_CONT_MTX(proc_cont));
	return i;
}

errcode_t process_destroy(proc_cont_t* proc_cont, procid_t id)
{
	errcode_t err;
	process_t* this_proc = NULL;
	assert(proc_cont != NULL);

	pthread_mutex_lock(&PROC_CONT_MTX(proc_cont));

	if (id >= PROC_CONT_N_PROC(proc_cont) || id < 0)
	{
		pthread_mutex_unlock(&PROC_CONT_MTX(proc_cont));
		return ecFail;
	}

	this_proc = &PROC_CONT_SPEC_PROC(proc_cont, id);

	if (PROC_JUNK(this_proc))
	{
		pthread_mutex_unlock(&PROC_CONT_MTX(proc_cont));
		return ecNotFound;
	}

	post_t* post = create_post(fcDel,NULL, 0);
	err = compose_mail(this_proc, post);

	if (err != ecSuccess)
	{
		free(post); //post->args is NULL
	}
	PROC_DEL(this_proc) = TRUE;

	pthread_cond_wait(&PROC_CONT_DEL(this_proc), &PROC_CONT_MTX(proc_cont));
	//TODO add here code for sleeping

	//READ_END(&PROC_CONT_LOCK(proc_cont));
	return ecSuccess;
}

errcode_t proc_cont_destroy(proc_cont_t* proc_cont)
{
	int i;

	if (proc_cont == NULL)
	{
		return ecNotFound;
	}

	for (i = 0; i < PROC_CONT_N_PROC(proc_cont); ++i)
	{
		//initialization will create a buffer of 0 => all processes are junk at start
		if (!(PROC_CONT_SPEC_PROC(proc_cont,i).junk))
		{
			return ecFail;
		}
	}

	//no need to use dealloc_process since process is
	//considered junk only if thread,mail_box,mail_mutex,no_mail was freed
	free(PROC_CONT_PRC(proc_cont));
	PROC_CONT_PRC(proc_cont) = NULL;

	//disk data will be destroyed with process deletion and disk deletion

	return ecSuccess;
}

/**
 * We send a struct containing
 * proc_cont_t* and the current thread id
 */
BOOL process_func(void* arg)
{
	BOOL to_return = FALSE;
	func_arg* arguments = ARG(arg);

	proc_cont_t* cont = ARG_PROC(arguments);
	process_t* this_proc = &PROC_CONT_SPEC_PROC(cont,ARG_PID(arguments));

	struct _queue_t* mail_box = PROC_MAIL(this_proc);
	post_t* post = NULL;

	pthread_mutex_lock(&PROC_MAIL_LOCK(this_proc));

	while (queue_size(mail_box) == 0) //use while though only one thread uses the mail box
	{
		pthread_cond_wait(&PROC_COND(this_proc),&PROC_MAIL_LOCK(this_proc));
		pthread_mutex_lock(&PROC_MAIL_LOCK(this_proc)); //after release, lock needs to be acquired again
	}

	post = queue_pop(mail_box);
	pthread_mutex_unlock(&PROC_MAIL_LOCK(this_proc));

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
			process_dealloc(cont, arguments->curr_pid);
			to_return = TRUE;
			break;

		default:
			break;
	}

	post_destroy(post);
	post = NULL;

	return to_return;
	//TODO implement
}
