/*
 * pcb_util.c
 *
 *  Created on: Jun 2, 2010
 *      Author: Freifeld Royi
 */

#include "pcb_util.h"
#include "app_util.h"
#include "logger.h"
#include <malloc.h>
#include <assert.h>
#include <stdio.h>

errcode_t compose_mail(proc_cont_t* proc_cont, procid_t pid, post_t* post)
{
	process_t* prc = &PROC_CONT_SPEC_PROC(proc_cont,pid);
	assert(post != NULL);

	if (pid > PROC_CONT_N_PROC(proc_cont))
	{
		return ecNotFound;
	}

	if (PROC_JUNK(prc))
	{
		return ecNotFound;
	}

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
		free(post->args);
	}

	free(post);
}

errcode_t read_decoder(proc_cont_t* proc_cont, post_t* post)
{
	virt_addr_t* vAddr;
	int amount;

	assert(post -> n_args == 2);
	assert(post -> func == fcRead);

	vAddr = (virt_addr_t*)post -> args[0];
	amount = *(int*)post ->args[1];

	return sim_read(proc_cont, vAddr, 1, amount, NULL);
}

errcode_t loop_read_decoder(proc_cont_t* proc_cont, post_t* post)
{
	virt_addr_t* vAddr;
	int offset;
	int amount;

	assert(post -> n_args == 3);
	assert(post -> func == fcLoopRead);

	vAddr = (virt_addr_t*)post -> args[0];
	offset = *(int*)post -> args[1];
	amount = *(int*)post -> args[2];

	return sim_read(proc_cont, vAddr, offset, amount, NULL);
}

errcode_t read_file_decoder(proc_cont_t* proc_cont, post_t* post)
{
	virt_addr_t* vAddr;
	int amount;
	char* file_name;

	assert(post -> n_args == 3);
	assert(post -> func == fcReadToFile);

	vAddr = (virt_addr_t*)post -> args[0];
	amount = *(int*)post -> args[1];
	file_name = (char*)post -> args[2];

	return sim_read(proc_cont, vAddr, 1, amount, file_name);
}

errcode_t loop_read_file_decoder(proc_cont_t* proc_cont, post_t* post)
{
	virt_addr_t* vAddr;
	int offset;
	int amount;
	char* file_name;

	assert(post -> n_args == 4);
	assert(post -> func == fcLoopReadToFile);

	vAddr = (virt_addr_t*)post -> args[0];
	offset = *(int*)post -> args[1];
	amount = *(int*)post -> args[2];
	file_name = (char*)post -> args[3];

	return sim_read(proc_cont, vAddr, offset, amount, file_name);
}

errcode_t write_decoder(proc_cont_t* proc_cont, post_t* post)
{
	virt_addr_t* vAddr;
	unsigned char* s;
	int amount;

	assert(post -> n_args == 3);
	assert(post -> func == fcWrite);

	vAddr = (virt_addr_t*)post -> args[0];
	s = (unsigned char*)post -> args[1];
	amount = *(int*)post -> args[2];

	return sim_write(proc_cont, vAddr, s, amount);
}

errcode_t loop_write_decoder(proc_cont_t* proc_cont, post_t* post)
{
	virt_addr_t* vAddr;
	unsigned char c;
	int off;
	int amount;

	assert(post -> n_args == 4);
	assert(post -> func == fcLoopWrite);

	vAddr = (virt_addr_t*)post -> args[0];
	c = *(unsigned char*)post -> args[1];
	off = *(int*)post -> args[2];
	amount = *(int*)post -> args[3];

	return sim_loop_write(proc_cont,vAddr, c, off, amount);
}

/****************************************process functions*******************************/

