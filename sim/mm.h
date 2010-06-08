/*
 * mm.h
 *
 *  Created on: May 19, 2010
 *      Author: Freifeld Royi
 */

#ifndef MM_H_
#define MM_H_

#include "util/vmsim_types.h"
#include "util/rwlock.h"

typedef struct _mm_t
{
	BYTE* data;

	int npages;
	int page_size;

	// bitmap_t bitmap; TODO not needed since ipt is used as mm's bitmap

	rwlock_t lock;

	void* orig_addr; //debug
} mm_t;

#define MM(x) ((mm_t *) (x))
#define MM_DATA(x) (x) -> data
#define MM_LOCK(x) (x) -> lock
//#define MM_BITMAP(x) MM((x)) -> bitmap
#define MM_NUM_OF_PAGES(x) (x) -> npages
#define MM_PAGE_SIZE(x) (x) -> page_size

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
 * reads a page from the main memory
 * if succeeded ecSuccess will be returned
 * ecFail will be returned in case of unallocated memory
 *
 * assumptions:
 *	mm is not NULL
 *	page is an unsigned int between 0 to NUM_OF_PAGES_IN_MM - 1
 *
 * @param mm - the main memory
 * @param page - the page to read
 * @param buf - outer pointer, a buffer to write to. It is assumed that buf is atleast  PAGE_SIZE
 */
errcode_t mm_read(mm_t* mm, int page, BYTE* buf);

/**
 * write a page to disk
 * It is assumed that write operation will be monitored via
 * mmu, thus no need for allocation check.
 * NOTE: use this function after using the alloc_page or there may be data loss
 *
 * @param mm - the main memory
 * @param page - the page to write to
 * @param page_data - Buffer of data to read from
 */
errcode_t mm_write(mm_t* mm, int page, BYTE* buf);

/**
 * deallocates the main memory
 */
void mm_destroy(mm_t* mm);

#endif /* MM_H_ */
