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
#include "sim/mmu.h"

typedef struct
{
	BOOL initialized;

	unsigned max_num_of_proc;
	unsigned page_size;
	unsigned num_of_proc_page;
	unsigned shift_clock;

	mmu_t* mmu;
	//TODO fill up later on
} app_data_t;

#define APP_DATA(x)	((app_data_t *) (x))
#define APP_DATA_NUM_OF_PROC(x) APP_DATA((x)) -> max_num_of_proc
#define APP_DATA_PAGE_SIZE(x) APP_DATA((x)) -> page_size
#define APP_DATA_NUM_OF_PROC_PAGE(x) APP_DATA((x)) -> num_of_proc_page
#define APP_DATA_SHIFT_CLOCK(x) APP_DATA((x)) -> shift_clock
#define APP_DATA_INIT(x) APP_DATA((x)) -> initialized
#define APP_DATA_MMU(x) APP_DATA((x)) -> mmu
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
 * finds process with the given ID.
 *
 * if file_name is NULL, prints to stdout
 * else prints to specified file
 * offset > 0 => loop
 * offset < 0 => normal read
 */
void sim_read(int vaddr, int id, int off,int amount, char* file_name);

void write(int vaddr, int id, char* s, int amount);

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

#endif /* APP_UTIL_H_ */

//TODO finish "commented" functions as the project grows
