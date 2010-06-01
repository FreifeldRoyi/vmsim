/*
 * mm.h
 *
 *  Created on: May 19, 2010
 *      Author: Freifeld Royi
 */

#ifndef MM_H_
#define MM_H_

#include "util/bitmap.h"
#include "util/vmsim_types.h"
#include "util/rwlock.h"

typedef struct _mm_t
{
	BYTE* data;

	int npages;
	int page_size;

	bitmap_t bitmap;

	rwlock_t lock;

	void* orig_addr; //debug
} mm_t;

#define MM(x) ((mm_t *) (x))
#define MM_DATA(x) MM((x)) -> data
#define MM_LOCK(x) MM((x)) -> lock
#define MM_BITMAP(x) MM((x)) -> bitmap
#define MM_NUM_OF_PAGES(x) MM((x)) -> npages
#define MM_PAGE_SIZE(x) MM((x)) -> page_size

/**
 * initializes the main memory
 * according to the given parameters
 * will return ecFail if initialization failed
 * will return ecSuccess if initialization success
 *
 * @param mm - the pointer to the main memory to be allocated
 * @param npages - number of pages in the main memory
 * @param pagesize - size of each page
 */
errcode_t mm_init(mm_t* mm, int npages, int pagesize);

/**
 * gets a copy of a page already set in the main memory
 * if succeeded ecSuccess will be returned
 * ecFail otherwise
 *
 * @param mm - the main memory
 * @param page - the page
 * @param page_data - where to save the data
 */
errcode_t mm_get_page(mm_t* mm, int page, BYTE* page_data);

/**
 * resets a page inside the main memory
 * ecSuccess will be returned if operation succeeded
 * ecFail otherwise
 *
 * @param mm - the main memory
 * @param page - the page to set
 * @param page_data - the data to be set
 */
errcode_t mm_set_page(mm_t* mm, int page, BYTE* page_data);

/**
 * deallocates the main memory
 */
//TODO pay attention, in non-graceful exit, the mm may contain data being held also by the disk, needs to pay attention not to "double free" memory. will cause glibc errors
void mm_destroy(mm_t* mm);

#endif /* MM_H_ */
