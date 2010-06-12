#include "disk.h"

#include <malloc.h>
#include <string.h>
#include <assert.h>

#define READ_START(_disk) rwlock_acquire_read(&(_disk)->lock)
#define WRITE_START(_disk) rwlock_acquire_write(&(_disk)->lock)
#define READ_END(_disk) rwlock_release_read(&(_disk)->lock)
#define WRITE_END(_disk) rwlock_release_write(&(_disk)->lock)

///NOTE - static functions must be called with locked held.

static BYTE* get_page_start(disk_t* disk, int page)
{
	return &disk->data[page*disk->page_size];
}

static BOOL is_page_allocated(disk_t* disk, int page)
{
	return (page>=0) && bitmap_get(&disk->alloc_bitmap, page);
}

static void alloc_page(disk_t* disk, int page)
{
	assert(!is_page_allocated(disk, page));
	bitmap_set(&disk->alloc_bitmap, page);
}

static void free_page(disk_t* disk, int page)
{
	assert(is_page_allocated(disk, page));
	bitmap_clear(&disk->alloc_bitmap, page);
}

static BOOL hasEnoughConsecutivePages(disk_t* disk, int page_idx_start, int npages)
{
	int i;

	if (page_idx_start + npages > disk->npages)
	{
		return FALSE;
	}
	for (i=0; i < npages; ++i)
	{
		if (bitmap_get(&disk->alloc_bitmap, page_idx_start + i) == TRUE)
		{
			return FALSE;
		}
	}
	return TRUE;
}


errcode_t disk_init(disk_t* disk, int npages, int pagesize, int blocksize)
{
	disk->data = calloc(npages, pagesize);
	if (disk->data == NULL)
	{
		return ecFail;
	}
	disk->orig_addr = disk->data;

	disk->npages = npages;
	disk->page_size = pagesize;
	disk->process_block_size = blocksize;

	bitmap_init(&disk->alloc_bitmap, npages);
	rwlock_init(&disk->lock);

	return ecSuccess;
}

errcode_t disk_get_page(disk_t* disk, int page, BYTE *page_data)
{
	READ_START(disk);
	assert(disk->data != NULL);
	assert(disk->orig_addr == disk->data);
	assert(page < disk->npages);
	if (!is_page_allocated(disk, page))
	{
		READ_END(disk);
		return ecFail;
	}
	memcpy(page_data, get_page_start(disk, page), disk->page_size);
	READ_END(disk);
	return ecSuccess;
}

errcode_t disk_set_page(disk_t* disk, int page, BYTE *page_data)
{
	WRITE_START(disk);
	assert(disk->data != NULL);
	assert(disk->orig_addr == disk->data);
	assert(page < disk->npages);

	if (!is_page_allocated(disk, page))
	{
		WRITE_END(disk);
		return ecFail;
	}
	memcpy(get_page_start(disk, page), page_data, disk->page_size);
	WRITE_END(disk);
	return ecSuccess;
}

errcode_t disk_alloc_process_block(disk_t* disk, int* page)
{
	int pageidx;

	WRITE_START(disk);
	assert(disk->data != NULL);

	//find the first clear page. since all pages are the same size, this
	// should also be the first available block of pages of the proper size...
	if (bitmap_first_clear(&disk->alloc_bitmap, page) != ecSuccess)
	{
		WRITE_END(disk);
		return ecFail;
	}

	//...unless npages is not a multiple of process_block_size, in which case
	// we may be left with some extra pages that are not enough for a whole block.
	if (!hasEnoughConsecutivePages(disk, *page, disk->process_block_size))
	{
		WRITE_END(disk);
		return ecFail;
	}

	for (pageidx=0;pageidx<disk->process_block_size; ++pageidx)
	{
		alloc_page(disk, *page + pageidx);
	}

	WRITE_END(disk);
	return ecSuccess;
}

errcode_t disk_free_process_block(disk_t* disk, int page)
{
	int pageidx;
	WRITE_START(disk);
	assert(disk->data != NULL);

	for (pageidx = 0; pageidx < disk->process_block_size; ++pageidx)
	{
		free_page(disk, page + pageidx);
	}

	WRITE_END(disk);
	return ecSuccess;
}

void disk_destroy(disk_t* disk)
{
	WRITE_START(disk);
	assert(disk->orig_addr == disk->data);

	free (disk->data);
	disk->data = NULL;

	bitmap_destroy(&disk->alloc_bitmap);
	WRITE_END(disk);
	rwlock_destroy(&disk->lock);
}

#include "tests/disk_tests.c"
