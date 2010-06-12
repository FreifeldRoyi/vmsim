#include "cunit/cunit.h"

#include "util/worker_thread.h"

#include <string.h>
#include <unistd.h>

#define BLOCK_SIZE 2
#define NPAGES 11
#define PAGESIZE 32
#define NPROCS NPAGES/BLOCK_SIZE


cunit_err_t test_disk_init()
{
	disk_t disk;
	int idx;

	ASSERT_EQUALS(ecSuccess, disk_init(&disk, NPAGES, PAGESIZE, BLOCK_SIZE));
	for (idx = 0; idx < NPAGES * PAGESIZE; ++idx)
	{
		//make sure all pages are accessible for read/write.
		disk.data[idx] = 0xAA;
	}

	disk_destroy(&disk);
	return ceSuccess;
}

cunit_err_t test_disk_set_get_page()
{
	disk_t disk;
	int pageidx, i;
	int junk;
	BYTE *page_data = calloc(1, PAGESIZE);

	ASSERT_EQUALS(ecSuccess, disk_init(&disk, NPAGES, PAGESIZE, NPAGES));
	//allocate the entire disk, just to prevent assertions
	disk_alloc_process_block(&disk, &junk);

	for (pageidx = 0; pageidx < NPAGES; ++pageidx)
	{
		memset(page_data, pageidx, PAGESIZE);
		ASSERT_EQUALS(ecSuccess, disk_set_page(&disk, pageidx, page_data));
	}

	for (pageidx = 0; pageidx < NPAGES; ++pageidx)
	{
		ASSERT_EQUALS(ecSuccess, disk_get_page(&disk, pageidx, page_data));
		for (i=0; i<PAGESIZE; ++i)
		{
			ASSERT_EQUALS(page_data[i], (BYTE)pageidx);
		}
	}
	free(page_data);
	disk_destroy(&disk);
	return ceSuccess;
}

cunit_err_t test_disk_get_page_start()
{
	disk_t disk;
	int i, page_start=0;
	ASSERT_EQUALS(ecSuccess, disk_init(&disk, NPAGES, PAGESIZE, BLOCK_SIZE));

	for (i=0; i<NPAGES; ++i)
	{
		ASSERT_EQUALS(page_start ,get_page_start(&disk, i)- disk.data);
		page_start += PAGESIZE;
	}

	disk_destroy(&disk);
	return ceSuccess;
}

cunit_err_t test_disk_alloc_free_process_block()
{
	disk_t disk;
	int i,j, junk;
	int process_pages[NPROCS];

	ASSERT_EQUALS(ecSuccess, disk_init(&disk, NPAGES, PAGESIZE, BLOCK_SIZE));

	//allocate max amount of process blocks
	for (i=0; i<NPROCS; ++i)
	{
		ASSERT_EQUALS(ecSuccess, disk_alloc_process_block(&disk, &process_pages[i]));
		for (j=0; j<i; ++j)
		{//check that we really got different addresses
			ASSERT_TRUE(process_pages[i] != process_pages[j]);
		}
	}

	//make sure we can't allocate more blocks
	ASSERT_TRUE(ecSuccess != disk_alloc_process_block(&disk, &junk));

	//free all the blocks
	for (i=0; i<NPROCS; ++i)
	{
		ASSERT_EQUALS(ecSuccess, disk_free_process_block(&disk, process_pages[i]));
	}

	//make sure we can now allocate the max amount of blocks
	for (i=0; i<NPROCS; ++i)
	{
		ASSERT_EQUALS(ecSuccess, disk_alloc_process_block(&disk, &process_pages[i]));
		for (j=0; j<i; ++j)
		{//check that we really got different addresses
			ASSERT_TRUE(process_pages[i] != process_pages[j]);
		}
	}

	disk_destroy(&disk);
	return ceSuccess;
}

#define MT_MAX_BLOCKS 1000

typedef struct {
	int num_alloc;
	disk_t* disk;
	int* blocks;
}disk_alloc_func_params_t;

typedef struct {
	disk_t* disk;
	int* blocks;
}disk_modify_func_params_t;

static BOOL disk_alloc_func(void* arg)
{
	disk_alloc_func_params_t* param = (disk_alloc_func_params_t*)arg;
	if (param->num_alloc == MT_MAX_BLOCKS)
		return TRUE;

	disk_alloc_process_block(param->disk, &param->blocks[param->num_alloc]);
	++param->num_alloc;

	return FALSE;
}

static BOOL disk_read_func(void* arg)
{
	disk_modify_func_params_t* param = (disk_modify_func_params_t*)arg;
	int i;
	BYTE page_data[PAGESIZE];

	for (i=0;i<NPROCS * MT_MAX_BLOCKS; ++i)
	{
		disk_get_page(param->disk, param->blocks[i], page_data);
	}

	return FALSE;
}

