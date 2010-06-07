/*
 * pcb_util.h
 *
 *  Created on: Jun 2, 2010
 *      Author: freifeldroyi
 */

#ifndef PCB_UTIL_H_
#define PCB_UTIL_H_

#include "util/worker_thread.h"
#include "util/vmsim_types.h"
#include "sim/mmu.h"
#include "util/queue.h"
/****************************************definitions*******************************/

#define READ_START(_lock) rwlock_acquire_read(_lock)
#define WRITE_START(_lock) rwlock_acquire_write(_lock)
#define READ_END(_lock) rwlock_release_read(_lock)
#define WRITE_END(_lock) rwlock_release_write(_lock)

typedef struct
{
	procid_t pid;

	int disk_block_start;
	int block_size; //can later be removed
	BOOL junk; //if TRUE can be overridden
	BOOL del; //if TRUE, process got a delete message

	worker_thread_t* proc_thrd;

	struct _queue_t* mail_box;
	pthread_mutex_t mail_mutex;
	pthread_cond_t no_mail;
} process_t;

#define PROCESS(x) ((process_t *) (x))
#define PROC_PID(x) PROCESS((x)) -> pid
#define PROC_STRT(x) PROCESS((x)) -> disk_block_start
#define PROC_JUNK(x) PROCESS((x)) -> junk
#define PROC_DEL(x) PROCESS((x)) -> del
#define PROC_THRD(x) PROCESS((x)) -> proc_thrd
#define PROC_MAIL(x) PROCESS((x)) -> mail_box
#define PROC_MAIL_LOCK(x) PROCESS((x)) -> mail_mutex
#define PROC_COND(x) PROCESS((x)) -> no_mail

typedef struct
{
	process_t* processes;

	mmu_t* mmu;

	unsigned max_num_of_proc;

	pthread_mutex_t mutex;
	pthread_cond_t delete;

} proc_cont_t; //process container

#define PROC_CONT(x) ((proc_cont_t *) (x))
#define PROC_CONT_PRC(x) PROC_CONT((x)) -> processes //returns processes
#define PROC_CONT_N_PROC(x) PROC_CONT((x)) -> max_num_of_proc
#define PROC_CONT_MMU(x) PROC_CONT((x)) -> mmu
#define PROC_CONT_SPEC_PROC(_cont, _pid) (PROC_CONT_PRC((_cont)))[_pid]
#define PROC_CONT_MTX(x) PROC_CONT((x)) -> mutex
#define PROC_CONT_DEL(x) PROC_CONT((x)) -> delete

typedef enum
{
	fcRead,
	fcLoopRead,
	fcReadToFile,
	fcLoopReadToFile,
	fcWrite,
	fcLoopWrite,
	fcDel
} func_t;

typedef struct
{
	func_t func;
	void** args;
	int n_args;
} post_t;

typedef struct
{
	proc_cont_t* cont;
	procid_t curr_pid;
} func_arg;

#define ARG(x) ((func_arg *) (x))
#define ARG_CONT(x) ARG((x)) -> cont
#define ARG_PID(x) ARG((x)) -> curr_pid
#define ARG_PROC(x) ARG_CONT((x))

/****************************************aid functions*******************************/

/**
 * creates and sends mail to the process
 */
errcode_t compose_mail(process_t* prc, post_t* post);

/**
 * creates post struct
 *
 */
post_t* create_post(func_t func, void** args, int nargs);

/**
 * destroys the struct func_arg
 * DOE'S NOT free memory of proc_cont_t
 */
void func_arg_destroy(func_arg* arg);

/**
 * deallocate all memory associated with post
 * including all post->args
 *
 * @param post post to destroy
 */
void post_destroy(post_t* post);

/****************************************process functions*******************************/

/**
 * deallocates process' queue
 * memory on MM
 * and memory on DISK
 * call this function only if process is active
 *
 * @param prc - process container
 * @param pid - process's id to delete
 */
errcode_t process_dealloc(proc_cont_t* prc, procid_t pid);


#endif /* PCB_UTIL_H_ */
