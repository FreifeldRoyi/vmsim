/*
 * disk.h
 *
 *  Created on: 12/05/2010
 *      Author: tom
 */

#ifndef DISK_H_
#define DISK_H_

#include "util/bitmap.h"
#include "util/vmsim_types.h"
#include "util/rwlock.h"

typedef struct {
	BYTE* data;

	int npages;
	int page_size;
	int process_block_size;

	bitmap_t alloc_bitmap;

	rwlock_t lock;

	void* orig_addr;//used for debugging
}disk_t;

#define DISK_NUM_OF_PAGES(_disk) ((_disk)->npages)

errcode_t disk_init(disk_t* disk, int npages, int pagesize, int blocksize);

errcode_t disk_alloc_process_block(disk_t* disk, int* page);
errcode_t disk_free_process_block(disk_t* disk, int page);

errcode_t disk_get_page(disk_t* disk, int page, BYTE *page_data);
errcode_t disk_set_page(disk_t* disk, int page, BYTE *page_data);

void disk_destroy(disk_t* disk);

#endif /* DISK_H_ */
