#include "mmu.h"
#include "prm.h"

#include "util/logger.h"

#include <pthread.h>

#include <string.h>
#include <stdlib.h>
#include <assert.h>

#define MMU_ACQUIRE_DISKMAP(_mmu) DEBUG("Acquire diskmap\n");pthread_mutex_lock(&(_mmu)->diskmap_lock); DEBUG("OK\n")
#define MMU_RELEASE_DISKMAP(_mmu) DEBUG("Release diskmap\n");pthread_mutex_unlock(&(_mmu)->diskmap_lock); DEBUG("OK\n")

#define MMU_ACQUIRE_PAGE(_mmu, _pfn) DEBUG1("Locking PFN %d\n", _pfn);pthread_mutex_lock(&(_mmu)->page_frame_locks[_pfn])
#define MMU_RELEASE_PAGE(_mmu, _pfn) pthread_mutex_unlock(&(_mmu)->page_frame_locks[_pfn]);DEBUG1("Released PFN %d\n", _pfn)

#define IPT_READ_START(_mmu) DEBUG("Acquiring IPT R\n");rwlock_acquire_read(&(_mmu)->ipt_lock)
#define IPT_READ_END(_mmu) rwlock_release_read(&(_mmu)->ipt_lock);DEBUG("Released IPT R\n")
#define IPT_WRITE_START(_mmu) DEBUG("Acquiring IPT W\n");rwlock_acquire_write(&(_mmu)->ipt_lock)
#define IPT_WRITE_END(_mmu) rwlock_release_write(&(_mmu)->ipt_lock);DEBUG("Released IPT W\n")

void handle_out_of_mem()
{

}

static BOOL disk_map_comparator(void* k1, void* k2)
{
	return !VIRT_ADDR_EQ(*(virt_addr_t*)k1, *(virt_addr_t*)k2);
}

static errcode_t
mmu_pin_page_and_reference(mmu_t* mmu, virt_addr_t vaddr, phys_addr_t* mem_page, ipt_ref_t reftype)
{
	errcode_t errcode;
	DEBUG2("Trying to pin (%d,%d)\n", VIRT_ADDR_PID(vaddr), VIRT_ADDR_PAGE(vaddr));
	IPT_READ_START(mmu);
/*The while loop is because the pagefault handler must be called with an unlocked MMU,
 * and maybe the page was swapped out again after we swapped it in but before we locked
 * the MMU.
 * */
	while (ipt_translate(&mmu->mem_ipt, vaddr, mem_page) != ecSuccess)
	{
		IPT_READ_END(mmu);
		errcode = prm_pagefault(vaddr);
		assert (errcode == ecSuccess);
		IPT_READ_START(mmu);
	}

	MMU_ACQUIRE_PAGE(mmu, *mem_page);

	if (reftype != refNone)
	{
		ipt_reference(&mmu->mem_ipt, vaddr, reftype);
	}

	IPT_READ_END(mmu);
	DEBUG2("Pinned (%d,%d)\n", VIRT_ADDR_PID(vaddr), VIRT_ADDR_PAGE(vaddr));
	return ecSuccess;
}

errcode_t
mmu_pin_page(mmu_t* mmu, virt_addr_t vaddr, phys_addr_t* mem_page)
{
	return mmu_pin_page_and_reference(mmu, vaddr, mem_page, refNone);
}

errcode_t
mmu_unpin_page(mmu_t* mmu, virt_addr_t vaddr)
{
	phys_addr_t mem_page;
	errcode_t errcode = ipt_translate(&mmu->mem_ipt, vaddr, &mem_page);
	DEBUG2("Unpinning %d:%d\n", VIRT_ADDR_PID(vaddr), VIRT_ADDR_PAGE(vaddr));
	assert(errcode == ecSuccess);
	MMU_RELEASE_PAGE(mmu, mem_page);
	return ecSuccess;
}

errcode_t mmu_init(mmu_t* mmu, mm_t* mem, disk_t* disk)
{
	int i;
	pthread_mutexattr_t diskmap_lock_attr;

	mmu->mem = mem;
	mmu->disk = disk;
	ipt_init(&mmu->mem_ipt, MM_NUM_OF_PAGES(mem));
	map_init(&mmu->disk_map, sizeof(virt_addr_t), sizeof(phys_addr_t), disk_map_comparator);

	pthread_mutexattr_init(&diskmap_lock_attr);
//	pthread_mutexattr_settype(&diskmap_lock_attr, PTHREAD_MUTEX_RECURSIVE_NP);
	pthread_mutex_init(&mmu->diskmap_lock, &diskmap_lock_attr);

	rwlock_init(&mmu->ipt_lock);
	mmu->page_frame_locks = (pthread_mutex_t*)calloc(MM_NUM_OF_PAGES(mem), sizeof(pthread_mutex_t));
	for (i=0; i<MM_NUM_OF_PAGES(mem); ++i)
	{
		pthread_mutex_init(&mmu->page_frame_locks[i], NULL);
	}

	return ecSuccess;
}

errcode_t mmu_map_page_unlocked(mmu_t* mmu, virt_addr_t addr)
{
	assert(!ipt_has_translation(&mmu->mem_ipt, addr));
	DEBUG2("Mapping %d:%d\n", VIRT_ADDR_PID(addr), VIRT_ADDR_PAGE(addr));
	return ipt_add(&mmu->mem_ipt, addr);
}

