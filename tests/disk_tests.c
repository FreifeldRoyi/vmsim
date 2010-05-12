#include "cunit/cunit.h"

#include <string.h>

const int NPAGES = 10;
const int PAGESIZE = 16;


cunit_err_t test_disk_init()
{
	disk_t disk;
	int idx;

	ASSERT_EQUALS(ecSuccess, disk_init(&disk, NPAGES, PAGESIZE));
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
	BYTE *page_data = calloc(1, PAGESIZE);

	ASSERT_EQUALS(ecSuccess, disk_init(&disk, NPAGES, PAGESIZE));

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
	ASSERT_EQUALS(ecSuccess, disk_init(&disk, NPAGES, PAGESIZE));

	for (i=0; i<NPAGES; ++i)
	{
		ASSERT_EQUALS(page_start ,get_page_start(&disk, i)- disk.data);
		page_start += PAGESIZE;
	}

	disk_destroy(&disk);
	return ceSuccess;
}

void add_disk_tests()
{
	ADD_TEST(test_disk_init);
	ADD_TEST(test_disk_get_page_start);
	ADD_TEST(test_disk_set_get_page);
}
