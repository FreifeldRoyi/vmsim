#include "cunit/cunit.h"
#include "util/worker_thread.h"
#include <string.h>
#include <unistd.h>


#define MEM_NPAGES 11
#define DISK_NPAGES 109
#define DISK_BLOCKSIZE 10
#define PAGESIZE 32

static cunit_err_t
write_pattern(mmu_t* mmu, virt_addr_t addr, unsigned offset, unsigned nbytes, BYTE* buf)
{
	unsigned i;

	for (i=0; i < nbytes; ++i)
	{
		buf[i] = i&0xFF;
	}

	return ceSuccess;
}

static cunit_err_t
validate_pattern(mmu_t* mmu, virt_addr_t addr, unsigned offset, unsigned nbytes, BYTE* buf)
{
	unsigned i;

	for (i=0; i < nbytes; ++i)
	{
		ASSERT_EQUALS(i&0xFF, buf[i]);
	}

	return ceSuccess;
}

static cunit_err_t
validated_write(mmu_t* mmu, virt_addr_t addr, unsigned offset, unsigned nbytes)
{
	BYTE buf[PAGESIZE]; //just to avoid dynamic allocation

	ASSERT_EQUALS(ceSuccess, write_pattern(mmu, addr, offset, nbytes, buf));
	ASSERT_EQUALS(ecSuccess, mmu_write(mmu, addr, offset, nbytes, buf));
	memset(buf, 0, ARRSIZE(buf));
	ASSERT_EQUALS(ecSuccess, mmu_read(mmu, addr, offset, nbytes, buf));
	ASSERT_EQUALS(ceSuccess, validate_pattern(mmu,addr,offset,nbytes,buf));

	return ceSuccess;
}

static cunit_err_t
alloc_junk_pages(mmu_t* mmu,int npages)
{
	int i;
	virt_addr_t addr;

	for (i=0; i< npages ; ++i)
	{
		VIRT_ADDR_PAGE(addr) = i % DISK_BLOCKSIZE;
		VIRT_ADDR_PID(addr) = i / DISK_BLOCKSIZE;
		ASSERT_EQUALS(ecSuccess, mmu_alloc_page(mmu, addr, 0));
	}
	return ecSuccess;
}

static cunit_err_t
free_junk_pages(mmu_t* mmu,int npages)
{
	int i;
	virt_addr_t addr;

	for (i=0; i< npages ; ++i)
	{
		VIRT_ADDR_PAGE(addr) = i % DISK_BLOCKSIZE;
		VIRT_ADDR_PID(addr) = i / DISK_BLOCKSIZE;
		ASSERT_EQUALS(ecSuccess, mmu_free_page(mmu, addr));
	}
	return ecSuccess;
}

cunit_err_t test_mmu_alloc_free_sanity()
{
	mmu_t mmu;
	mm_t mem;
	disk_t disk;
	virt_addr_t addr = {0,0};
	int i;

	ASSERT_EQUALS(ecSuccess, mm_init(&mem, MEM_NPAGES, PAGESIZE));
	ASSERT_EQUALS(ecSuccess, disk_init(&disk,DISK_NPAGES, PAGESIZE, DISK_BLOCKSIZE));

	ASSERT_EQUALS(ecSuccess, mmu_init(&mmu, &mem, &disk));

	for (i=0; i< MEM_NPAGES * 5; ++i)
	{
		ASSERT_EQUALS(ecSuccess, mmu_alloc_page(&mmu, addr, 0));
		ASSERT_EQUALS(ecSuccess, mmu_free_page(&mmu, addr));
	}

	mmu_destroy(&mmu);
	mm_destroy(&mem);
	disk_destroy(&disk);

	return ceSuccess;
}

