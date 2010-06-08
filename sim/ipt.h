#ifndef IPT_H_
#define IPT_H_

/**Inverse page-table as described in the assignment.*/

#include "util/vmsim_types.h"
#include "util/locks.h"
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

}ipt_t;

/**Initialize an IPT.
 * @param ipt the IPT to initialize
 * @param size 	the number of entries in the IPT, which corresponds to the number
 * 				of physical memory pages.
 *
 * @return ecSuccess on success, some other code on failure.
 * */
errcode_t ipt_init(ipt_t* ipt, int size);

/**Check if an IPT has a translation for a given vaddr.
 * @param ipt the IPT to use
 * @param addr the address to check
 *
 * @return 	TRUE if the IPT contains a translation for the given vaddr,
 * 			FALSE otherwise.
 * */
BOOL ipt_has_translation(ipt_t* ipt, virt_addr_t addr);

/**Check if a given vaddr was referenced for write since it was added to the
 * IPT.
 * @param ipt the IPT to use
 * @param addr the address to check *
 *
 * @return TRUE if the addr is 'dirty', FALSE otherwise.
 * */
BOOL ipt_is_dirty(ipt_t* ipt, virt_addr_t addr);

/**Lock a vaddr. Only one thread may lock a given vaddr at a given point in time.
 * NOTE: This function may actually lock more then a single vaddr, and the number
 * and (possibly) other addresses that will be locked is implementation-dependent.
 *
 * @param ipt the IPT to use
 * @param addr the vaddr to lock
 *
 * */
void ipt_lock_vaddr(ipt_t* ipt, virt_addr_t addr);

/** Unlock a vaddr. Any other vaddrs that were locked along with it will be unlocked
 * as well.
 *
 * @param ipt the IPT to use
 * @param addr the vaddr to unlock
 *
 * */
void ipt_unlock_vaddr(ipt_t* ipt, virt_addr_t addr);

/**Prevent any modification to this IPT. Any ongoing modifications are allowed to
 * finish.
 * @param ipt the IPT to lock
 * */
void ipt_lock_all_vaddr(ipt_t* ipt);

/**Allow modification to this IPT.
 * @param ipt the IPT to unlock
 * */
void ipt_unlock_all_vaddr(ipt_t* ipt);

/**Mark a page as referenced for read/write
 * @param ipt the IPT to use
 * @param addr the addr to reference
 * @param reftype the reference type (read/write)
 *
 * @return ecSuccess on success, some other code on failure.
 * */
errcode_t ipt_reference(ipt_t* ipt, virt_addr_t addr, ipt_ref_t reftype);

/**Translate a virtual address to a physical address *
 * @param ipt the IPT to use
 * @param addr the vaddr to translate this vaddr must be in the IPT.
 * @param paddr a pointer to a paddr that recieves the result of the translation
 *
 * @return ecSuccess on success, some other code on failure.
 * */
errcode_t ipt_translate(ipt_t* ipt, virt_addr_t addr, phys_addr_t* paddr);

/**Add a vaddr to this IPT. call ipt_translate to find out what paddr was allocated for
 * this vaddr.
 * @param ipt the IPT to use
 * @param addr the vaddr to add.
 *
 * @return ecSuccess on success, some other code on failure.
 * */
errcode_t ipt_add(ipt_t* ipt, virt_addr_t addr);

/**Remove a vaddr from this IPT.
 * @param ipt the IPT to use
 * @param addr the vaddr to remove. addr is assumed to be in the IPT.
 * @return ecSuccess on success, some other code on failure.
 * */
errcode_t ipt_remove(ipt_t* ipt, virt_addr_t addr);

/**Perform an operation on every valid IPT entry.
 * @param ipt the IPT to use
 * @param func 	a function that will be called once for every vaddr in the IPT with
 * 				the matching paddr and page data for it.
 * @return ecSuccess on success, some other code on failure.
 * */
errcode_t ipt_for_each_entry(ipt_t* ipt, void (*func)(phys_addr_t, page_data_t*));

/**Return the number of references to any page in the IPT since the last call
 * to ipt_zero_ref_count (or since it's initialization).
 * @param ipt the IPT to use
 * @param refcount a pointer to an int that receives the refcount.
 * @return ecSuccess on success, some other code on failure.
 * */
errcode_t ipt_ref_count(ipt_t* ipt, int* refcount);

/**Zero the reference count of the IPT.
 * @param ipt the IPT to use
 * @return ecSuccess on success, some other code on failure.
 * */
errcode_t ipt_zero_ref_count(ipt_t* ipt);


/**Finalize an IPT.
 * @param ipt the IPT to finalize
 * */
void ipt_destroy(ipt_t* ipt);

#endif /* IPT_H_ */
