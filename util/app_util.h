/*
 * ui_util.h
 *
 *  Created on: May 19, 2010
 *      Author: Freifeld Royi
 */

#ifndef APP_UTIL_H_
#define APP_UTIL_H_

#include "vmsim_types.h"
#include "sim/ipt.h"
#include "sim/mm.h"
#include "sim/disk.h"

typedef struct
{
	BOOL initialized;

	unsigned max_num_of_proc;
	unsigned page_size;
	unsigned num_of_proc_page;
	unsigned shift_clock;

	ipt_t ipt_table;
	mm_t main_memory;
	disk_t disk;
	//TODO fill up later on
} app_data_t;

#define APP_DATA(x)	((app_data_t *) (x))
#define APP_DATA_NUM_OF_PROC(x) APP_DATA((x)) -> max_num_of_proc
#define APP_DATA_PAGE_SIZE(x) APP_DATA((x)) -> page_size
#define APP_DATA_NUM_OF_PROC_PAGE(x) APP_DATA((x)) -> num_of_proc_page
#define APP_DATA_SHIFT_CLOCK(x) APP_DATA((x)) -> shift_clock
#define APP_DATA_INIT(x) APP_DATA((x)) -> initialized
#define APP_DATA_IPT(x) APP_DATA((x)) -> ipt_table
#define APP_DATA_MM(x) APP_DATA((x)) -> main_memory
#define APP_DATA_DISK(x) APP_DATA((x)) -> disk
/**
 * prints the given MM unit
 */
void print_MM(mm_t* mm);

/**
 * prints the IPT table
 */
void print_MMU_table(ipt_t* table);

/**
 * creates a new process and prints its id on screen
 * updates all data structures
 * TODO maybe add parameters later
 * returns the process' id or -1 in case of a failure
 */
int create_process();

/**
 * deletes the process with the given id
 * or prints a message if an error occurred.
 * NOTE: the UI thread will perform this job
 * 	and the UI will not return until this job is finished
 */
void del_process(int id);

/**
 *prints the registers of the aging algorithm
 */
//void print_registers(register_t** reg);

/**
 *prints the HAT
 */
//void print_HAT(hat_t* hat);

/**
 * switches the given system to Monitor mode
 */
//void monitor_on();

/**
 * switches the system to No Monitor mode
 */
//void monitor_off();

void load_app_data(char* file_name, app_data_t* app_data);

/**
 * free all application related data
 */
void free_app_data(app_data_t* app_data);

#endif /* APP_UTIL_H_ */

//TODO finish "commented" functions as the project grows
