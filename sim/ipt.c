#include "ipt.h"
#include "util/logger.h"

#include <stdlib.h>
#include <assert.h>
#include <stdio.h>

#define READ_START(_ipt) DEBUG("Acquiring R\n");rwlock_acquire_read(&_ipt->lock)
#define READ_END(_ipt) rwlock_release_read(&_ipt->lock);DEBUG("Released R\n")

#define WRITE_START(_ipt) DEBUG("Acquiring W\n");rwlock_acquire_write(&_ipt->lock)
#define WRITE_END(_ipt) rwlock_release_write(&_ipt->lock);DEBUG("Released W\n")

#define IPT_INVALID -1

static int ipt_hash(ipt_t* ipt, virt_addr_t addr)
{
	return (VIRT_ADDR_PID(addr) * VIRT_ADDR_PAGE(addr)) % ipt->size;
}

errcode_t ipt_init(ipt_t* ipt, int size)
{
	int i;

	ipt->entries = calloc(sizeof(ipt_entry_t), size);
	if (ipt->entries == NULL)
	{
		return ecFail;
	}
	ipt->size = size;
	ipt->num_valid_entries = 0;
	if (rwlock_init(&ipt->lock) != ecSuccess)
	{
		return ecFail;
	}

	for (i=0; i<size; ++i)
	{
		ipt->entries[i].next =
			ipt->entries[i].prev = IPT_INVALID;
	}

	DEBUG1("Created IPT %p\n", ipt->entries);
	return ecSuccess;
}

static int get_vaddr_idx(ipt_t* ipt,virt_addr_t addr)
{
	int idx = ipt_hash(ipt, addr);
	if (ipt->entries[idx].page_data.valid)
	{
		while ( (idx != IPT_INVALID) &&
				(!VIRT_ADDR_EQ(ipt->entries[idx].page_data.addr, addr))
				)
		{
			assert(ipt->entries[idx].page_data.valid);//invalid entry inside a hash list.
			idx = ipt->entries[idx].next;
		}

		if (idx != -1)
		{
			DEBUG1("\nidx %d\n", idx);
			return idx;
		}
	}
	/*We didn't find addr in its hash list.
	 * But maybe it was inserted to the wrong place in the table because the appropriate
	 *   place for it was taken by some addr with the wrong hash value.
	 *   so we'll try a linear search now.
	 * */

	for (idx = 0; idx < ipt->size; ++idx)
	{
		if ((VIRT_ADDR_EQ(ipt->entries[idx].page_data.addr, addr))&&
				(ipt->entries[idx].page_data.valid))
		{
			DEBUG1("\nlinear idx %d\n", idx);
			return idx;
		}
	}

	return -1;
}

static void init_ipt_slot(ipt_t* ipt, int idx,virt_addr_t addr, int next, int prev)
{
	ipt->entries[idx].page_data.addr = addr;
	ipt->entries[idx].page_data.dirty = FALSE;
	ipt->entries[idx].page_data.referenced = FALSE;
	ipt->entries[idx].next = next;
	ipt->entries[idx].prev = prev;
	ipt->entries[idx].page_data.valid = TRUE;
}

static void dump_list(ipt_t* ipt)
{
	int i;

	printf("IPT Dump:(%d valid entries)\nidx|vaddr|prev|next|valid\n", ipt->num_valid_entries);
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
	int idx;

	idx= get_vaddr_idx(ipt, addr);

	if (idx == IPT_INVALID)
	{
		DEBUG("\nget_vaddr_idx returned IPT_INVALID\n");
		return FALSE;
	}

	if (!ipt->entries[idx].page_data.valid)
	{
		DEBUG("\nget_vaddr_idx returned an invalid entry\n");
		return FALSE;
	}

	if (!VIRT_ADDR_EQ(ipt->entries[idx].page_data.addr, addr))
	{
		DEBUG("\nget_vaddr_idx returned an entry with the wrong vaddr\n");
		return FALSE;
	}

	return TRUE;
}

BOOL ipt_has_translation(ipt_t* ipt, virt_addr_t addr)
{
	BOOL ret;
	READ_START(ipt);

	ret = ipt_has_translation_unlocked(ipt, addr);

	READ_END(ipt);
	return ret;
}

BOOL ipt_is_dirty(ipt_t* ipt, virt_addr_t addr)
{
	BOOL dirty;
	READ_START(ipt);
	assert (ipt_has_translation_unlocked(ipt, addr));
	dirty = ipt->entries[get_vaddr_idx(ipt, addr)].page_data.dirty;
	READ_END(ipt);
	return dirty;
}

BOOL ipt_is_referenced(ipt_t* ipt, virt_addr_t addr)
{
	BOOL ref;
	READ_START(ipt);
	assert (ipt_has_translation_unlocked(ipt, addr));
	ref = ipt->entries[get_vaddr_idx(ipt, addr)].page_data.referenced;
	READ_END(ipt);
	return ref;
}

errcode_t ipt_reference(ipt_t* ipt, virt_addr_t addr, ipt_ref_t reftype)
{
	WRITE_START(ipt);
	assert (ipt_has_translation_unlocked(ipt, addr));

	ipt->entries[get_vaddr_idx(ipt, addr)].page_data.referenced = TRUE;
	if (reftype == refWrite)
	{
		ipt->entries[get_vaddr_idx(ipt, addr)].page_data.dirty = TRUE;
	}
	WRITE_END(ipt);

	return ecSuccess;
}

