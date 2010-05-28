/*
 * ui_util.c
 *
 *  Created on: May 19, 2010
 *      Author: Freifeld Royi
 */

#include "app_util.h"
#include <string.h>
#include <malloc.h>
#include <assert.h>

static void print_ipt_entry(ipt_entry_t entry)
{
	if (!(entry.page_data.valid))
	{
		procid_t pid = entry.page_data.addr.pid;
		unsigned page_num = entry.page_data.addr.page;
		BOOL dirty = entry.page_data.dirty;
		BOOL aging_ref = entry.page_data.referenced;
		int next = entry.next;
		int prev = entry.prev;

		printf("(%u, %u, %d, %d, %d, %d)\n",pid,page_num,dirty,aging_ref,next,prev);
	}
	else
	{
		printf("(free)\n");
	}
}

/**
 * prints a binary representation of BYTEs
 * with no new line char ("\n") at the end
 */
static void print_BYTE(BYTE* byte)
{
	int i;

	for (i = BYTE_SIZE - 1; i <= 0; --i)
	{
		if ((1 << i) & (*byte))
			printf("1");
		else
			printf("0");
	}
}

static void print_bitmap(bitmap_t* bitmap)
{
	int i;

	for (i = 0; i < bitmap->size; ++i)
	{
		printf("[%d]: ", i);
		print_BYTE(&bitmap->data[i]);
		printf("\n");
	}

	printf("\n");
}

void print_MMU_table(ipt_t* table)
{
	int i,tbl_size = table->size;
	ipt_entry_t *entries = table->entries;
	printf("NOTE\n----\nentries will be printed as follows: (pid, page_num, dirty_bit, aging_reference_bit, next, prev)"
			"for free (unused) pages \"(free)\" will be printed\n");

	for (i = 0; i < tbl_size; ++i)
	{
		print_ipt_entry(entries[i]);
	}
}

void print_MM(mm_t* mm)
{
	int num_of_pages = MM_NUM_OF_PAGES(&mm);
	int	page_size = MM_PAGE_SIZE(&mm);

	printf("DATA: ");
	print_BYTE(MM_DATA(&mm));

	printf("NUM OF PAGES: %d\n", num_of_pages);
	printf("PAGE SIZE: %d\n", page_size);

	printf("BITMAP: ");
	print_bitmap(&MM_BITMAP(&mm));
}

int create_process()
{
	//TODO implement and check h file if parameters were added
}

void del_process(int pid)
{
	//TODO implement
}

static BOOL input_ok(int err)
{
	if (err <= 0 || err == EOF)
		return FALSE;

	return TRUE;
}

BOOL load_app_data(char* file_name, app_data_t* app_data)
{
	FILE* f;
	unsigned n_page_mm;
	unsigned n_page_disk;

	int err;

	assert(app_data != NULL);

	if (app_data->initialized)
	{
		printf("Data already initialized");
		return FALSE;
	}

	f = fopen(file_name,"r");

	if (f == NULL)
	{
		printf("File not found: %s", file_name);
		return FALSE;
	}

	fscanf(f, "MaxNumOfProcesses = %u", &APP_DATA_NUM_OF_PROC(app_data));
	fscanf(f, "PageSize = %u", &APP_DATA_PAGE_SIZE(app_data));
	fscanf(f, "NumOfPagesInMM = %u", &n_page_mm);
	fscanf(f, "NumOfPagesInDisk = %u", &n_page_disk);
	fscanf(f, "NumOfProcessPages = %u", &APP_DATA_NUM_OF_PROC_PAGE(app_data));
	fscanf(f, "ShiftClock = %u", &APP_DATA_SHIFT_CLOCK(app_data));

	disk_init(APP_DATA_DISK(app_data), n_page_disk, APP_DATA_PAGE_SIZE(app_data), APP_DATA_PAGE_SIZE(app_data)/* TODO block size maybe a different size??*/);
	mm_init(APP_DATA_MM(app_data), n_page_mm, APP_DATA_PAGE_SIZE(app_data));
	mmu_init(APP_DATA_MMU(app_data), APP_DATA_MM(app_data), APP_DATA_DISK(app_data));
	//TODO if any more fields are added to the app_data struct, don't forget to handle here

	fclose(f);

	return TRUE;
}

void free_app_data(app_data_t* app_data)
{
	mm_destroy(APP_DATA_MM(app_data));
	disk_destroy(APP_DATA_DISK(app_data));
	mmu_destroy(APP_DATA_MMU(app_data));

	//TODO if any more fields are added to the app_data struct, don't forget to handle here

	free(app_data);
}