/*
 * ui_util.h
 *
 *  Created on: May 19, 2010
 *      Author: Freifeld Royi
 */

#ifndef APP_UTIL_H_
#define APP_UTIL_H_

#include "vmsim_types.h"
#include "vmsim/ipt.h"
#include "vmsim/mm.h"
#include "vmsim/disk.h"
#include "vmsim/mmu.h"
#include "vmsim/ipt.h"
#include "vmsim/pcb.h"

typedef struct
{
	BOOL initialized;

	unsigned page_size;
	unsigned num_of_proc_page;
	unsigned shift_clock;

	proc_cont_t* proc_cont;
} app_data_t;

#define APP_DATA(x)	((app_data_t *) (x))
#define APP_DATA_PAGE_SIZE(x) (x) -> page_size
#define APP_DATA_NUM_OF_PROC_PAGE(x) (x) -> num_of_proc_page
#define APP_DATA_SHIFT_CLOCK(x) (x) -> shift_clock
#define APP_DATA_INIT(x) (x) -> initialized
#define APP_DATA_PROC_CONT(x) (x) -> proc_cont
#define APP_DATA_MMU(x) PROC_CONT_MMU(APP_DATA_PROC_CONT((x)))

/**
 * prints the given MM unit
 */
void print_MM(mm_t* mm);

void print_hit_rate(mmu_t* mmu);

/**
 * prints the IPT table
 */
void print_MMU_table(ipt_t* table);

/**
 * creates a new process and prints its id on screen
 * updates all data structures
 *
 * @return the process' id or -1 in case of a failure
 */
int create_process(app_data_t* app_data);

/**
 * deletes the process with the given id
 * or prints a message if an error occurred.
 * NOTE: the UI thread will perform this job
 * 	and the UI will not return until this job is finished
 */
void del_process(app_data_t* app_data, procid_t pid);

/**
 * next group of functions, sends a message to specified process according to
 * @param id.
 */
errcode_t read_process(proc_cont_t* proc_cont, int vaddr, int id, int amount);

errcode_t loop_read_process(proc_cont_t* proc_cont, int vaddr, int id, int off, int amount);

errcode_t read_to_file_process(proc_cont_t* proc_cont, int vaddr, int id, int amount, char* file_name);

errcode_t loop_read_to_file_process(proc_cont_t* proc_cont, int vaddr, int id, int off, int amount, char* file_name);

errcode_t write_process(proc_cont_t* proc_cont, int vaddr, int id, char* s);

errcode_t loop_write_process(proc_cont_t* proc_cont, int vaddr, int id, char c, int off, int amount);

/**
 * prints the registers of the aging algorithm
 */
void print_registers(ipt_t* ipt);

/**
 * prints the HAT
 *
 * @param ipt - the ipt table
 */
void print_HAT(ipt_t* ipt);

/**
 * switches the given system to Monitor mode
 */
void monitor_on();

/**
 * switches the system to No Monitor mode
 */
void monitor_off();

void debug_on();

/**
 * loads all application data associated with app_data_t struct
 * app_data should not be initialized or NULLified
 * NOTE: be sure to memset app_data to 0 before using this function
 *
 * @param file_name - the file name to be read, assumed to be correct
 * @param app_data - the pointer to the app_data to be set
 */
BOOL load_app_data(char* file_name, app_data_t* app_data);

/**
 * free all application related data
 */
void free_app_data(app_data_t* app_data);

/**
 * used as a lock for printing operations
 */
void wait_job_done();

/**
 * signal printing operations
 */
void signal_job_done();

/**
 * initializes printing lock
 */
void init_job_done();

/**
 * finalizes printing lock
 */
void destroy_job_done();

#endif /* APP_UTIL_H_ */