errcode_t ipt_translate(ipt_t* ipt, virt_addr_t addr, phys_addr_t* paddr)
{
	READ_START(ipt);
	if (!ipt_has_translation_unlocked(ipt, addr))
	{
		READ_END(ipt);
		return ecFail;
	}

	*paddr = get_vaddr_idx(ipt, addr);
	READ_END(ipt);
	return ecSuccess;
}

errcode_t ipt_reverse_translate(ipt_t* ipt, phys_addr_t paddr, virt_addr_t* vaddr)
{
	READ_START(ipt);
	*vaddr = ipt->entries[paddr].page_data.addr;

	READ_END(ipt);
	return ecSuccess;
}

static errcode_t do_add(ipt_t* ipt, virt_addr_t addr)
{
	int idx = ipt_hash(ipt, addr);
	int prev_idx, next_idx;

	//if we don't to walk the hash list, just initialize to proper slot and return.
	if (!ipt->entries[idx].page_data.valid)
	{
		init_ipt_slot(ipt, idx, addr, IPT_INVALID, IPT_INVALID);
		return ecSuccess;
	}

	idx = 0;
	//find the next available slot
	while ( (idx < ipt->size) && (ipt->entries[idx].page_data.valid))
	{
		++idx;
	}

	if (idx == ipt->size)
	{
		return ecFail;
	}

	//we'll add the new mapping to the head of the list(one item after the first, since we can't move the first).
	//this has 3 reasons:
	//1. this is a newly-loaded page and will therefore probably be accessed really soon.
	//2. this will make the ipt_add operation independent of hash list length
	//3. i'm lazy.

	prev_idx = ipt_hash(ipt, addr); //first item in the list
	next_idx = ipt->entries[prev_idx].next; //second item in the list

	init_ipt_slot(ipt, idx, addr, next_idx, prev_idx);
	ipt->entries[prev_idx].next = idx;
	if (next_idx != IPT_INVALID)
	{
		ipt->entries[next_idx].prev = idx;
	}

//	dump_list(ipt);
	return ecSuccess;
}

errcode_t ipt_add(ipt_t* ipt, virt_addr_t addr)
{
	errcode_t errcode;
	WRITE_START(ipt);
	assert(!ipt_has_translation_unlocked(ipt, addr));
	DEBUG3("ipt_add: %p adding %d:%d\n",ipt, VIRT_ADDR_PID(addr), VIRT_ADDR_PAGE(addr));

	errcode = do_add(ipt, addr);
	if (errcode == ecSuccess)
	{
		++ipt->num_valid_entries;
	}

	WRITE_END(ipt);
	return errcode;
}

errcode_t ipt_remove(ipt_t* ipt, virt_addr_t addr)
{
	int idx, next_idx, prev_idx;
	WRITE_START(ipt);
	assert(ipt_has_translation_unlocked(ipt, addr));

	DEBUG3("ipt_remove: %p removing %d:%d\n",ipt, VIRT_ADDR_PID(addr), VIRT_ADDR_PAGE(addr));

//	dump_list(ipt);

	idx = get_vaddr_idx(ipt, addr);
	prev_idx = ipt->entries[idx].prev;
	next_idx = ipt->entries[idx].next;

	if (prev_idx == IPT_INVALID)
	{

		if (next_idx != IPT_INVALID)
		{
			/*this is the first element in a hashlist.
			 * since we want subsequent searches to find the right hashlist, we'll move
			 * the second element to this index, since this is the expected index for
			 * this vaddr.
			 */
			ipt->entries[idx] = ipt->entries[next_idx];
			ipt->entries[idx].prev = IPT_INVALID;
			ipt->entries[next_idx].page_data.valid = FALSE;

			if (ipt->entries[idx].next != IPT_INVALID)
			{
				ipt->entries[ipt->entries[idx].next].prev = idx;
			}
		}
		else
		{
			//just delete this element
			ipt->entries[idx].page_data.valid = FALSE;
		}
	}
	else
	{
		ipt->entries[prev_idx].next = next_idx;
		if ( next_idx != IPT_INVALID)
		{
			ipt->entries[next_idx].prev = prev_idx;
		}
		ipt->entries[idx].page_data.valid = FALSE;
	}

	--ipt->num_valid_entries;
//	dump_list(ipt);

	WRITE_END(ipt);
	return ecSuccess;
}

errcode_t ipt_for_each_entry(ipt_t* ipt, void (*func)(phys_addr_t, page_data_t*))
{
	int i;
	READ_START(ipt);
	for (i=0; i<ipt->size; ++i)
	{
		if (ipt->entries[i].page_data.valid)
		{
			func(i, &ipt->entries[i].page_data);
		}
	}
	READ_END(ipt);
	return ecSuccess;
}

void ipt_destroy(ipt_t* ipt)
{
	WRITE_START(ipt);
	DEBUG1("Destroying IPT %p\n", ipt->entries);
	free(ipt->entries);
	ipt->entries = NULL;
	WRITE_END(ipt);

	rwlock_destroy(&ipt->lock);
}

#include "tests/ipt_tests.c"