cunit_err_t test_mmu_alloc_free_multiple()
{
	mmu_t mmu;
	mm_t mem;
	disk_t disk;
	virt_addr_t addr = {0,0};
	int i;

	ASSERT_EQUALS(ecSuccess, mm_init(&mem, MEM_NPAGES, PAGESIZE));
	ASSERT_EQUALS(ecSuccess, disk_init(&disk,DISK_NPAGES, PAGESIZE, DISK_BLOCKSIZE));

	ASSERT_EQUALS(ecSuccess, mmu_init(&mmu, &mem, &disk));

	for (i=0; i< 5; ++i)
	{
		ASSERT_EQUALS(ecSuccess, mmu_alloc_multiple(&mmu, addr,MEM_NPAGES, 0));
		ASSERT_EQUALS(ecSuccess, mmu_free_multiple(&mmu, addr, MEM_NPAGES));
	}

	mmu_destroy(&mmu);
	mm_destroy(&mem);
	disk_destroy(&disk);

	return ceSuccess;
}

cunit_err_t test_mmu_alloc_read_free_multiple()
{
	mmu_t mmu;
	mm_t mem;
	disk_t disk;
	virt_addr_t addr = {0,0};
	int i,j;
	BYTE buf[PAGESIZE];

	ASSERT_EQUALS(ecSuccess, mm_init(&mem, MEM_NPAGES, PAGESIZE));
	ASSERT_EQUALS(ecSuccess, disk_init(&disk,DISK_NPAGES, PAGESIZE, DISK_BLOCKSIZE));

	ASSERT_EQUALS(ecSuccess, mmu_init(&mmu, &mem, &disk));

	for (i=0; i< 5; ++i)
	{
		ASSERT_EQUALS(ecSuccess, mmu_alloc_multiple(&mmu, addr,MEM_NPAGES, 0));
		for (j=0; j<MEM_NPAGES; ++j)
		{
			VIRT_ADDR_PAGE(addr) = j;
			mmu_read(&mmu, addr, 0, PAGESIZE, buf);
		}
		VIRT_ADDR_PAGE(addr) = 0;
		ASSERT_EQUALS(ecSuccess, mmu_free_multiple(&mmu, addr, MEM_NPAGES));
	}

	mmu_destroy(&mmu);
	mm_destroy(&mem);
	disk_destroy(&disk);

	return ceSuccess;
}

cunit_err_t test_mmu_alloc_free_oom()
{
	mmu_t mmu;
	mm_t mem;
	disk_t disk;

	ASSERT_EQUALS(ecSuccess, mm_init(&mem, MEM_NPAGES, PAGESIZE));
	ASSERT_EQUALS(ecSuccess, disk_init(&disk,DISK_NPAGES, PAGESIZE, DISK_BLOCKSIZE));

	ASSERT_EQUALS(ecSuccess, mmu_init(&mmu, &mem, &disk));

	ASSERT_EQUALS(ceSuccess, alloc_junk_pages(&mmu, DISK_NPAGES));
	ASSERT_EQUALS(ceSuccess, free_junk_pages(&mmu, DISK_NPAGES));

	mmu_destroy(&mmu);
	mm_destroy(&mem);
	disk_destroy(&disk);

	return ceSuccess;
}

#define NPROCS (DISK_NPAGES/DISK_BLOCKSIZE)

typedef struct
{
	mmu_t* mmu;
	procid_t pid;
	int disk_page;
	int alloc_size;
	int nallocs;
}alloc_free_thread_params_t;

static BOOL
alloc_free_func(void* arg)
{
	alloc_free_thread_params_t* params = (alloc_free_thread_params_t*)arg;
	virt_addr_t addr;
	VIRT_ADDR_PID(addr)=params->pid;
	VIRT_ADDR_PAGE(addr)=0;

	assert(mmu_alloc_multiple(params->mmu, addr, params->alloc_size,params->disk_page) == ecSuccess);
	assert(validated_write(params->mmu, addr, 0, PAGESIZE) == ceSuccess);
	assert(mmu_free_multiple(params->mmu, addr, params->alloc_size) == ecSuccess);
	++params->nallocs;

	return FALSE;
}

