#ifndef MMU_H_
#define MMU_H_

#include "util/queue.h"
#include "util/rwlock.h"
#include "util/map.h"
#include "ipt.h"
#include "mm.h"
#include "disk.h"

typedef struct
{
	mm_t* mem;
	disk_t* disk;

	ipt_t mem_ipt;
	map_t disk_map;

	pthread_mutex_t diskmap_lock;

	rwlock_t ipt_lock;
	pthread_mutex_t* page_frame_locks;
}mmu_t;

errcode_t mmu_init(mmu_t* mmu, mm_t* mem, disk_t* disk);

errcode_t mmu_alloc_multiple(mmu_t* mmu, virt_addr_t first_addr, int npages, int first_backing_page);
errcode_t mmu_free_multiple(mmu_t* mmu, virt_addr_t first_addr, int npages);
errcode_t mmu_alloc_page(mmu_t* mmu, virt_addr_t addr, int backing_page);
errcode_t mmu_free_page(mmu_t* mmu, virt_addr_t page);

errcode_t mmu_map_page(mmu_t* mmu, virt_addr_t addr);
errcode_t mmu_map_page_unlocked(mmu_t* mmu, virt_addr_t addr);
errcode_t mmu_unmap_page_unlocked(mmu_t* mmu, virt_addr_t page);

errcode_t mmu_read(mmu_t* mmu, virt_addr_t page, unsigned offset, unsigned nbytes, BYTE* buf);
errcode_t mmu_write(mmu_t* mmu, virt_addr_t page, unsigned offset, unsigned nbytes, BYTE* buf);

errcode_t mmu_for_each_mem_page(mmu_t* mmu, void (*func)(phys_addr_t, page_data_t*));

virt_addr_t mmu_phys_to_virt(mmu_t* mmu, phys_addr_t phys_addr);

errcode_t mmu_sync_to_backing_page_unlocked(mmu_t* mmu, virt_addr_t page);
errcode_t mmu_sync_from_backing_page_unlocked(mmu_t* mmu, virt_addr_t page);

void mmu_block_alloc_free(mmu_t* mmu);
void mmu_release_alloc_free(mmu_t* mmu);

errcode_t mmu_pin_page(mmu_t* mmu, virt_addr_t vaddr, phys_addr_t* mem_page);
errcode_t mmu_unpin_page(mmu_t* mmu, virt_addr_t vaddr);

void mmu_destroy(mmu_t* mmu);

#endif /* MMU_H_ */