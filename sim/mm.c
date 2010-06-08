/*
 * mm.c
 *
 *  Created on: May 26, 2010
 *      Author: Freifeld Royi
 */

#include "mm.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

#define READ_START(_mm) rwlock_acquire_read(&MM_LOCK(mm))
#define WRITE_START(_mm) rwlock_acquire_write(&MM_LOCK(mm))
#define READ_END(_mm) rwlock_release_read(&MM_LOCK(mm))
#define WRITE_END(_mm) rwlock_release_write(&MM_LOCK(mm))

static BYTE* get_page_start(mm_t* mm, int page)
{
	return &MM_DATA(mm)[page * MM_PAGE_SIZE(mm)];
}

errcode_t mm_init(mm_t* mm, int npages, int pagesize)
{
	MM_DATA(mm) = calloc(npages, pagesize);
	if (MM_DATA(mm) == NULL)
	{
		return ecFail;
	}
	mm->orig_addr = MM_DATA(mm);

	MM_NUM_OF_PAGES(mm) = npages;
	MM_PAGE_SIZE(mm) = pagesize;

	//bitmap_init(&(MM_BITMAP(mm)), npages);
	rwlock_init(&MM_LOCK(mm));

	return ecSuccess;
}

errcode_t mm_read(mm_t* mm, int page, BYTE* buf)
{
	READ_START(mm);

	assert(MM_DATA(mm) != NULL);
	assert(mm -> orig_addr == MM_DATA(mm));
	assert(page < MM_NUM_OF_PAGES(mm));

	/*if (!is_page_allocated(mm, page))
	{
		READ_END(mm);
		return ecFail;
	}*/

	memcpy(buf, get_page_start(mm,page), MM_PAGE_SIZE(mm));
	READ_END(mm);

	return ecSuccess;
}

errcode_t mm_write(mm_t* mm, int page, BYTE* buf)
{
	WRITE_START(mm);

	assert(MM_DATA(mm) != NULL);
	assert(mm -> orig_addr == MM_DATA(mm));
	assert(page < MM_NUM_OF_PAGES(mm));

	/*if (is_page_allocated(mm, page))
	{
		WRITE_END(mm);
		return ecFail;
	}*/

	memcpy(get_page_start(mm, page), buf, MM_PAGE_SIZE(mm));
	WRITE_END(mm);

	return ecSuccess;
}

void mm_destroy(mm_t* mm)
{
	WRITE_START(mm);
	assert(mm -> orig_addr == MM_DATA(mm));

	free(MM_DATA(mm));
	MM_DATA(mm) = NULL;

	//bitmap_destroy(&MM_BITMAP(mm));
	WRITE_END(mm);
	rwlock_destroy(&MM_LOCK(mm));
}