static cunit_err_t
do_test_mmu_alloc_free_stress(int alloc_size)
{
	mmu_t mmu;
	mm_t mem;
	disk_t disk;
	worker_thread_t threads[NPROCS];
	alloc_free_thread_params_t params[NPROCS];
	int i;

	ASSERT_EQUALS(ecSuccess, mm_init(&mem, MEM_NPAGES, PAGESIZE));
	ASSERT_EQUALS(ecSuccess, disk_init(&disk,DISK_NPAGES, PAGESIZE, DISK_BLOCKSIZE));

	ASSERT_EQUALS(ecSuccess, prm_init(&mmu));

	ASSERT_EQUALS(ecSuccess, mmu_init(&mmu, &mem, &disk));

	for (i=0; i<NPROCS; ++i)
	{
		worker_thread_create(&threads[i],alloc_free_func);
		params[i].pid = i;
		params[i].mmu = &mmu;
		params[i].nallocs = 0;
		params[i].alloc_size = alloc_size;
		disk_alloc_process_block(&disk, &params[i].disk_page);
	}

	for (i=0; i<NPROCS; ++i)
	{
		worker_thread_start(&threads[i],&params[i]);
	}

	sleep(5);

	for (i=0; i<NPROCS; ++i)
	{
		worker_thread_stop(&threads[i]);
		ASSERT_TRUE(params[i].nallocs > 0);
		worker_thread_destroy(&threads[i]);
	}

	prm_destroy();
	mmu_destroy(&mmu);
	disk_destroy(&disk);
	mm_destroy(&mem);

	return ceSuccess;
}

cunit_err_t test_mmu_alloc_free_stress()
{
	return do_test_mmu_alloc_free_stress(1);
}

cunit_err_t test_mmu_alloc_free_pagefault_stress()
{
	return do_test_mmu_alloc_free_stress(DISK_BLOCKSIZE);
}


cunit_err_t test_mmu_read_write_sanity()
{
	mmu_t mmu;
	mm_t mem;
	disk_t disk;
	virt_addr_t addr = {0,0};

	ASSERT_EQUALS(ecSuccess, mm_init(&mem, MEM_NPAGES, PAGESIZE));
	ASSERT_EQUALS(ecSuccess, disk_init(&disk,DISK_NPAGES, PAGESIZE, DISK_BLOCKSIZE));

	ASSERT_EQUALS(ecSuccess, mmu_init(&mmu, &mem, &disk));

	ASSERT_EQUALS(ecSuccess, mmu_alloc_page(&mmu, addr, 0));

	ASSERT_EQUALS(ceSuccess, validated_write(&mmu, addr, 0,PAGESIZE));

	mm_destroy(&mem);
	disk_destroy(&disk);
	mmu_destroy(&mmu);

	return ceSuccess;
}

cunit_err_t test_mmu_read_write_pagefault()
{
	mmu_t mmu;
	mm_t mem;
	disk_t disk;
	virt_addr_t addr = {0,0};

	ASSERT_EQUALS(ecSuccess, mm_init(&mem, MEM_NPAGES, PAGESIZE));
	ASSERT_EQUALS(ecSuccess, disk_init(&disk,DISK_NPAGES, PAGESIZE, DISK_BLOCKSIZE));

	ASSERT_EQUALS(ecSuccess, mmu_init(&mmu, &mem, &disk));

	ASSERT_EQUALS(ecSuccess, prm_init(&mmu));

	//allocate enough pages to make sure the next page will be allocated on disk.
	ASSERT_EQUALS(ceSuccess, alloc_junk_pages(&mmu, MEM_NPAGES));

	//now allocate a page on the disk

	VIRT_ADDR_PAGE(addr) = 0;
	VIRT_ADDR_PID(addr) = 9999;
	ASSERT_EQUALS(ecSuccess, mmu_alloc_page(&mmu, addr, 0));

	ASSERT_EQUALS(ceSuccess, validated_write(&mmu, addr, 0,PAGESIZE));

	ASSERT_EQUALS(ceSuccess, free_junk_pages(&mmu, MEM_NPAGES));
	ASSERT_EQUALS(ecSuccess, mmu_free_page(&mmu, addr));

	mm_destroy(&mem);
	disk_destroy(&disk);
	mmu_destroy(&mmu);
	prm_destroy();

	return ceSuccess;
}

