/*
 * mm.c
 *
 *  Created on: May 26, 2010
 *      Author: Freifeld Royi
 */

#include "mm.h"

#define READ_START(_mm) rwlock_acuire_read(MM(_mm) -> lock)
#define WRITE_START(_mm) rwlock_acquire_write(MM(_mm) -> lock)
#define READ_END(_mm) rwlock_release_read(&(MM(_mm) -> lock)
#define WRITE_END(_mm) rwlock_release_write(MM(_mm) -> lock)

static BOOL is_page_allocated(mm_t* mm, int page)
{
	return (page >= 0) && bitmap_get(&MM_BITMAP(mm), page);
}

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

	bitmap_init(&MM_BITMAP(mm), npages);
	rwlock_init(&MM_LOCK(mm));
}

errcode_t mm_get_page(mm_t* mm, int page, BYTE* page_data)
{
	READ_START(mm);
	assert(MM_DATA(mm) != NULL);
	assert(mm -> orig_addr == MM_DATA(mm));
	assert(page < MM_NUM_OF_PAGES(mm));

	if (!is_page_allocated(mm, page))
	{
		READ_END(mm);
		return ecFail;
	}

	memcpy(page_data, get_page_start(mm, page), page_data, MM_PAGE_SIZE(mm));
	READ_END(mm);
	return ecSuccess;
}

errcode_t mm_set_page(mm_t* mm, int page, BYTE* page_data)
{
	WRITE_START(mm);
	assert(MM_DATA(mm) != NULL);
	assert(mm->orig_addr == MM_DATA(mm));
	assert(page < MM_NUM_OF_PAGES(mm));

	if (!is_page_allocated(mm, page))
	{
		WRITE_END(mm);
		return ecFail;
	}

	memcpy(get_page_start(mm, page), page_data, MM_PAGE_SIZE(mm));
	WRITE_END(mm);
	return ecSuccess;
}

void mm_destroy(mm_t* mm)
{
	WRITE_START(mm);
	assert(mm -> orig_addr == MM_DATA(mm));

	free(MM_DATA(mm));
	MM_DATA(mm) = NULL;

	bitmap_destroy(&MM_BITMAP(mm));
	WRITE_END(mm);
	rwlock_destroy(&MM_LOCK(mm));
}
