#include "mmu.h"
#include "prm.h"

#include "util/logger.h"

#include "aging_daemon.h"

#include <pthread.h>

#include <string.h>
#include <stdlib.h>
#include <assert.h>

#define MMU_ACQUIRE_DISKMAP(_mmu) DEBUG("Acquire diskmap\n");pthread_mutex_lock(&(_mmu)->diskmap_lock); DEBUG("OK\n")
#define MMU_RELEASE_DISKMAP(_mmu) DEBUG("Release diskmap\n");pthread_mutex_unlock(&(_mmu)->diskmap_lock); DEBUG("OK\n")

static BOOL disk_map_comparator(void* k1, void* k2)
{
	return !VIRT_ADDR_PAGE_EQ(*(virt_addr_t*)k1, *(virt_addr_t*)k2);
}

errcode_t mmu_init(mmu_t* mmu, mm_t* mem, disk_t* disk, int aging_freq)
{
	errcode_t errcode;
	errcode = ipt_init(&mmu->mem_ipt, MM_NUM_OF_PAGES(mem));
	if (errcode != ecSuccess)
	{
		return errcode;
	}
	errcode = map_init(&mmu->disk_map, sizeof(virt_addr_t), sizeof(phys_addr_t), disk_map_comparator);
	if (errcode != ecSuccess)
	{
		return errcode;
	}

	mmu->mem = mem;
	mmu->disk = disk;
	mmu->aging_freq = aging_freq;

	memset(&mmu->stats, 0, sizeof(mmu->stats));

	return ecSuccess;
}

errcode_t mmu_map_page(mmu_t* mmu, virt_addr_t addr)
{
	errcode_t errcode;

	assert(!ipt_has_translation(&mmu->mem_ipt, addr));
	DEBUG2("Mapping %d:%d\n", VIRT_ADDR_PID(addr), VIRT_ADDR_PAGE(addr));
	errcode = ipt_add(&mmu->mem_ipt, addr);
	return errcode;
}

static errcode_t mmu_alloc_page(mmu_t* mmu, virt_addr_t addr, int backing_page)
{
	errcode_t errcode;

	DEBUG2("Allocating (%d:%d)\n", VIRT_ADDR_PID(addr), VIRT_ADDR_PAGE(addr));

	errcode = mmu_map_page(mmu, addr);
	//we don't care about the return code - if there was not enough room in the
	//memory, we'll map the page only to the disk, and on the next access to it
	//it will be swapped in.
	assert(map_get(&mmu->disk_map, &addr, NULL) == ecNotFound);
	errcode = map_set(&mmu->disk_map, &addr, &backing_page);
	DEBUG2("Added disk mapping: (%d:%d)\n", VIRT_ADDR_PID(addr), VIRT_ADDR_PAGE(addr));
	assert(errcode == ecSuccess);

	return errcode;
}

static errcode_t mmu_free_page(mmu_t* mmu, virt_addr_t page)
{
	DEBUG2("Freeing (%d:%d)\n", VIRT_ADDR_PID(page), VIRT_ADDR_PAGE(page));

	assert(map_get(&mmu->disk_map, &page, NULL)!=ecNotFound);

	mmu_unmap_page(mmu, page);//we don't check the return value because it's OK if the page isn't in memory.
	map_remove(&mmu->disk_map, &page);

	return ecSuccess;
}


errcode_t mmu_alloc_multiple(mmu_t* mmu, virt_addr_t first_addr, int npages, int first_backing_page)
{
	errcode_t errcode;
	int disk_page;
	virt_addr_t cur_vaddr;

	VIRT_ADDR_PID(cur_vaddr) = VIRT_ADDR_PID(first_addr);
	VIRT_ADDR_PAGE(cur_vaddr) = VIRT_ADDR_PAGE(first_addr);

	for (disk_page=first_backing_page; disk_page < first_backing_page + npages; ++disk_page)
	{
		//we intentionally don't lock the MMU for the whole operation. this way if
		//several processes allocate pages at the same time, some pages of each process
		//will get to live in memory instead of all the pages of a few processes.
		errcode = mmu_alloc_page(mmu, cur_vaddr, disk_page);
		if (errcode != ecSuccess)
		{
			return errcode;
		}
		VIRT_ADDR_PAGE(cur_vaddr)++;
	}
	return ecSuccess;
}