static BOOL disk_write_func(void* arg)
{
	disk_modify_func_params_t* param = (disk_modify_func_params_t*)arg;
	int i;
	BYTE page_data[PAGESIZE];

	memset(page_data, 0xDEADCACA, PAGESIZE);

	for (i=0;i<NPROCS * MT_MAX_BLOCKS; ++i)
	{
		disk_set_page(param->disk, param->blocks[i], page_data);
	}

	return FALSE;
}
cunit_err_t test_disk_multithread_alloc()
{
	int blocks[NPROCS * MT_MAX_BLOCKS];
	disk_alloc_func_params_t params[NPROCS];
	worker_thread_t threads[NPROCS];
	disk_t disk;
	int i;
	BOOL done = FALSE;

	memset(blocks,0,sizeof(blocks));

	ASSERT_EQUALS(ecSuccess, disk_init(&disk, NPAGES*MT_MAX_BLOCKS, PAGESIZE, BLOCK_SIZE));

	for (i=0; i<NPROCS; ++i)
	{
		params[i].blocks = &blocks[MT_MAX_BLOCKS * i];
		params[i].num_alloc = 0;
		params[i].disk = &disk;
		ASSERT_EQUALS(ecSuccess, worker_thread_create(&threads[i], disk_alloc_func));
	}

	for (i=0; i<NPROCS; ++i)
	{
		ASSERT_EQUALS(ecSuccess, worker_thread_start(&threads[i], &params[i]));
	}

	while (!done)
	{
		done = TRUE;

		for (i=0; i<NPROCS; ++i)
		{
			done = done && !worker_thread_is_running(&threads[i]);
		}
	}

	for (i=0; i<NPROCS; ++i)
	{
		worker_thread_destroy(&threads[i]);
	}

	disk_destroy(&disk);

	return ceSuccess;
}


cunit_err_t test_disk_multithread_alloc_modify()
{
	int blocks[NPROCS * MT_MAX_BLOCKS];
	disk_alloc_func_params_t alloc_params[NPROCS];
	disk_modify_func_params_t modify_params[NPROCS];

	worker_thread_t alloc_threads[NPROCS];
	worker_thread_t read_threads[NPROCS];
	worker_thread_t write_threads[NPROCS];
	disk_t disk;
	int i;
	BOOL done = FALSE;

	memset(blocks,0xFF,sizeof(blocks));

	ASSERT_EQUALS(ecSuccess, disk_init(&disk, NPAGES*MT_MAX_BLOCKS, PAGESIZE, BLOCK_SIZE));

	for (i=0; i<NPROCS; ++i)
	{
		alloc_params[i].blocks = &blocks[MT_MAX_BLOCKS * i];
		alloc_params[i].num_alloc = 0;
		alloc_params[i].disk = &disk;
		modify_params[i].blocks = blocks;
		modify_params[i].disk = &disk;

		ASSERT_EQUALS(ecSuccess, worker_thread_create(&alloc_threads[i], disk_alloc_func));
		ASSERT_EQUALS(ecSuccess, worker_thread_create(&read_threads[i], disk_read_func));
		ASSERT_EQUALS(ecSuccess, worker_thread_create(&write_threads[i], disk_write_func));
	}

	for (i=0; i<NPROCS; ++i)
	{
		ASSERT_EQUALS(ecSuccess, worker_thread_start(&alloc_threads[i], &alloc_params[i]));
		ASSERT_EQUALS(ecSuccess, worker_thread_start(&read_threads[i], &modify_params[i]));
		ASSERT_EQUALS(ecSuccess, worker_thread_start(&write_threads[i], &modify_params[i]));
	}

	while (!done)
	{
		done = TRUE;

		for (i=0; i<NPROCS; ++i)
		{
			done = done && !worker_thread_is_running(&alloc_threads[i]);
		}
	}

	sleep(2);

	for (i=0; i<NPROCS; ++i)
	{
		ASSERT_EQUALS(ecSuccess, worker_thread_stop(&read_threads[i]));
		ASSERT_EQUALS(ecSuccess, worker_thread_stop(&write_threads[i]));
		worker_thread_destroy(&alloc_threads[i]);
		worker_thread_destroy(&read_threads[i]);
		worker_thread_destroy(&write_threads[i]);
	}

	disk_destroy(&disk);

	return ceSuccess;
}

void add_disk_tests()
{
	ADD_TEST(test_disk_init);
	ADD_TEST(test_disk_get_page_start);
	ADD_TEST(test_disk_set_get_page);
	ADD_TEST(test_disk_alloc_free_process_block);
	ADD_TEST(test_disk_multithread_alloc);
	ADD_TEST(test_disk_multithread_alloc_modify);
}
