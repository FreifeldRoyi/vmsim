#include "ipt.h"

#include <stdlib.h>
#include <assert.h>
#include <stdio.h>

#define IPT_INVALID -1

static int ipt_hash(ipt_t* ipt, virt_addr_t addr)
{
	return (addr.pid * addr.page) % ipt->size;
}

errcode_t ipt_init(ipt_t* ipt, int size, mm_ops_t mm_ops)
{
	int i;

	ipt->entries = calloc(sizeof(ipt_entry_t), size);
	if (ipt->entries == NULL)
	{
		return ecFail;
	}
	ipt->size = size;
	ipt->mm_ops = mm_ops;

	for (i=0; i<size; ++i)
	{
		ipt->entries[i].next =
			ipt->entries[i].prev = IPT_INVALID;
	}

	return ecSuccess;
}

static int get_vaddr_idx(ipt_t* ipt,virt_addr_t addr)
{
	int idx = ipt_hash(ipt, addr);
	if (!ipt->entries[idx].valid)
		return IPT_INVALID; //empty hash list
	while ( (!VIRT_ADDR_EQ(ipt->entries[idx].addr, addr))&&
			(idx != IPT_INVALID)
			)
	{
		assert(ipt->entries[idx].valid);//invalid entry inside a hash list.
		idx = ipt->entries[idx].next;
	}
	return idx;
}

static void init_ipt_slot(ipt_t* ipt, int idx,virt_addr_t addr, int next, int prev)
{
	ipt->entries[idx].addr = addr;
	ipt->entries[idx].dirty = FALSE;
	ipt->entries[idx].referenced = FALSE;
	ipt->entries[idx].next = next;
	ipt->entries[idx].prev = prev;
	ipt->entries[idx].valid = TRUE;
}

static void dump_list(ipt_t* ipt)
{
	int i;

	printf("IPT Dump:\nidx|prev|next|valid\n");
	for (i=0; i<ipt->size; ++i)
	{
		printf("%3d|%4d|%4d|%s\n", 	i,
									ipt->entries[i].prev,
									ipt->entries[i].next,
									ipt->entries[i].valid?"Yes":"No");
	}
}

BOOL ipt_has_translation(ipt_t* ipt, virt_addr_t addr)
{
	int idx = get_vaddr_idx(ipt, addr);

	if (idx == IPT_INVALID)
		return FALSE;

	if (!ipt->entries[idx].valid)
		return FALSE;

	if (!VIRT_ADDR_EQ(ipt->entries[idx].addr, addr))
		return FALSE;

	return TRUE;
}

BOOL ipt_is_dirty(ipt_t* ipt, virt_addr_t addr)
{
	assert (ipt_has_translation(ipt, addr));
	return ipt->entries[get_vaddr_idx(ipt, addr)].dirty;
}

BOOL ipt_is_referenced(ipt_t* ipt, virt_addr_t addr)
{
	assert (ipt_has_translation(ipt, addr));
	return ipt->entries[get_vaddr_idx(ipt, addr)].referenced;
}

errcode_t ipt_reference(ipt_t* ipt, virt_addr_t addr, ipt_ref_t reftype)
{
	assert (ipt_has_translation(ipt, addr));

	ipt->entries[get_vaddr_idx(ipt, addr)].referenced = TRUE;
	if (reftype == refWrite)
	{
		ipt->entries[get_vaddr_idx(ipt, addr)].dirty = TRUE;
	}

	return ecSuccess;
}

errcode_t ipt_translate(ipt_t* ipt, virt_addr_t addr, phys_addr_t* paddr)
{
	if (!ipt_has_translation(ipt, addr))
	{
		ipt->mm_ops.page_fault(addr);
	}

	assert(ipt_has_translation(ipt, addr)); //if the page fault handler didn't load the right page, something
											//is very wrong...

	*paddr = get_vaddr_idx(ipt, addr);
	return ecSuccess;
}

static errcode_t do_add(ipt_t* ipt, virt_addr_t addr)
{
	int idx = ipt_hash(ipt, addr);
	int prev_idx, next_idx;

	//if we don't to walk the hash list, just initialize to proper slot and return.
	if (!ipt->entries[idx].valid)
	{
		init_ipt_slot(ipt, idx, addr, IPT_INVALID, IPT_INVALID);
		return ecSuccess;
	}

	idx = 0;
	//find the next available slot
	while ((ipt->entries[idx].valid) && (idx < ipt->size))
	{
		++idx;
	}

	if (ipt->entries[idx].valid)
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
	errcode_t errcode = do_add(ipt, addr);
	if (errcode == ecSuccess)
	{
		return ecSuccess;
	}

	ipt->mm_ops.out_of_mem();
	assert(do_add(ipt, addr) == ecSuccess); //if we don't have any free slots after calling the OOM handler,
											//something is very wrong.
	return ecSuccess;
}

errcode_t ipt_remove(ipt_t* ipt, virt_addr_t addr)
{
	int idx, next_idx, prev_idx;
	assert(ipt_has_translation(ipt, addr));

	idx = get_vaddr_idx(ipt, addr);
	prev_idx = ipt->entries[idx].prev;
	next_idx = ipt->entries[idx].next;

	if ( prev_idx != IPT_INVALID)
	{
		ipt->entries[prev_idx].next = next_idx;
	}
	if ( next_idx != IPT_INVALID)
	{
		ipt->entries[next_idx].prev = prev_idx;
	}

	ipt->entries[idx].valid = FALSE;

	return ecSuccess;
}

void ipt_destroy(ipt_t* ipt)
{
	free(ipt->entries);
	ipt->entries = NULL;
}

#include "tests/ipt_tests.c"