cunit_err_t test_mmu_sync_to_backing_page()
{
	mmu_t mmu;
	mm_t mem;
	disk_t disk;
	virt_addr_t addr = {0,0};
	int disk_page;
	BYTE buf[PAGESIZE];

	ASSERT_EQUALS(ecSuccess, mm_init(&mem, MEM_NPAGES, PAGESIZE));
	ASSERT_EQUALS(ecSuccess, disk_init(&disk,DISK_NPAGES, PAGESIZE, DISK_BLOCKSIZE));

	ASSERT_EQUALS(ecSuccess, disk_alloc_process_block(&disk, &disk_page));

	ASSERT_EQUALS(ecSuccess, mmu_init(&mmu, &mem, &disk));

	VIRT_ADDR_PAGE(addr) = 0;
	VIRT_ADDR_PID(addr) = 9999;
	ASSERT_EQUALS(ecSuccess, mmu_alloc_page(&mmu, addr, disk_page));

	ASSERT_EQUALS(ceSuccess, write_pattern(&mmu, addr, 0,PAGESIZE, buf));
	ASSERT_EQUALS(ecSuccess, mmu_write(&mmu, addr, 0, PAGESIZE, buf));

	ASSERT_EQUALS(ecSuccess, mmu_sync_to_backing_page(&mmu, addr));

	memset(buf, 0, ARRSIZE(buf));
	ASSERT_EQUALS(ecSuccess, disk_get_page(&disk, disk_page, buf));
	ASSERT_EQUALS(ceSuccess, validate_pattern(&mmu, addr, 0, PAGESIZE, buf));


	ASSERT_EQUALS(ecSuccess, mmu_free_page(&mmu, addr));

	mm_destroy(&mem);
	disk_destroy(&disk);
	mmu_destroy(&mmu);

	return ceSuccess;
}

cunit_err_t test_mmu_sync_from_backing_page()
{
	mmu_t mmu;
	mm_t mem;
	disk_t disk;
	virt_addr_t addr = {0,0};
	int disk_page;
	BYTE buf[PAGESIZE];

	ASSERT_EQUALS(ecSuccess, mm_init(&mem, MEM_NPAGES, PAGESIZE));
	ASSERT_EQUALS(ecSuccess, disk_init(&disk,DISK_NPAGES, PAGESIZE, DISK_BLOCKSIZE));

	ASSERT_EQUALS(ecSuccess, disk_alloc_process_block(&disk, &disk_page));

	ASSERT_EQUALS(ecSuccess, mmu_init(&mmu, &mem, &disk));

	VIRT_ADDR_PAGE(addr) = 0;
	VIRT_ADDR_PID(addr) = 9999;
	ASSERT_EQUALS(ecSuccess, mmu_alloc_page(&mmu, addr, disk_page));

	ASSERT_EQUALS(ceSuccess, write_pattern(&mmu, addr, 0,PAGESIZE, buf));
	ASSERT_EQUALS(ecSuccess, disk_set_page(&disk, disk_page, buf));

	ASSERT_EQUALS(ecSuccess, mmu_sync_from_backing_page(&mmu, addr));

	memset(buf, 0, ARRSIZE(buf));
	ASSERT_EQUALS(ecSuccess, mmu_read(&mmu, addr, 0, PAGESIZE, buf));
	ASSERT_EQUALS(ceSuccess, validate_pattern(&mmu, addr, 0, PAGESIZE, buf));

	ASSERT_EQUALS(ecSuccess, mmu_free_page(&mmu, addr));

	mm_destroy(&mem);
	disk_destroy(&disk);
	mmu_destroy(&mmu);

	return ceSuccess;
}

void add_mmu_tests()
{
	ADD_TEST(test_mmu_alloc_free_sanity);
	ADD_TEST(test_mmu_alloc_free_multiple);
	ADD_TEST(test_mmu_alloc_read_free_multiple);
	ADD_TEST(test_mmu_alloc_free_oom);
	ADD_TEST(test_mmu_read_write_sanity);
	ADD_TEST(test_mmu_read_write_pagefault);
	ADD_TEST(test_mmu_sync_to_backing_page);
	ADD_TEST(test_mmu_sync_from_backing_page);
	ADD_TEST(test_mmu_alloc_free_stress);
	ADD_TEST(test_mmu_alloc_free_pagefault_stress);
}
