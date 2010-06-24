#include "cunit/cunit.h"
#include "util/worker_thread.h"
#include <string.h>
#include <unistd.h>
#include <time.h>


#define MEM_NPAGES 11
#define DISK_NPAGES 109
#define DISK_BLOCKSIZE 10
#define PAGESIZE 32

#define NEVER_AGE -1

static cunit_err_t
write_pattern(unsigned nbytes, BYTE* buf)
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
	addr.offset = offset;

	ASSERT_EQUALS(ceSuccess, write_pattern(nbytes, buf));
	ASSERT_EQUALS(ecSuccess, mmu_write(mmu, addr, nbytes, buf));
	memset(buf, 0, ARRSIZE(buf));
	ASSERT_EQUALS(ecSuccess, mmu_read(mmu, addr, nbytes, buf));
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
	virt_addr_t addr = {0,0,0};
	int i;

	ASSERT_EQUALS(ecSuccess, mm_init(&mem, MEM_NPAGES, PAGESIZE));
	ASSERT_EQUALS(ecSuccess, disk_init(&disk,DISK_NPAGES, PAGESIZE, DISK_BLOCKSIZE));

	ASSERT_EQUALS(ecSuccess, mmu_init(&mmu, &mem, &disk, NEVER_AGE));

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
	virt_addr_t addr = {0,0,0};
	int i;

	ASSERT_EQUALS(ecSuccess, mm_init(&mem, MEM_NPAGES, PAGESIZE));
	ASSERT_EQUALS(ecSuccess, disk_init(&disk,DISK_NPAGES, PAGESIZE, DISK_BLOCKSIZE));

	ASSERT_EQUALS(ecSuccess, mmu_init(&mmu, &mem, &disk, NEVER_AGE));

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
	virt_addr_t addr = {0,0,0};
	int i,j;
	BYTE buf[PAGESIZE];

	ASSERT_EQUALS(ecSuccess, mm_init(&mem, MEM_NPAGES, PAGESIZE));
	ASSERT_EQUALS(ecSuccess, disk_init(&disk,DISK_NPAGES, PAGESIZE, DISK_BLOCKSIZE));

	ASSERT_EQUALS(ecSuccess, mmu_init(&mmu, &mem, &disk, NEVER_AGE));

	for (i=0; i< 5; ++i)
	{
		ASSERT_EQUALS(ecSuccess, mmu_alloc_multiple(&mmu, addr,MEM_NPAGES, 0));
		for (j=0; j<MEM_NPAGES; ++j)
		{
			VIRT_ADDR_PAGE(addr) = j;
			mmu_read(&mmu, addr, PAGESIZE, buf);
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

	ASSERT_EQUALS(ecSuccess, mmu_init(&mmu, &mem, &disk, NEVER_AGE));

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
do_test_mmu_alloc_free_stress(mmu_t* mmu, int alloc_size)
{
	worker_thread_t threads[NPROCS];
	alloc_free_thread_params_t params[NPROCS];
	int i;

	ASSERT_EQUALS(ecSuccess, prm_init(mmu));


	for (i=0; i<NPROCS; ++i)
	{
		worker_thread_create(&threads[i],alloc_free_func);
		params[i].pid = i;
		params[i].mmu = mmu;
		params[i].nallocs = 0;
		params[i].alloc_size = alloc_size;
		disk_alloc_process_block(mmu->disk, &params[i].disk_page);
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


	return ceSuccess;
}

cunit_err_t test_mmu_alloc_free_stress()
{
	mmu_t mmu;
	mm_t mem;
	disk_t disk;
	cunit_err_t err;

	ASSERT_EQUALS(ecSuccess, mm_init(&mem, MEM_NPAGES, PAGESIZE));
	ASSERT_EQUALS(ecSuccess, disk_init(&disk,DISK_NPAGES, PAGESIZE, DISK_BLOCKSIZE));
	ASSERT_EQUALS(ecSuccess, mmu_init(&mmu, &mem, &disk, NEVER_AGE));

	err = do_test_mmu_alloc_free_stress(&mmu, 1);

	mmu_destroy(&mmu);
	disk_destroy(&disk);
	mm_destroy(&mem);

	return err;
}

cunit_err_t test_mmu_alloc_free_pagefault_stress()
{
	mmu_t mmu;
	mm_t mem;
	disk_t disk;
	cunit_err_t err;

	ASSERT_EQUALS(ecSuccess, mm_init(&mem, MEM_NPAGES, PAGESIZE));
	ASSERT_EQUALS(ecSuccess, disk_init(&disk,DISK_NPAGES, PAGESIZE, DISK_BLOCKSIZE));
	ASSERT_EQUALS(ecSuccess, mmu_init(&mmu, &mem, &disk, NEVER_AGE));

	err = do_test_mmu_alloc_free_stress(&mmu, DISK_BLOCKSIZE);

	mmu_destroy(&mmu);
	disk_destroy(&disk);
	mm_destroy(&mem);

	return err;
}

BOOL hat_locking_func(void* arg)
{
	mmu_t* mmu = arg;
	struct timespec ts;
	ts.tv_sec = 0;
	ts.tv_nsec = 500000;

	rwlock_acquire_write(&mmu->mem_ipt.hat_lock);
	nanosleep(&ts,NULL);
	rwlock_release_write(&mmu->mem_ipt.hat_lock);

	return FALSE;
}

cunit_err_t test_mmu_alloc_free_pagefault_hatlock_stress()
{
	mmu_t mmu;
	mm_t mem;
	disk_t disk;
	worker_thread_t hat_locking_thread;
	cunit_err_t err;

	ASSERT_EQUALS(ecSuccess, mm_init(&mem, MEM_NPAGES, PAGESIZE));
	ASSERT_EQUALS(ecSuccess, disk_init(&disk,DISK_NPAGES, PAGESIZE, DISK_BLOCKSIZE));
	ASSERT_EQUALS(ecSuccess, mmu_init(&mmu, &mem, &disk, NEVER_AGE));

	ASSERT_EQUALS(ecSuccess, worker_thread_create(&hat_locking_thread, &hat_locking_func));
	ASSERT_EQUALS(ecSuccess, worker_thread_start(&hat_locking_thread, &mmu));

	err = do_test_mmu_alloc_free_stress(&mmu, DISK_BLOCKSIZE);

	ASSERT_EQUALS(ecSuccess, worker_thread_stop(&hat_locking_thread));

	mmu_destroy(&mmu);
	disk_destroy(&disk);
	mm_destroy(&mem);

	return err;
}

cunit_err_t test_mmu_read_write_sanity()
{
	mmu_t mmu;
	mm_t mem;
	disk_t disk;
	virt_addr_t addr = {0,0,0};

	ASSERT_EQUALS(ecSuccess, mm_init(&mem, MEM_NPAGES, PAGESIZE));
	ASSERT_EQUALS(ecSuccess, disk_init(&disk,DISK_NPAGES, PAGESIZE, DISK_BLOCKSIZE));

	ASSERT_EQUALS(ecSuccess, mmu_init(&mmu, &mem, &disk, NEVER_AGE));

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
	virt_addr_t addr = {0,0,0};

	ASSERT_EQUALS(ecSuccess, mm_init(&mem, MEM_NPAGES, PAGESIZE));
	ASSERT_EQUALS(ecSuccess, disk_init(&disk,DISK_NPAGES, PAGESIZE, DISK_BLOCKSIZE));

	ASSERT_EQUALS(ecSuccess, mmu_init(&mmu, &mem, &disk, NEVER_AGE));

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
	virt_addr_t addr = {0,0,0};
	int disk_page;
	BYTE buf[PAGESIZE];

	ASSERT_EQUALS(ecSuccess, mm_init(&mem, MEM_NPAGES, PAGESIZE));
	ASSERT_EQUALS(ecSuccess, disk_init(&disk,DISK_NPAGES, PAGESIZE, DISK_BLOCKSIZE));

	ASSERT_EQUALS(ecSuccess, disk_alloc_process_block(&disk, &disk_page));

	ASSERT_EQUALS(ecSuccess, mmu_init(&mmu, &mem, &disk, NEVER_AGE));

	VIRT_ADDR_PAGE(addr) = 0;
	VIRT_ADDR_PID(addr) = 9999;
	ASSERT_EQUALS(ecSuccess, mmu_alloc_page(&mmu, addr, disk_page));

	ASSERT_EQUALS(ceSuccess, write_pattern(PAGESIZE, buf));
	ASSERT_EQUALS(ecSuccess, mmu_write(&mmu, addr, PAGESIZE, buf));

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
	virt_addr_t addr = {0,0,0};
	int disk_page;
	BYTE buf[PAGESIZE];

	ASSERT_EQUALS(ecSuccess, mm_init(&mem, MEM_NPAGES, PAGESIZE));
	ASSERT_EQUALS(ecSuccess, disk_init(&disk,DISK_NPAGES, PAGESIZE, DISK_BLOCKSIZE));

	ASSERT_EQUALS(ecSuccess, disk_alloc_process_block(&disk, &disk_page));

	ASSERT_EQUALS(ecSuccess, mmu_init(&mmu, &mem, &disk, NEVER_AGE));

	VIRT_ADDR_PAGE(addr) = 0;
	VIRT_ADDR_PID(addr) = 9999;
	ASSERT_EQUALS(ecSuccess, mmu_alloc_page(&mmu, addr, disk_page));

	ASSERT_EQUALS(ceSuccess, write_pattern(PAGESIZE, buf));
	ASSERT_EQUALS(ecSuccess, disk_set_page(&disk, disk_page, buf));

	ASSERT_EQUALS(ecSuccess, mmu_sync_from_backing_page(&mmu, addr));

	memset(buf, 0, ARRSIZE(buf));
	ASSERT_EQUALS(ecSuccess, mmu_read(&mmu, addr, PAGESIZE, buf));
	ASSERT_EQUALS(ceSuccess, validate_pattern(&mmu, addr, 0, PAGESIZE, buf));

	ASSERT_EQUALS(ecSuccess, mmu_free_page(&mmu, addr));

	mm_destroy(&mem);
	disk_destroy(&disk);
	mmu_destroy(&mmu);

	return ceSuccess;
}

#define AGING_FREQ 5

cunit_err_t test_mmu_aging_sanity()
{
	mmu_t mmu;
	mm_t mem;
	disk_t disk;
	phys_addr_t paddr;
	virt_addr_t addr = {0,0,0};
	BYTE buf[PAGESIZE];
	int i;

	ASSERT_EQUALS(ecSuccess, mm_init(&mem, MEM_NPAGES, PAGESIZE));
	ASSERT_EQUALS(ecSuccess, disk_init(&disk,DISK_NPAGES, PAGESIZE, DISK_BLOCKSIZE));
	ASSERT_EQUALS(ecSuccess, mmu_init(&mmu, &mem, &disk, AGING_FREQ));

	ASSERT_EQUALS(ecSuccess, aging_daemon_start(&mmu));

	ASSERT_EQUALS(ecSuccess, mmu_alloc_page(&mmu, addr, 0));

	for (i=0; i < AGING_FREQ * 3; ++i)
	{
		ASSERT_EQUALS(ecSuccess, mmu_read(&mmu, addr, PAGESIZE, buf));
	}

	//we expect the aging daemon to run 3 times, and this page was referenced in all 3.
	//this means it's uppermost 4 bits should be 1
	ASSERT_EQUALS(ecSuccess, ipt_translate(&mmu.mem_ipt, addr, &paddr));
	ASSERT_EQUALS(0xF0000000, mmu.mem_ipt.entries[paddr].page_data.page_age);

	ASSERT_EQUALS(ecSuccess, mmu_free_page(&mmu, addr));

	aging_daemon_stop();

	mmu_destroy(&mmu);
	mm_destroy(&mem);
	disk_destroy(&disk);

	return ceSuccess;
}

cunit_err_t test_mmu_aging_2pages_rw()
{
	mmu_t mmu;
	mm_t mem;
	disk_t disk;
	phys_addr_t paddr;
	virt_addr_t addr1 = {0,0,0}, addr2 = {1,1,0};
	BYTE buf[PAGESIZE];
	int i;

	ASSERT_EQUALS(ecSuccess, mm_init(&mem, MEM_NPAGES, PAGESIZE));
	ASSERT_EQUALS(ecSuccess, disk_init(&disk,DISK_NPAGES, PAGESIZE, DISK_BLOCKSIZE));
	ASSERT_EQUALS(ecSuccess, mmu_init(&mmu, &mem, &disk, AGING_FREQ));

	ASSERT_EQUALS(ecSuccess, aging_daemon_start(&mmu));

	ASSERT_EQUALS(ecSuccess, mmu_alloc_page(&mmu, addr1, 0));
	ASSERT_EQUALS(ecSuccess, mmu_alloc_page(&mmu, addr2, 1));

	for (i=0; i < AGING_FREQ * 3; ++i)
	{
		ASSERT_EQUALS(ecSuccess, mmu_read(&mmu, addr1, PAGESIZE, buf));
	}

	for (i=0; i < AGING_FREQ * 3; ++i)
	{
		ASSERT_EQUALS(ecSuccess, mmu_write(&mmu, addr2, PAGESIZE, buf));
	}


	ASSERT_EQUALS(ecSuccess, ipt_translate(&mmu.mem_ipt, addr1, &paddr));
	ASSERT_EQUALS(0xF0000000>>3, mmu.mem_ipt.entries[paddr].page_data.page_age);

	ASSERT_EQUALS(ecSuccess, ipt_translate(&mmu.mem_ipt, addr2, &paddr));
	ASSERT_EQUALS(0xe2000000, mmu.mem_ipt.entries[paddr].page_data.page_age);

	ASSERT_EQUALS(ecSuccess, mmu_free_page(&mmu, addr1));
	ASSERT_EQUALS(ecSuccess, mmu_free_page(&mmu, addr2));

	aging_daemon_stop();

	mmu_destroy(&mmu);
	mm_destroy(&mem);
	disk_destroy(&disk);

	return ceSuccess;
}

cunit_err_t test_mmu_alloc_free_pagefault_aging_stress()
{
	mmu_t mmu;
	mm_t mem;
	disk_t disk;
	cunit_err_t err;

	ASSERT_EQUALS(ecSuccess, mm_init(&mem, MEM_NPAGES, PAGESIZE));
	ASSERT_EQUALS(ecSuccess, disk_init(&disk,DISK_NPAGES, PAGESIZE, DISK_BLOCKSIZE));
	ASSERT_EQUALS(ecSuccess, mmu_init(&mmu, &mem, &disk, AGING_FREQ));

	aging_daemon_start(&mmu);

	err = do_test_mmu_alloc_free_stress(&mmu, DISK_BLOCKSIZE);

	aging_daemon_stop(&mmu);

	mmu_destroy(&mmu);
	disk_destroy(&disk);
	mm_destroy(&mem);

	return err;
}

static BOOL stats_thread_func(void* arg)
{
	mmu_t* mmu = arg;
	mmu_stats_t stats= mmu_get_stats(mmu);
	struct timespec ts;
	static mmu_stats_t old_stats = {0,0};

	assert(old_stats.hits <= stats.hits);
	assert(old_stats.nrefs <= stats.nrefs);

	ts.tv_sec = 0;
	ts.tv_nsec = 50000;
	nanosleep(&ts, NULL);
	old_stats = stats;
	return FALSE;
}

cunit_err_t test_mmu_alloc_free_pagefault_aging_stats_stress()
{
	mmu_t mmu;
	mm_t mem;
	disk_t disk;
	cunit_err_t err;
	worker_thread_t stats_thread;

	assert(ecSuccess == worker_thread_create(&stats_thread, &stats_thread_func));

	ASSERT_EQUALS(ecSuccess, mm_init(&mem, MEM_NPAGES, PAGESIZE));
	ASSERT_EQUALS(ecSuccess, disk_init(&disk,DISK_NPAGES, PAGESIZE, DISK_BLOCKSIZE));
	ASSERT_EQUALS(ecSuccess, mmu_init(&mmu, &mem, &disk, AGING_FREQ));

	worker_thread_start(&stats_thread, &mmu);

	aging_daemon_start(&mmu);

	err = do_test_mmu_alloc_free_stress(&mmu, DISK_BLOCKSIZE);

	aging_daemon_stop(&mmu);
	worker_thread_stop(&stats_thread);

	ASSERT_TRUE(mmu_get_stats(&mmu).nrefs > 0);
	ASSERT_TRUE(mmu_get_stats(&mmu).hits > 0);

	mmu_destroy(&mmu);
	disk_destroy(&disk);
	mm_destroy(&mem);

	return err;
}

typedef struct
{
	mmu_t* mmu;
	int pid;
}loop_writer_params_t;

BOOL loop_writer_thread(void* arg)
{
	loop_writer_params_t* params = arg;
	BYTE data = (BYTE)params->pid;
	virt_addr_t addr;
	int i;
	int page_size = params->mmu->mem->page_size;
	int npages = params->mmu->disk->process_block_size;

	VIRT_ADDR_PID(addr) = params->pid;
	VIRT_ADDR_PAGE(addr) = 0;

	for (i=0; i< page_size * npages; ++i)
	{
		VIRT_ADDR_PAGE(addr) = i/page_size;
		VIRT_ADDR_OFFSET(addr) = i%page_size;
		ASSERT_EQUALS(ecSuccess, mmu_write(params->mmu, addr, 1, &data));
	}

	for (i=0; i< page_size * npages; ++i)
	{
		data = 0;
		VIRT_ADDR_PAGE(addr) = i/page_size;
		VIRT_ADDR_OFFSET(addr) = i%page_size;
		ASSERT_EQUALS(ecSuccess, mmu_read(params->mmu, addr, 1, &data));
		ASSERT_EQUALS(params->pid, data);
	}

	return FALSE;
}

cunit_err_t do_test_mmu_submission_test(int nprocs,
										int page_size,
										int mm_npages,
										int disk_npages,
										int proc_npages,
										int aging_freq)
{
	mmu_t mmu;
	mm_t mem;
	disk_t disk;
	worker_thread_t threads[nprocs];
	loop_writer_params_t params[nprocs];
	int i;
	int disk_page;
	virt_addr_t addr = {0,0,0};

	ASSERT_EQUALS(ecSuccess, mm_init(&mem, mm_npages, page_size));
	ASSERT_EQUALS(ecSuccess, disk_init(&disk,disk_npages, page_size, proc_npages));
	ASSERT_EQUALS(ecSuccess, mmu_init(&mmu, &mem, &disk, aging_freq));

	ASSERT_EQUALS(ecSuccess, aging_daemon_start(&mmu));
	ASSERT_EQUALS(ecSuccess, prm_init(&mmu));

	for (i=0; i<nprocs; ++i)
	{
		worker_thread_create(&threads[i],loop_writer_thread);
		params[i].pid = i;
		params[i].mmu = &mmu;
		disk_alloc_process_block(mmu.disk, &disk_page);
		VIRT_ADDR_PID(addr) = i;
		mmu_alloc_multiple(&mmu, addr, proc_npages, disk_page);
	}

	for (i=0; i<nprocs; ++i)
	{
		worker_thread_start(&threads[i],&params[i]);
	}

	sleep(5);

	for (i=0; i<nprocs; ++i)
	{
		worker_thread_stop(&threads[i]);
		worker_thread_destroy(&threads[i]);
	}

	prm_destroy();
	mmu_destroy(&mmu);
	disk_destroy(&disk);
	mm_destroy(&mem);

	return ceSuccess;
}

cunit_err_t test_mmu_submission_test1()
{
	return do_test_mmu_submission_test(2,8,2,8,4,2);
}

cunit_err_t test_mmu_submission_test2()
{
	return do_test_mmu_submission_test(6,16,16,96,16,32);
}

cunit_err_t test_mmu_submission_test3()
{
	return do_test_mmu_submission_test(16,1024,512,16384,1024,512);
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
	ADD_TEST(test_mmu_aging_sanity);
	ADD_TEST(test_mmu_aging_2pages_rw);
	ADD_TEST(test_mmu_alloc_free_stress);
	ADD_TEST(test_mmu_alloc_free_pagefault_stress);
	ADD_TEST(test_mmu_alloc_free_pagefault_hatlock_stress);
	ADD_TEST(test_mmu_alloc_free_pagefault_aging_stress);
	ADD_TEST(test_mmu_alloc_free_pagefault_aging_stats_stress);
	ADD_TEST(test_mmu_submission_test1);
	ADD_TEST(test_mmu_submission_test2);
	ADD_TEST(test_mmu_submission_test3);
}
