#include "ipt.h"
#include "util/logger.h"

#include <stdlib.h>
#include <assert.h>
#include <stdio.h>

#define REFCNT_READ_START(_ipt) rwlock_acquire_read(&ipt->refcnt_lock)
#define REFCNT_READ_END(_ipt) rwlock_release_read(&ipt->refcnt_lock)
#define REFCNT_WRITE_START(_ipt) rwlock_acquire_write(&ipt->refcnt_lock)
#define REFCNT_WRITE_END(_ipt) rwlock_release_write(&ipt->refcnt_lock)

#define IPT_INVALID -1

static int ipt_hash(ipt_t* ipt, virt_addr_t addr)
{
	return (VIRT_ADDR_PID(addr) * VIRT_ADDR_PAGE(addr)) % ipt->size;
}

static int ipt_hat_idx_of(ipt_t* ipt, virt_addr_t addr)
{
	return ipt->hat[ipt_hash(ipt, addr)].ipt_idx;
}

void ipt_lock_vaddr(ipt_t* ipt, virt_addr_t addr)
{
	rwlock_acquire_read(&ipt->hat_lock);
	rwlock_acquire_write(&ipt->hat[ipt_hash(ipt, addr)].lock);
}

void ipt_unlock_vaddr(ipt_t* ipt, virt_addr_t addr)
{
	rwlock_release_write(&ipt->hat[ipt_hash(ipt, addr)].lock);
	rwlock_release_read(&ipt->hat_lock);
}

void ipt_lock_all_vaddr(ipt_t* ipt)
{
	rwlock_acquire_write(&ipt->hat_lock);
}

void ipt_unlock_all_vaddr(ipt_t* ipt)
{
	rwlock_release_write(&ipt->hat_lock);
}

static void ipt_set_hat(ipt_t* ipt, virt_addr_t addr, int idx)
{
	ipt->hat[ipt_hash(ipt, addr)].ipt_idx = idx;
}

static int ipt_get_next_free_entry(ipt_t* ipt)
{
	int ret;
	if (queue_size(ipt->free_pages) == 0)
	{
		ret = IPT_INVALID;
	}
	else
	{
		ret = (int)queue_pop(ipt->free_pages);
	}
	return ret;
}

static int get_vaddr_idx(ipt_t* ipt,virt_addr_t addr)
{
	int idx = ipt_hat_idx_of(ipt, addr);
	while ( (idx != -1) &&
			!VIRT_ADDR_EQ(ipt->entries[idx].page_data.addr, addr))
	{
		assert(ipt->entries[idx].page_data.valid);
		idx = ipt->entries[idx].next;
	}

	if (idx == -1) //we traversed the hashlist and no entry was found
	{
		return IPT_INVALID;
	}
/*
	if (!ipt->entries[idx].page_data.valid)
	{
		return IPT_INVALID;
	}*/
	return idx;
}

errcode_t ipt_init(ipt_t* ipt, int size)
{
	int i;

	ipt->entries = calloc(sizeof(ipt_entry_t), size);
	if (ipt->entries == NULL)
	{
		return ecFail;
	}
	ipt->hat = calloc(sizeof(hat_entry_t), size);
	if (ipt->hat == NULL)
	{
		return ecFail;
	}
	ipt->free_pages = queue_init();
	ipt->size = size;

	ipt->ref_count = 0;
	if (rwlock_init(&ipt->refcnt_lock) != ecSuccess)
	{
		return ecFail;
	}

	if (rwlock_init(&ipt->hat_lock) != ecSuccess)
	{
		return ecFail;
	}

	for (i=0; i<size; ++i)
	{
		ipt->entries[i].next =
			ipt->entries[i].prev = IPT_INVALID;
		ipt->hat[i].ipt_idx = IPT_INVALID;
		rwlock_init(&ipt->hat[i].lock);
		queue_push(ipt->free_pages, (void*)i);
	}

	DEBUG1("Created IPT %p\n", ipt->entries);
	return ecSuccess;
}

static void init_ipt_slot(ipt_t* ipt, int idx,virt_addr_t addr, int next, int prev)
{
	ipt->entries[idx].next = next;
	ipt->entries[idx].prev = prev;
	ipt->entries[idx].page_data.addr = addr;
	ipt->entries[idx].page_data.dirty = FALSE;
	ipt->entries[idx].page_data.referenced = FALSE;
	ipt->entries[idx].page_data.valid = TRUE;
	ipt->entries[idx].page_data.page_age = 0;
	SET_MSB(ipt->entries[idx].page_data.page_age, 1);
}

static void dump_list(ipt_t* ipt)
{
	int i;

	printf("HAT Dump:\nidx|ipt_idx\n");
	for (i=0; i<ipt->size; ++i)
	{
		printf("%3d|%d\n", i, ipt->hat[i].ipt_idx);
	}
	printf("\n\nIPT Dump:\nidx|vaddr|prev|next|valid\n");
	for (i=0; i<ipt->size; ++i)
	{
		printf("%3d|(%d:%d)|%4d|%4d|%s\n", 	i,
									VIRT_ADDR_PID(ipt->entries[i].page_data.addr),
									VIRT_ADDR_PAGE(ipt->entries[i].page_data.addr),
									ipt->entries[i].prev,
									ipt->entries[i].next,
									ipt->entries[i].page_data.valid?"Yes":"No");
	}
}

BOOL ipt_has_translation_unlocked(ipt_t* ipt, virt_addr_t addr)
{
	return get_vaddr_idx(ipt, addr) != IPT_INVALID;
}

