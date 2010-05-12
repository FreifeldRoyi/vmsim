#include "cunit/cunit.h"

#include <string.h>

const int BLOCK_SIZE = 2;
const int NPAGES = 11;
const int PAGESIZE = 1024;


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
	const int NPROCS = NPAGES/BLOCK_SIZE;
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

void add_disk_tests()
{
	ADD_TEST(test_disk_init);
	ADD_TEST(test_disk_get_page_start);
	ADD_TEST(test_disk_set_get_page);
	ADD_TEST(test_disk_alloc_free_process_block);
}
