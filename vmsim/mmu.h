#ifndef MMU_H_
#define MMU_H_

/**A simulated MMU*/

#include "util/queue.h"
#include "util/locks.h"
#include "util/map.h"
#include "ipt.h"
#include "mm.h"
#include "disk.h"

typedef struct
{
	int nrefs; //total number of references
	int hits;  //number of hit references
}mmu_stats_t;

typedef struct
{
	mm_t* mem;
	disk_t* disk;

	ipt_t mem_ipt;
	map_t disk_map;

	unsigned aging_freq;

	mmu_stats_t stats;
	rwlock_t stats_lock;

}mmu_t;

/**Initialize an MMU
 * @param mmu the MMU to initialize
 * @param disk the disk attached to this MMU
 * @param mem the main memory attached to this MMU
 * @param 	aging_freq the number of references between calls to the aging daemon.
 * 			lower this parameter for more accurate swapping decisions, raise this
 * 			parameter for better performance.
 * @return ecSuccess on success, some other code on failure.
 * */
errcode_t mmu_init(mmu_t* mmu, mm_t* mem, disk_t* disk, int aging_freq);

/**Allocate mapping for multiple sequential vaddrs.
 * @param mmu the MMU to use
 * @param first_addr the first vaddr to map. The other vaddrs to map are
 * 					 assumed to have the same PID with sequential page numbers.
 * @param npages the number of vaddr to map
 * @param first_backing_page 	the backing page for the first vaddr. The backing
 * 								pages for the other vaddrs are assumed to be sequential
 * 								on the disk starting from this one.
 * @return ecSuccess on success, some other code on failure.
 * */
errcode_t mmu_alloc_multiple(mmu_t* mmu, virt_addr_t first_addr, int npages, int first_backing_page);

/**Free the mapping of multiple pages.
 * @param mmu the MMU to use
 * @param first_addr the first vaddr to free. The other vaddrs to free are
 * 					 assumed to have the same PID with sequential page numbers.
 * @param npages the number of vaddr to free
 * @return ecSuccess on success, some other code on failure.
 * */
errcode_t mmu_free_multiple(mmu_t* mmu, virt_addr_t first_addr, int npages);

/**Map an allocated page into memory. This function is unlocked and must be called
 * from a context that previously locked the vaddr, such as an mmu_read/mmu_write call.
 *
 * @param mmu the MMU to use
 * @param addr the vaddr to map
 * */
errcode_t mmu_map_page(mmu_t* mmu, virt_addr_t addr);

/**Unmap an allocated page from memory, keeping it's backing page on disk. This function
 * is unlocked and must be called from a context that previously locked the vaddr, such
 * as an mmu_read/mmu_write call.
 *
 * @param mmu the MMU to use
 * @param addr the vaddr to map
 * */
errcode_t mmu_unmap_page(mmu_t* mmu, virt_addr_t page);

/**Read from an allocated (but not necessarily mapped) page. The page will be
 * mapped(swapped-in) if needed.
 *
 * @param mmu the MMU to use
 * @param page the address to read from
 * @param nbytes the number of bytes to read
 * @param buf a buffer into which the read data is copied.
 *
 * @return ecSuccess on success, some other code on failure.
 * */
errcode_t mmu_read(mmu_t* mmu, virt_addr_t addr, unsigned nbytes, BYTE* buf);

/**Write to an allocated (but not necessarily mapped) page. The page will be
 * mapped(swapped-in) if needed.
 *
 * @param mmu the MMU to use
 * @param page the address to write to
 * @param nbytes the number of bytes to write
 * @param buf a buffer from which the data is copied to the page.
 *
 * @return ecSuccess on success, some other code on failure.
 * */
errcode_t mmu_write(mmu_t* mmu, virt_addr_t addr, unsigned nbytes, BYTE* buf);

/**Perform an operation on every mapped page.
 * @param mmu the MMU to use
 * @param func 	a function that will be called once for every mapped vaddr with
 * 				the matching paddr and page data for it.
 * @return ecSuccess on success, some other code on failure.
 * */
errcode_t mmu_for_each_mem_page(mmu_t* mmu, void (*func)(phys_addr_t, page_data_t*));

/**Make sure that the memory page and it's backing page are identical, writing the page
 * to the backing page if needed.This function is unlocked and must be called
 * from a context that previously locked the vaddr and also blocked allocation/deallocation,
 * such as an mmu_read/mmu_write call.
 *
 * @param mmu the MMU to use
 * @param page the page to sync
 * @return ecSuccess on success, some other code on failure.
 * */
errcode_t mmu_sync_to_backing_page(mmu_t* mmu, virt_addr_t page);

/**Make sure that the memory page and it's backing page are identical, writing the page
 * to the memory page if needed.This function is unlocked and must be called
 * from a context that previously locked the vaddr and also blocked allocation/deallocation,
 * such as an mmu_read/mmu_write call.
 *
 * @param mmu the MMU to use
 * @param page the page to sync
 * @return ecSuccess on success, some other code on failure.
 * */
errcode_t mmu_sync_from_backing_page(mmu_t* mmu, virt_addr_t page);

/**Return a copy of the MMU statistics at a given point in time.
 * @param mmu the MMU to use
 * @return a copy of the current MMU stats
 * */
mmu_stats_t mmu_get_stats(mmu_t* mmu);

/**Finalize an MMU.
 * This function does not finalize the disk/mm attached to the MMU.
 * @param mmu the MMU to finalize
 * */
void mmu_destroy(mmu_t* mmu);

#endif /* MMU_H_ */
