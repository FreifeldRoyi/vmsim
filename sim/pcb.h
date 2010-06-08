/*
 * pcb.h
 *
 *  Created on: May 30, 2010
 *      Author: Freifeld Royi
 */

#ifndef PCB_H_
#define PCB_H_

#include "util/pcb_util.h"

#include <pthread.h>

/**
 * initializes a new process container according to max number of processes given
 * since it is called before any asynchronous run is taking place,
 * this function is not thread safe
 */
proc_cont_t* init_proc_cont(int nprocs, mmu_t* mmu);

/**
 * creates a new process and adds to the proc_cont
 *
 * if there an error has occurred a number < 0 will be returned
 */
int init_process(proc_cont_t* proc_cont);

/**
 * destroys the process container given,
 * sends a stop message to all processes
 * free all memory associated with each process
 * free proc_cont
 */
errcode_t proc_cont_destroy(proc_cont_t* proc_cont);

/**
 * sends a stop message to the process with the given id
 *
 * @return -
 * 			ecFail - id is not within the range of process number
 * 			ecSuccess - process delete message was sent
 * 			ecNotFound - in case that a process with the given id does not exists
 */
errcode_t process_destroy(proc_cont_t* proc_cont, procid_t id);

/**
 * This is the function being run by the process
 */
BOOL process_func(void* arg);

#endif /* PCB_H_ */

//TODO add functions
//TODO sync
