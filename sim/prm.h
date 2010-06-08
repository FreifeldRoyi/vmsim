#ifndef PRM_H_
#define PRM_H_

/**The page replacement manager as described in the assignment*/

#include "util/vmsim_types.h"
#include "mmu.h"

/**Initialize the PRM and start running it's thread.
 *
 * @param mmu the mmu that the PRM uses.
 * @return ecSuccess on success, some other code on failure.
 * */
errcode_t prm_init(mmu_t* mmu);

/**Request the PRM to load the given vaddr from the disk to the main memory
 *
 * @param addr the vaddr to load from disk
 * @return ecSuccess on success, some other code on failure.
 * */
errcode_t prm_pagefault(virt_addr_t addr);

/**Stop the PRM's thread and destroy the PRM itself.
 * */
void prm_destroy();


#endif /* PRM_H_ */