errcode_t mmu_free_multiple(mmu_t* mmu, virt_addr_t first_addr, int npages)
{
	errcode_t errcode;
	int cur_page;
	virt_addr_t cur_vaddr;

	VIRT_ADDR_PID(cur_vaddr) = VIRT_ADDR_PID(first_addr);
	VIRT_ADDR_PAGE(cur_vaddr) = VIRT_ADDR_PAGE(first_addr);

	for (cur_page=0; cur_page < npages; ++cur_page)
	{
		//we intentionally don't lock the MMU for the whole operation. this way if
		//several processes allocate pages at the same time, some pages of each process
		//will get to live in memory instead of all the pages of a few processes.
		errcode = mmu_free_page(mmu, cur_vaddr);
		if (errcode != ecSuccess)
		{
			return errcode;
		}
		VIRT_ADDR_PAGE(cur_vaddr)++;
	}
	return ecSuccess;
}

errcode_t mmu_unmap_page(mmu_t* mmu, virt_addr_t page)
{
	errcode_t errcode;
	DEBUG2("Unmapping %d:%d\n", VIRT_ADDR_PID(page), VIRT_ADDR_PAGE(page));

	errcode = ipt_remove(&mmu->mem_ipt, page);
	assert(errcode != ecFail); //ecNotFound is OK

	return errcode;
}

static errcode_t mmu_pin_page(	mmu_t* mmu, virt_addr_t page)
{
	/*NOTE: the INFO printouts here are required by the assignment.
	 * */
	if (ipt_has_translation(&mmu->mem_ipt, page))
	{
		INFO2("IPT has a mapping for (%d:%d)\n", VIRT_ADDR_PID(page), VIRT_ADDR_PAGE(page));
		rwlock_acquire_write(&mmu->stats_lock);
		++mmu->stats.hits;
		++mmu->stats.nrefs;
		rwlock_release_write(&mmu->stats_lock);
		return ecSuccess;
	}
	else
	{
		INFO2("No mapping for (%d:%d) - A pagefault has occured.\n", VIRT_ADDR_PID(page), VIRT_ADDR_PAGE(page));
		rwlock_acquire_write(&mmu->stats_lock);
		++mmu->stats.nrefs;
		rwlock_release_write(&mmu->stats_lock);
		return prm_pagefault(page);
	}
}

static errcode_t mmu_unpin_page(	mmu_t* mmu,
						virt_addr_t page)
{
	return ecSuccess;
}


static errcode_t
mmu_age_pages_if_needed(mmu_t* mmu)
{
	unsigned refcount;
	ipt_ref_count(&mmu->mem_ipt, &refcount);

	DEBUG2("refcount=%d, aging_freq=%d\n", refcount, mmu->aging_freq);
	if (refcount >= mmu->aging_freq)
	{
		ipt_zero_ref_count(&mmu->mem_ipt);
		aging_daemon_update_pages();
	}
	return ecSuccess;
}

errcode_t mmu_read(	mmu_t* mmu,
					virt_addr_t addr,
					unsigned nbytes,
					BYTE* buf)
{
	phys_addr_t mem_page;
	phys_addr_t mem_addr;
	errcode_t errcode = ecSuccess;
	int offset = addr.offset;
	assert(offset + (int)nbytes <= MM_PAGE_SIZE(mmu->mem));

	errcode = mmu_pin_page(mmu, addr);
	assert (errcode == ecSuccess);

	errcode = ipt_reference(&mmu->mem_ipt, addr, refRead);
	assert (errcode == ecSuccess);

	errcode = ipt_translate(&mmu->mem_ipt, addr, &mem_page);

	///TODO change this when we have a proper API for MM
	mem_addr = mem_page * MM_PAGE_SIZE(mmu->mem) + offset;
	memcpy(buf, MM_DATA(mmu->mem) + mem_addr, nbytes);

	errcode = mmu_unpin_page(mmu, addr);

	mmu_age_pages_if_needed(mmu);

	return errcode;
}

