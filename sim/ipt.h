#ifndef IPT_H_
#define IPT_H_

#include "util/vmsim_types.h"

typedef enum {refRead, refWrite} ipt_ref_t;

typedef struct
{
	virt_addr_t addr;
	BOOL dirty;
	BOOL referenced;
	BOOL valid;

	unsigned extra_data;
}page_data_t;

typedef struct _ipt_entry_t{
	page_data_t page_data;

	int next;
	int prev;
}ipt_entry_t;

typedef struct{
	ipt_entry_t *entries;
	int size;
	int num_valid_entries;
}ipt_t;

errcode_t ipt_init(ipt_t* ipt, int size);

BOOL ipt_has_translation(ipt_t* ipt, virt_addr_t addr);
BOOL ipt_is_dirty(ipt_t* ipt, virt_addr_t addr);
BOOL ipt_is_referenced(ipt_t* ipt, virt_addr_t addr);

errcode_t ipt_reference(ipt_t* ipt, virt_addr_t addr, ipt_ref_t reftype);
errcode_t ipt_translate(ipt_t* ipt, virt_addr_t addr, phys_addr_t* paddr);
errcode_t ipt_reverse_translate(ipt_t* ipt, phys_addr_t paddr, virt_addr_t* vaddr);

errcode_t ipt_add(ipt_t* ipt, virt_addr_t addr);
errcode_t ipt_remove(ipt_t* ipt, virt_addr_t addr);

errcode_t ipt_for_each_entry(ipt_t* ipt, void (*func)(phys_addr_t, page_data_t*));



void ipt_destroy(ipt_t* ipt);

#endif /* IPT_H_ */
