#include "disk.h"

#include <malloc.h>
#include <string.h>
#include <assert.h>

static BYTE* get_page_start(disk_t* disk, int page)
{
	return &disk->data[page*disk->page_size];
}

errcode_t disk_init(disk_t* disk, int npages, int pagesize)
{
	disk->data = calloc(npages, pagesize);
	if (disk->data == NULL)
	{
		return ecFail;
	}
	disk->orig_addr = disk->data;

	disk->npages = npages;
	disk->page_size = pagesize;
	return ecSuccess;
}

errcode_t disk_get_page(disk_t* disk, int page, BYTE *page_data)
{
	assert(disk->orig_addr == disk->data);
	assert(page < disk->npages);
	memcpy(page_data, get_page_start(disk, page), disk->page_size);
	return ecSuccess;
}

errcode_t disk_set_page(disk_t* disk, int page, BYTE *page_data)
{
	assert(disk->orig_addr == disk->data);
	assert(page < disk->npages);
	memcpy(get_page_start(disk, page), page_data, disk->page_size);
	return ecSuccess;
}

void disk_destroy(disk_t* disk)
{
	assert(disk->orig_addr == disk->data);
	free (disk->data);
	disk->data = NULL;
}

#include "tests/disk_tests.c"