errcode_t process_dealloc(proc_cont_t* proc_cont, procid_t pid)
{
	pthread_mutex_lock(&PROC_CONT_MTX(proc_cont));
	mmu_t* mmu = PROC_CONT_MMU(proc_cont);
	disk_t* disk = mmu -> disk;
	process_t* this_proc = &PROC_CONT_SPEC_PROC(proc_cont, pid);
	virt_addr_t vAddr;
	vAddr.page = 0;
	vAddr.pid = pid;

	assert(mmu != NULL);
	assert(disk != NULL);
	assert(PROC_JUNK(this_proc) == FALSE);
	assert(PROC_DEL(this_proc) == TRUE);

	DEBUG1("Destroying Process %d\n", pid);
	pthread_mutex_lock(&PROC_MAIL_LOCK(this_proc));
	while (queue_size(PROC_MAIL(this_proc)) != 0)
	{
		DEBUG("Popping item\n");
		queue_pop(PROC_MAIL(this_proc));
	}

	DEBUG("Destroying queue\n");
	queue_destroy(PROC_MAIL(this_proc));
	PROC_MAIL(this_proc) = NULL;
	pthread_mutex_unlock(&PROC_MAIL_LOCK(this_proc));

	DEBUG("Destroying mail mutex\n");
	pthread_mutex_destroy(&PROC_MAIL_LOCK(this_proc));

	DEBUG("Destroying no_mail condition\n");
	pthread_cond_destroy(&PROC_COND(this_proc));

	DEBUG("Deallocate mmu memory\n");
	mmu_free_multiple(mmu, vAddr, PROC_CONT_PRC_BLK_SZE(proc_cont));

	DEBUG("Deallocate pcb on disk\n");
	disk_free_process_block(disk, PROC_STRT(this_proc));

	DEBUG("Deleting thread\n");
	worker_thread_destroy(&PROC_THRD(this_proc));

	DEBUG("Assigning junk = TRUE\n");
	PROC_JUNK(this_proc) = TRUE;

	DEBUG("Freeing function argument\n");
	func_arg_destroy(this_proc->proc_thrd_arg);

	DEBUG("Signaling process container over del wait\n");
	pthread_cond_signal(&PROC_CONT_DEL(proc_cont));

	DEBUG("Unlocking mutex\n");
	pthread_mutex_unlock(&PROC_CONT_MTX(proc_cont));

	return ecSuccess;
}

static void advance_virt_addr(virt_addr_t* addr, int offset, int page_size)
{
	addr->offset += offset;
	while (addr->offset >= (unsigned)page_size)
	{
		++addr->page;
		addr->offset -= page_size;
	}
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
	assert(amount > 0);

	while (multiplier < amount && err == ecSuccess)
	{
		INFO3("reading from (%d:%d:%d)\n", VIRT_ADDR_PID(*vAddr),VIRT_ADDR_PAGE(*vAddr), VIRT_ADDR_OFFSET(*vAddr));
		err = mmu_read(PROC_CONT_MMU(proc_cont), *vAddr, 1, &buf[multiplier]);
		++multiplier;
		advance_virt_addr(vAddr, off,page_size);
	}

	if (err == ecSuccess)
	{
		if (file_name == NULL)
		{
			f = stdout;
		}
		else
		{
			INFO1("Output to %s\n", file_name);
			f = fopen(file_name, "w+");
		}

		for (i = 0; i < multiplier; ++i)
		{
			fprintf(f, "%c\n", buf[i]);
		}

		if (file_name != NULL)
			fclose(f);
	}
	free(buf);

	signal_job_done();

	return err;
}

errcode_t sim_write(proc_cont_t* proc_cont, virt_addr_t* vAddr, unsigned char* s, int amount)
{
	int multiplier = 0;
	int page_size = PROC_CONT_MMU(proc_cont) -> mem -> page_size;

	errcode_t err = ecSuccess;

	assert(amount > 0);

	while ( (multiplier < amount) && (multiplier < page_size) && (err == ecSuccess))
	{
		INFO3("writing to (%d:%d:%d)\n", VIRT_ADDR_PID(*vAddr),VIRT_ADDR_PAGE(*vAddr), VIRT_ADDR_OFFSET(*vAddr));
		err = mmu_write(PROC_CONT_MMU(proc_cont), *vAddr, 1, &s[multiplier]);
		++multiplier;
		advance_virt_addr(vAddr, 1,page_size);
	}

	signal_job_done();

	return err;
}

errcode_t sim_loop_write(proc_cont_t* proc_cont, virt_addr_t* vAddr, unsigned char c, int offset,int amount)
{
	int multiplier = 0;
	int page_size = PROC_CONT_MMU(proc_cont) -> mem -> page_size;

	errcode_t err = ecSuccess;

	assert(offset >= 0);
	assert(amount > 0);

	while ((multiplier < amount) && (err == ecSuccess))
	{
		INFO3("writing to (%d:%d:%d)\n", VIRT_ADDR_PID(*vAddr),VIRT_ADDR_PAGE(*vAddr), VIRT_ADDR_OFFSET(*vAddr));
		err = mmu_write(PROC_CONT_MMU(proc_cont), *vAddr, 1, &c);
		++multiplier;
		advance_virt_addr(vAddr, offset,page_size);
	}

	signal_job_done();

	return err;
}
