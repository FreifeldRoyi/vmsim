#include "disk.h"

#include <malloc.h>
#include <string.h>
#include <assert.h>

static BYTE* get_page_start(disk_t* disk, int page)
{
	return &disk->data[page*disk->page_size];
}

static BOOL is_page_allocated(disk_t* disk, int page)
{
	return bitmap_get(&disk->alloc_bitmap, page);
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

	return ecSuccess;
}

errcode_t disk_get_page(disk_t* disk, int page, BYTE *page_data)
{
	assert(disk->orig_addr == disk->data);
	assert(page < disk->npages);
	assert(is_page_allocated(disk, page));
	memcpy(page_data, get_page_start(disk, page), disk->page_size);
	return ecSuccess;
}

errcode_t disk_set_page(disk_t* disk, int page, BYTE *page_data)
{
	assert(disk->orig_addr == disk->data);
	assert(page < disk->npages);
	assert(is_page_allocated(disk, page));
	memcpy(get_page_start(disk, page), page_data, disk->page_size);
	return ecSuccess;
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


errcode_t disk_alloc_process_block(disk_t* disk, int* page)
{
	int pageidx;

	//find the first clear page. since all pages are the same size, this
	// should also be the first available block of pages of the proper size...
	if (bitmap_first_clear(&disk->alloc_bitmap, page) != ecSuccess)
	{
		return ecFail;
	}

	//...unless npages is not a multiple of process_block_size, in which case
	// we may be left with some extra pages that are not enough for a whole block.
	if (!hasEnoughConsecutivePages(disk, *page, disk->process_block_size))
	{
		return ecFail;
	}

	for (pageidx=0;pageidx<disk->process_block_size; ++pageidx)
	{
		alloc_page(disk, *page + pageidx);
	}
	return ecSuccess;
}

errcode_t disk_free_process_block(disk_t* disk, int page)
{
	int pageidx;

	for (pageidx = 0; pageidx < disk->process_block_size; ++pageidx)
	{
		free_page(disk, page + pageidx);
	}
	return ecSuccess;
}

void disk_destroy(disk_t* disk)
{
	assert(disk->orig_addr == disk->data);
	free (disk->data);
	disk->data = NULL;
}

#include "tests/disk_tests.c"
