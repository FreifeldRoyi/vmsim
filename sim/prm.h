#ifndef PRM_H_
#define PRM_H_

#include "util/vmsim_types.h"
#include "mmu.h"

errcode_t prm_init(mmu_t* mmu);

errcode_t prm_pagefault(virt_addr_t addr);

void prm_destroy();


#endif /* PRM_H_ */