errcode_t mmu_map_page(mmu_t* mmu, virt_addr_t addr)
{
	errcode_t errcode;
	IPT_WRITE_START(mmu);
	errcode = mmu_map_page_unlocked(mmu, addr);
	IPT_WRITE_END(mmu);
	return errcode;
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

errcode_t mmu_alloc_page(mmu_t* mmu, virt_addr_t addr, int backing_page)
{
	errcode_t errcode;

	mmu_block_alloc_free(mmu);
	IPT_WRITE_START(mmu);

	DEBUG2("Allocating (%d:%d)\n", VIRT_ADDR_PID(addr), VIRT_ADDR_PAGE(addr));

	errcode = mmu_map_page_unlocked(mmu, addr);
	//we don't care about the return code - if there was not enough room in the
	//memory, we'll map the page only to the disk, and on the next access to it
	//it will be swapped in.
	assert(map_get(&mmu->disk_map, &addr, NULL) == ecNotFound);
	errcode = map_set(&mmu->disk_map, &addr, &backing_page);
	DEBUG2("Added disk mapping: (%d:%d)\n", VIRT_ADDR_PID(addr), VIRT_ADDR_PAGE(addr));
	assert(errcode == ecSuccess);

	IPT_WRITE_END(mmu);
	mmu_release_alloc_free(mmu);
	return errcode;
}

errcode_t mmu_unmap_page_unlocked(mmu_t* mmu, virt_addr_t page)
{
	errcode_t errcode;
	phys_addr_t mem_page;
	DEBUG2("Unmapping %d:%d\n", VIRT_ADDR_PID(page), VIRT_ADDR_PAGE(page));
	if (ipt_translate(&mmu->mem_ipt, page, &mem_page) == ecSuccess)
	{
		errcode = ipt_remove(&mmu->mem_ipt, page);
		MMU_RELEASE_PAGE(mmu, mem_page);
		assert( errcode == ecSuccess);
	}
	return errcode;
}

errcode_t mmu_free_page(mmu_t* mmu, virt_addr_t page)
{
	phys_addr_t mem_page;
	DEBUG2("Freeing (%d:%d)\n", VIRT_ADDR_PID(page), VIRT_ADDR_PAGE(page));

	mmu_block_alloc_free(mmu);

	IPT_READ_START(mmu);

	if (ipt_translate(&mmu->mem_ipt, page, &mem_page) == ecSuccess)
	{
		mmu_pin_page(mmu, page, &mem_page);
	}

	IPT_READ_END(mmu);


	assert(map_get(&mmu->disk_map, &page, NULL)!=ecNotFound);
	mmu_unmap_page_unlocked(mmu, page);
	map_remove(&mmu->disk_map, &page);


	mmu_release_alloc_free(mmu);

	return ecSuccess;
}

errcode_t mmu_read(	mmu_t* mmu,
					virt_addr_t page,
					unsigned offset,
					unsigned nbytes,
					BYTE* buf)
{
	phys_addr_t mem_page;
	phys_addr_t mem_addr;
	errcode_t errcode;
	assert(offset + nbytes <= MM_PAGE_SIZE(mmu->mem));

	mmu_block_alloc_free(mmu);

	errcode = mmu_pin_page_and_reference(mmu, page, &mem_page, refRead);
	assert (errcode == ecSuccess);

	///TODO change this when we have a proper API for MM
	mem_addr = mem_page * MM_PAGE_SIZE(mmu->mem) + offset;
	memcpy(buf, MM_DATA(mmu->mem) + mem_addr, nbytes);

	errcode = mmu_unpin_page(mmu, page);

	mmu_release_alloc_free(mmu);

	return errcode;
}

errcode_t mmu_write(mmu_t* mmu,
					virt_addr_t page,
					unsigned offset,
					unsigned nbytes,
					BYTE* buf)
{
	phys_addr_t mem_page;
	phys_addr_t mem_addr = 0;
	errcode_t errcode;
	assert(offset + nbytes <= MM_PAGE_SIZE(mmu->mem));

	mmu_block_alloc_free(mmu);
	errcode = mmu_pin_page_and_reference(mmu, page, &mem_page, refWrite);

	assert (errcode == ecSuccess);


	///TODO change this when we have a proper API for MM
	mem_addr = mem_page * MM_PAGE_SIZE(mmu->mem) + offset;
	memcpy(MM_DATA(mmu->mem) + mem_addr, buf, nbytes);

	errcode = mmu_unpin_page(mmu, page);

	mmu_release_alloc_free(mmu);

	return errcode;
}

errcode_t mmu_sync_to_backing_page_unlocked(mmu_t* mmu, virt_addr_t page)
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

errcode_t mmu_sync_from_backing_page_unlocked(mmu_t* mmu, virt_addr_t page)
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
	assert(errcode != ecNotFound);
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

virt_addr_t mmu_phys_to_virt(mmu_t* mmu, phys_addr_t phys_addr)
{
	virt_addr_t vaddr;
	ipt_reverse_translate(&mmu->mem_ipt, phys_addr, &vaddr);
	return vaddr;
}

void mmu_block_alloc_free(mmu_t* mmu)
{
	MMU_ACQUIRE_DISKMAP(mmu);
}

void mmu_release_alloc_free(mmu_t* mmu)
{
	MMU_RELEASE_DISKMAP(mmu);
}

void mmu_destroy(mmu_t* mmu)
{
	mmu_block_alloc_free(mmu);
	ipt_destroy(&mmu->mem_ipt);
	map_destroy(&mmu->disk_map);
	mmu_release_alloc_free(mmu);

	pthread_mutex_destroy(&mmu->diskmap_lock);
	///TODO finish this
}

#include "tests/mmu_tests.c"
