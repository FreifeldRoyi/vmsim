/*
 * disk.h
 *
 *  Created on: 12/05/2010
 *      Author: tom
 */

#ifndef DISK_H_
#define DISK_H_

#include "util/vmsim_types.h"

typedef struct {
	BYTE* data;

	int npages;
	int page_size;

	void* orig_addr;//used for debugging
}disk_t;

errcode_t disk_init(disk_t* disk, int npages, int pagesize);

errcode_t disk_alloc_process_block(disk_t* disk, int* page);

errcode_t disk_get_page(disk_t* disk, int page, BYTE *page_data);
errcode_t disk_set_page(disk_t* disk, int page, BYTE *page_data);

void disk_destroy(disk_t* disk);

#endif /* DISK_H_ */