BOOL ipt_has_translation(ipt_t* ipt, virt_addr_t addr)
{
	BOOL ret;

	ret = ipt_has_translation_unlocked(ipt, addr);

	return ret;
}

BOOL ipt_is_dirty(ipt_t* ipt, virt_addr_t addr)
{
	BOOL dirty;
	assert (ipt_has_translation_unlocked(ipt, addr));
	dirty = ipt->entries[get_vaddr_idx(ipt, addr)].page_data.dirty;
	return dirty;
}

errcode_t ipt_reference(ipt_t* ipt, virt_addr_t addr, ipt_ref_t reftype)
{
	assert (ipt_has_translation_unlocked(ipt, addr));

	ipt->entries[get_vaddr_idx(ipt, addr)].page_data.referenced = TRUE;
	if (reftype == refWrite)
	{
		ipt->entries[get_vaddr_idx(ipt, addr)].page_data.dirty = TRUE;
	}

	REFCNT_WRITE_START(ipt);
	++ipt->ref_count;
	REFCNT_WRITE_END(ipt);

	return ecSuccess;
}

errcode_t ipt_translate(ipt_t* ipt, virt_addr_t addr, phys_addr_t* paddr)
{
	*paddr = get_vaddr_idx(ipt, addr);
	assert(*paddr != IPT_INVALID);
	return ecSuccess;
}

static int ipt_hash_list_last_element(ipt_t* ipt, virt_addr_t addr)
{
	int idx = ipt_hat_idx_of(ipt, addr);
	int prev_idx = IPT_INVALID;

	while (idx != IPT_INVALID)
	{
		prev_idx = idx;
		idx = ipt->entries[idx].next;
	}

	return prev_idx;
}

errcode_t ipt_add(ipt_t* ipt, virt_addr_t addr)
{
	int tail_idx, new_idx;


	//first, allocate a new entry in the IPT
	new_idx = ipt_get_next_free_entry(ipt);
	if (new_idx == IPT_INVALID)
	{
		//free pages list is empty. no room in the IPT.
		return ecFail;
	}

	//now add the new IPT entry to the HAT/hashlist as needed
	tail_idx = ipt_hash_list_last_element(ipt, addr);
	if (tail_idx == IPT_INVALID)
	{
		//no hash list for this vaddr. create a new one.
		ipt_set_hat(ipt, addr, new_idx);
	}
	else
	{
		//insert the new item at the end of the hash list
		ipt->entries[tail_idx].next = new_idx;
	}

	//and finally, initialize the new entry.
	init_ipt_slot(ipt, new_idx, addr, -1, tail_idx);

	DEBUG3("Adding (%d:%d) at %d\n", VIRT_ADDR_PID(addr), VIRT_ADDR_PAGE(addr), new_idx);

//	dump_list(ipt);

	return ecSuccess;
}

errcode_t ipt_remove(ipt_t* ipt, virt_addr_t addr)
{
	int idx, prev_idx, next_idx;

	idx = get_vaddr_idx(ipt, addr);
	if (idx == IPT_INVALID)
	{
		return ecNotFound;
	}

	prev_idx = ipt->entries[idx].prev;
	next_idx = ipt->entries[idx].next;

	//we were at the head of the hashlist, need to update the HAT.
	if (prev_idx == IPT_INVALID)
	{
		ipt_set_hat(ipt, addr, next_idx);
	}

	//remove the item from the hashlist
	if (next_idx != IPT_INVALID)
	{
		ipt->entries[next_idx].prev = prev_idx;
	}
	if (prev_idx != IPT_INVALID)
	{
		ipt->entries[prev_idx].next = next_idx;
	}

	ipt->entries[idx].page_data.valid = FALSE;

	queue_push(ipt->free_pages, (void*)idx);

	DEBUG3("Removing (%d:%d) from %d\n", VIRT_ADDR_PID(addr), VIRT_ADDR_PAGE(addr), idx);

//	dump_list(ipt);

	return ecSuccess;
}

errcode_t ipt_for_each_entry(ipt_t* ipt, void (*func)(phys_addr_t, page_data_t*))
{
	int i;
	for (i=0; i<ipt->size; ++i)
	{
		if (ipt->entries[i].page_data.valid)
		{
			func(i, &ipt->entries[i].page_data);
		}
	}
	return ecSuccess;
}

errcode_t ipt_ref_count(ipt_t* ipt, int* refcount)
{
	REFCNT_READ_START(ipt);
	*refcount = ipt->ref_count;
	REFCNT_READ_END(ipt);
	return ecSuccess;
}

errcode_t ipt_zero_ref_count(ipt_t* ipt)
{
	REFCNT_WRITE_START(ipt);
	ipt->ref_count = 0;
	REFCNT_WRITE_END(ipt);
	return ecSuccess;
}

void ipt_destroy(ipt_t* ipt)
{
	int i;
	DEBUG1("Destroying IPT %p\n", ipt->entries);
	free(ipt->entries);
	ipt->entries = NULL;

	for (i=0; i < ipt->size; ++i)
	{
		rwlock_destroy(&ipt->hat[i].lock);
	}

	rwlock_destroy(&ipt->hat_lock);
	rwlock_destroy(&ipt->refcnt_lock);
	while (queue_size(ipt->free_pages) > 0)
	{
		queue_pop(ipt->free_pages);
	}
	queue_destroy(ipt->free_pages);

	free(ipt->hat);
}

#include "tests/ipt_tests.c"
