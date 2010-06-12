#ifndef DISK_H_
#define DISK_H_

/**A simulation of a disk-based swap region*/

#include "util/bitmap.h"
#include "util/vmsim_types.h"
#include "util/locks.h"

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

/**Initialize a disk.
 * @param disk the disk to initialize
 * @param npages the number of pages that the disk should contain
 * @param pagesize the size (in bytes) of a disk's page
 * @param blocksize the number of contiguous pages a process stores on the disk.
 *
 * @return ecSuccess on success, some other code on failure.
 * */
errcode_t disk_init(disk_t* disk, int npages, int pagesize, int blocksize);

/**Allocate the pages for a process's page-block.
 * @param disk the disk to use
 * @param page 	a pointer that receives the absolute index of the first page in
 * 				the allocated block
 * @return ecSuccess on success, some other code on failure.
 * */
errcode_t disk_alloc_process_block(disk_t* disk, int* page);

/**Free the pages from a process's page-block.
 * The page is assumed to have been allocated.
 * @param disk the disk to use
 * @param page the absolute index of the first page in the allocated block
 * @return ecSuccess on success, some other code on failure.
 * */
errcode_t disk_free_process_block(disk_t* disk, int page);

/**Read a page's content
 *
 * @param disk the disk to use
 * @param page the absolute index of the page to read
 * @param page_data a pointer to a buffer where the page's data is copied. The
 * 					buffer is assumed to be at least as large as a page.
 * @return ecSuccess on success, some other code on failure.
 * */
errcode_t disk_get_page(disk_t* disk, int page, BYTE *page_data);

/**Write a page's content
 *
 * @param disk the disk to use
 * @param page the absolute index of the page to modify
 * @param page_data a pointer to a buffer from which the page's data will be copied.
 * 					The buffer is assumed to be at least as large as a page.
 * @return ecSuccess on success, some other code on failure.
 * */
errcode_t disk_set_page(disk_t* disk, int page, BYTE *page_data);


/**Finalize a disk.
 * @param disk the disk to finalize
 * */
void disk_destroy(disk_t* disk);

#endif /* DISK_H_ */
