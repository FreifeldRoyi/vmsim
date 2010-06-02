#ifndef IPT_H_
#define IPT_H_

#include "util/vmsim_types.h"
#include "util/rwlock.h"
#include "util/queue.h"

typedef enum {refNone, refRead, refWrite} ipt_ref_t;

typedef struct
{
	virt_addr_t addr;
	BOOL dirty;
	BOOL referenced;
	BOOL valid;

	int users;

	unsigned page_age;
}page_data_t;

typedef struct _ipt_entry_t{
	page_data_t page_data;

	int next;
	int prev;
}ipt_entry_t;

typedef struct {

	int ipt_idx;

	rwlock_t lock;

}hat_entry_t;

typedef struct{
	ipt_entry_t* entries;

	hat_entry_t* hat;
	rwlock_t hat_lock;

	int size;

	struct _queue_t* free_pages;

	int ref_count;
	rwlock_t refcnt_lock;

//	rwlock_t lock;
}ipt_t;

errcode_t ipt_init(ipt_t* ipt, int size);

BOOL ipt_has_translation(ipt_t* ipt, virt_addr_t addr);
BOOL ipt_is_dirty(ipt_t* ipt, virt_addr_t addr);

void ipt_lock_vaddr_read(ipt_t* ipt, virt_addr_t addr);
void ipt_lock_vaddr_write(ipt_t* ipt, virt_addr_t addr);
void ipt_unlock_vaddr_read(ipt_t* ipt, virt_addr_t addr);
void ipt_unlock_vaddr_write(ipt_t* ipt, virt_addr_t addr);

void ipt_lock_all_vaddr(ipt_t* ipt);
void ipt_unlock_all_vaddr(ipt_t* ipt);

errcode_t ipt_reference(ipt_t* ipt, virt_addr_t addr, ipt_ref_t reftype);
errcode_t ipt_translate(ipt_t* ipt, virt_addr_t addr, phys_addr_t* paddr);
errcode_t ipt_reverse_translate(ipt_t* ipt, phys_addr_t paddr, virt_addr_t* vaddr);

errcode_t ipt_add(ipt_t* ipt, virt_addr_t addr);
errcode_t ipt_remove(ipt_t* ipt, virt_addr_t addr);

errcode_t ipt_for_each_entry(ipt_t* ipt, void (*func)(phys_addr_t, page_data_t*));

errcode_t ipt_ref_count(ipt_t* ipt, int* refcount);
errcode_t ipt_zero_ref_count(ipt_t* ipt);

void ipt_destroy(ipt_t* ipt);

#endif /* IPT_H_ */