errcode_t mmu_write(mmu_t* mmu,
					virt_addr_t addr,
					unsigned nbytes,
					BYTE* buf)
{
	phys_addr_t mem_page;
	phys_addr_t mem_addr;
	errcode_t errcode = ecSuccess;
	unsigned offset = addr.offset;
	assert(offset + nbytes <= (unsigned)MM_PAGE_SIZE(mmu->mem));

	errcode = mmu_pin_page(mmu, addr);
	assert (errcode == ecSuccess);

	errcode = ipt_reference(&mmu->mem_ipt, addr, refWrite);
	assert (errcode == ecSuccess);

	errcode = ipt_translate(&mmu->mem_ipt, addr, &mem_page);

	///TODO change this when we have a proper API for MM
	mem_addr = mem_page * MM_PAGE_SIZE(mmu->mem) + offset;
	memcpy(MM_DATA(mmu->mem) + mem_addr, buf, nbytes);

	errcode = mmu_unpin_page(mmu, addr);

	mmu_age_pages_if_needed(mmu);

	return errcode;
}

errcode_t mmu_sync_to_backing_page(mmu_t* mmu, virt_addr_t page)
{
	int disk_page;
	phys_addr_t mem_page;
	errcode_t errcode;
	BYTE* page_data;

	if (!ipt_is_dirty(&mmu->mem_ipt, page))
	{
		//if the page is not dirty, no need to write it.
		return ecSuccess;
	}

	errcode = ipt_translate(&mmu->mem_ipt, page, &mem_page);

	errcode = map_get(&mmu->disk_map, &page, &disk_page);
	assert(errcode != ecNotFound);
	if (errcode != ecSuccess)
	{
		return ecFail;
	}

	assert(errcode != ecNotFound);
	if (errcode != ecSuccess)
	{
		return ecFail;
	}

	///TODO change this when we have a proper API for MM
	page_data = MM_DATA(mmu->mem) + MM_PAGE_SIZE(mmu->mem)*mem_page;
	errcode = disk_set_page(mmu->disk,disk_page, page_data);

	return errcode;
}

errcode_t mmu_sync_from_backing_page(mmu_t* mmu, virt_addr_t page)
{
	int disk_page;
	phys_addr_t mem_page;
	errcode_t errcode;
	BYTE* page_data;

	errcode = ipt_translate(&mmu->mem_ipt, page, &mem_page);
	assert(errcode != ecNotFound);
	if (errcode != ecSuccess)
	{
		return ecFail;
	}

	errcode = map_get(&mmu->disk_map, &page, &disk_page);
	assert(errcode != ecNotFound);  //if this assert fails it means the page has no
									//backing store.
	if (errcode != ecSuccess)
	{
		return ecFail;
	}

	///TODO change this when we have a proper API for MM
	page_data = MM_DATA(mmu->mem) + MM_PAGE_SIZE(mmu->mem)*mem_page;
	errcode = disk_get_page(mmu->disk,disk_page, page_data);

	return errcode;
}

errcode_t mmu_for_each_mem_page(mmu_t* mmu, void (*func)(phys_addr_t, page_data_t*))
{
	return ipt_for_each_entry(&mmu->mem_ipt, func);
}

mmu_stats_t mmu_get_stats(mmu_t* mmu)
{
	mmu_stats_t ret;
	rwlock_acquire_read(&mmu->stats_lock);
	ret = mmu->stats;
	rwlock_release_read(&mmu->stats_lock);
	return ret;
}

void mmu_destroy(mmu_t* mmu)
{
	ipt_destroy(&mmu->mem_ipt);
	map_destroy(&mmu->disk_map);
}

#include "tests/mmu_tests.c"
