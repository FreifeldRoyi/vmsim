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

	ipt_t ipt_table;
	mm_t main_memory;
	disk_t disk;
	//TODO fill up later on
} app_data_t;

#define APP_DATA(x)	((app_data_t *) (x))
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

#endif /* APP_UTIL_H_ */

//TODO finish "commented" functions as the project grows
