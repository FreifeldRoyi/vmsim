/*
 * pcb_util.c
 *
 *  Created on: Jun 2, 2010
 *      Author: Freifeld Royi
 */

#include "pcb_util.h"
#include <malloc.h>
#include <assert.h>

errcode_t compose_mail(process_t* prc, post_t* post)
{
	assert(post != NULL);

	pthread_mutex_lock(&PROC_MAIL_LOCK(prc));
	if (prc == NULL)
	{
		pthread_mutex_unlock(&PROC_MAIL_LOCK(prc));
		return ecFail;
	}

	if (PROC_DEL(prc)) //process got exit flag
	{
		pthread_mutex_unlock(&PROC_MAIL_LOCK(prc));
		return ecNotFound;
	}

	queue_push(PROC_MAIL(prc), post);
	pthread_cond_signal(&PROC_COND(prc));
	pthread_mutex_unlock(&PROC_MAIL_LOCK(prc));
	return ecSuccess;
}

post_t* create_post(func_t func, void** args, int nargs)
{
	post_t* to_return = (post_t*)malloc(sizeof(post_t));

	to_return -> args = args;
	to_return -> func = func;
	to_return -> n_args = nargs;

	return to_return;
}

void func_arg_destroy(func_arg* arg)
{
	assert(arg != NULL);
	free(arg);
}

void post_destroy(post_t* post)
{
	int i;
	assert(post != NULL);

	if (post -> args != NULL)
	{
		for (i = 0; i < post -> n_args; ++i)
		{
			free((post -> args)[i]);
		}
	}

	free(post);
}

/****************************************process functions*******************************/

errcode_t process_dealloc(proc_cont_t* proc_cont, procid_t pid)
{
	mmu_t* mmu = PROC_CONT_MMU(proc_cont);
	disk_t* disk = mmu -> disk;
	process_t* this_proc = &PROC_CONT_SPEC_PROC(proc_cont, pid);

	assert(mmu != NULL);
	assert(disk != NULL);
	assert(PROC_JUNK(this_proc) == FALSE);
	assert(PROC_DEL(this_proc) == TRUE);

	pthread_mutex_lock(&PROC_MAIL_LOCK(this_proc));
	while (queue_size(PROC_MAIL(this_proc)) != 0)
	{
		queue_pop(PROC_MAIL(this_proc));
	}

	queue_destroy(PROC_MAIL(this_proc));
	PROC_MAIL(this_proc) = NULL;
	pthread_mutex_unlock(&PROC_MAIL_LOCK(this_proc));

	pthread_mutex_destroy(&PROC_MAIL_LOCK(this_proc));
	pthread_cond_destroy(&PROC_COND(this_proc));

	disk_free_process_block(disk, PROC_STRT(this_proc));
	//TODO delete from MM???

	worker_thread_destroy(PROC_THRD(this_proc));
	PROC_JUNK(this_proc) = TRUE;

	pthread_cond_signal(&PROC_CONT_DEL(proc_cont));

	return ecSuccess;
}

errcode_t sim_read(proc_cont_t* proc_cont, virt_addr_t* vAddr, int off,int amount, char* file_name)
{
	int page_size = PROC_CONT_MMU(proc_cont) -> mem -> page_size;
	FILE* f;
	errcode_t err = ecSuccess;

	BYTE* buf = (BYTE*)malloc((amount * off + 1) * sizeof(BYTE));
	int multiplier = 0;
	int i;

	assert(off > 0);
	assert(vAddr -> pid >= 0);
	assert(amount > 0);

	while (multiplier < amount - 1 && off * multiplier < page_size && err == ecSuccess)
	{
		err = mmu_read(PROC_CONT_MMU(proc_cont), *vAddr, off * multiplier, 1, &buf[multiplier]); //TODO check correctness
		++multiplier;
	}

	if (err == ecSuccess)
	{
		if (file_name == NULL)
			f = stdout;
		else
			f = fopen(file_name, "w+"); //TODO maybe a different mode

		for (i = 0; i < multiplier; ++i)
		{
			fprintf(f, "Char %d: %c\n", i, buf[i]);
		}

		if (file_name != NULL)
			fclose(f);
	}
	free(buf);

	return err;
}
