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
	BOOL junk; //if TRUE can be overridden
	BOOL del; //if TRUE, process got a delete message

	worker_thread_t* proc_thrd;

	struct _queue_t* mail_box;
	pthread_mutex_t mail_mutex;
	pthread_cond_t no_mail;
} process_t;

#define PROCESS(x) ((process_t *) (x))
#define PROC_PID(x) (x) -> pid
#define PROC_STRT(x) (x) -> disk_block_start
#define PROC_JUNK(x) (x) -> junk
#define PROC_DEL(x) (x) -> del
#define PROC_THRD(x) (x) -> proc_thrd
#define PROC_MAIL(x) (x) -> mail_box
#define PROC_MAIL_LOCK(x) (x) -> mail_mutex
#define PROC_COND(x) (x) -> no_mail

typedef struct
{
	process_t* processes;

	mmu_t* mmu;

	unsigned max_num_of_proc;
	unsigned prc_blk_size;

	pthread_mutex_t mutex;
	pthread_cond_t delete;

} proc_cont_t; //process container

#define PROC_CONT(x) ((proc_cont_t *) (x))
#define PROC_CONT_PRC(x) (x) -> processes //returns processes
#define PROC_CONT_N_PROC(x) (x) -> max_num_of_proc
#define PROC_CONT_PRC_BLK_SZE(x) (x) -> prc_blk_size
#define PROC_CONT_MMU(x) (x) -> mmu
#define PROC_CONT_SPEC_PROC(_cont, _pid) (PROC_CONT_PRC((_cont)))[_pid]
#define PROC_CONT_MTX(x) (x) -> mutex
#define PROC_CONT_DEL(x) (x) -> delete

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
#define ARG_CONT(x) (x) -> cont
#define ARG_PID(x) (x) -> curr_pid

/****************************************aid functions*******************************/

/**
 * creates and sends mail to the process
 */
errcode_t compose_mail(proc_cont_t* proc_cont, procid_t pid, post_t* post);

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

/*
 * args decoder for read post
 *
 * @param proc_cont - the process container
 * @param post - the post
 *
 * @return ecSuccess or some other code on failure
 */
errcode_t read_decoder(proc_cont_t* proc_cont, post_t* post);
errcode_t loop_read_decoder(proc_cont_t* proc_cont, post_t* post);
errcode_t read_file_decoder(proc_cont_t* proc_cont, post_t* post);
errcode_t loop_read_file_decoder(proc_cont_t* proc_cont, post_t* post);
errcode_t write_decoder(proc_cont_t* proc_cont, post_t* post);
errcode_t loop_write_decoder(proc_cont_t* proc_cont, post_t* post);

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

/**
 * TODO documentation
 */
errcode_t sim_read(proc_cont_t* proc_cont, virt_addr_t* vAddr, int off,int amount, char* file_name);

errcode_t sim_write(proc_cont_t* proc_cont, virt_addr_t* vAddr, unsigned char* s, int amount);

errcode_t sim_loop_write(proc_cont_t* proc_cont, virt_addr_t* vAddr, unsigned char c, int offset,int amount);


#endif /* PCB_UTIL_H_ */
