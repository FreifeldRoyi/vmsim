#ifndef IPT_H_
#define IPT_H_

#include "util/vmsim_types.h"

typedef enum {refRead, refWrite} ipt_ref_t;

typedef struct _ipt_entry_t{
	virt_addr_t addr;
	BOOL dirty;
	BOOL referenced;
	BOOL valid;

	void* extra_data;

	int next;
	int prev;
}ipt_entry_t;

typedef struct{
	ipt_entry_t *entries;
	int size;
}ipt_t;

errcode_t ipt_init(ipt_t* ipt, int size);

BOOL ipt_has_translation(ipt_t* ipt, virt_addr_t addr);
BOOL ipt_is_dirty(ipt_t* ipt, virt_addr_t addr);
BOOL ipt_is_referenced(ipt_t* ipt, virt_addr_t addr);

void ipt_get_extra_data(ipt_t* ipt, virt_addr_t addr);

errcode_t ipt_reference(ipt_t* ipt, virt_addr_t addr, ipt_ref_t reftype);
errcode_t ipt_translate(ipt_t* ipt, virt_addr_t addr, phys_addr_t* paddr);
errcode_t ipt_add(ipt_t* ipt, virt_addr_t addr);
errcode_t ipt_remove(ipt_t* ipt, virt_addr_t addr);

void ipt_destroy(ipt_t* ipt);

#endif /* IPT_H_ */
