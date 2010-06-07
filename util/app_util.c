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

#define BLOCK_SIZE 8 //TODO initial size.file doesn't contain this info

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
static void print_BYTE_binary(BYTE* byte)
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

static void print_bitmap_binary(bitmap_t* bitmap)
{
	int i;

	for (i = 0; i < bitmap->size; ++i)
	{
		printf("[%d]: ", i);
		print_BYTE_binary(&bitmap->data[i]);
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
	print_BYTE_binary(MM_DATA(&mm));

	printf("NUM OF PAGES: %d\n", num_of_pages);
	printf("PAGE SIZE: %d\n", page_size);

	//printf("BITMAP: ");
	//print_bitmap_binary(&MM_BITMAP(&mm));
}

BOOL load_app_data(char* file_name, app_data_t* app_data)
{
	FILE* f;

	unsigned nproc;
	unsigned page_size;
	unsigned n_page_mm;
	unsigned n_page_disk;
	unsigned n_proc_pages;
	unsigned shift_clock;

	mm_t* mm = (mm_t*)calloc(0, sizeof(mm_t));
	disk_t* disk = (disk_t*)calloc(0, sizeof(disk_t));;
	mmu_t* mmu = (mmu_t*)calloc(0, sizeof(mmu_t));;

	assert(app_data != NULL);
	assert(mm != NULL);
	assert(disk != NULL);
	assert(mmu != NULL);

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

	fscanf(f, "MaxNumOfProcesses = %u\n", &nproc);
	fscanf(f, "PageSize = %u\n", &page_size);
	fscanf(f, "NumOfPagesInMM = %u\n", &n_page_mm);
	fscanf(f, "NumOfPagesInDisk = %u\n", &n_page_disk);
	fscanf(f, "NumOfProcessPages = %u\n", &n_proc_pages);
	fscanf(f, "ShiftClock = %u", &shift_clock);
	//input correctness isn't checked

	APP_DATA_PAGE_SIZE(app_data) = page_size;
	APP_DATA_NUM_OF_PROC_PAGE(app_data) = n_proc_pages;
	APP_DATA_SHIFT_CLOCK(app_data) = shift_clock;

	disk_init(disk, n_page_disk, page_size,  page_size/* TODO block size maybe a different size??*/);
	mm_init(mm, n_page_mm, page_size);
	mmu_init(mmu, mm, disk, shift_clock);

	APP_DATA_PROC_CONT(app_data) = init_proc_cont(nproc, mmu);
	//TODO if any more fields are added to the app_data struct, don't forget to handle here
	//TODO what about PRM and Aging Daemon?

	//TODO seg fault in fclose(f);
	//fclose(f);

	APP_DATA_INIT(app_data) = TRUE;

	return TRUE;
}

void free_app_data(app_data_t* app_data)
{
	proc_cont_t* proc_cont = APP_DATA_PROC_CONT(app_data);
	mmu_t* mmu = PROC_CONT_MMU(proc_cont);

	//TODO NOTE: if mm and disk are destroyed within the mmu delete the next piece of code
	//delete
	mm_t* mm = mmu->mem;
	disk_t* disk = mmu->disk;
	mm_destroy(mm);
	disk_destroy(disk);
	//end delete

	//TODO NOTE: if mmu is destroyed within the process container delete the next line
	mmu_destroy(mmu);

	proc_cont_destroy(proc_cont);
	//TODO if any more fields are added to the app_data struct, don't forget to handle here

	free(app_data);
}

int create_process(app_data_t* app_data)
{
	int pid = -1;

	pid = init_process(APP_DATA_PROC_CONT(app_data));

	if (pid < 0)
	{
		printf("ERR: A process could not be created: error-%d\n", pid);
	}
	else
	{
		printf("Process with id %u was created\n", pid);
	}

	return pid;
}

void del_process(app_data_t* app_data, procid_t pid)
{
	errcode_t err = process_destroy(APP_DATA_PROC_CONT(app_data), pid);

	if(err == ecFail)
	{
		printf("Process id %u is not within range", pid);
	}
	else if (err == ecNotFound)
	{
		printf("Process %u was not found", pid);
	}

	//we just deal with error printing here
	//no need for success print
}

void sim_read(proc_cont_t* proc_cont, int vaddr, int id, int off,int amount, char* file_name)
{
	int page_size = PROC_CONT_MMU(proc_cont) -> mem -> page_size;
	virt_addr_t vAddr;
	FILE* f;
	errcode_t err = ecSuccess;

	BYTE* buf = (BYTE*)malloc((amount * off + 1) * sizeof(BYTE));
	int multiplier = 0;
	int i;

	//TODO check correctness
	vAddr.page = vaddr;
	vAddr.pid = id;

	assert(off > 0);
	assert(id >= 0);
	assert(amount > 0);

	while (multiplier < amount - 1 && off * multiplier < page_size && err == ecSuccess)
	{
		err = mmu_read(PROC_CONT_MMU(proc_cont), vAddr, off * multiplier, 1, &buf[multiplier]); //TODO check correctness
		++multiplier;
	}

	if (err == ecSuccess)
	{
		if (file_name == NULL)
			f = stdout;
		else
			f = fopen(file_name, "w+"); //TODO maybe a different mode

		for (i = 0; i < multiplier; ++i)
		{
			fprintf(f, "Char %d: %c\n", i, buf[i]);
		}

		if (file_name != NULL)
			fclose(f);
	}
	free(buf);
}

void sim_write(proc_cont_t* proc_cont, int vaddr, int id, char* s, int off,int amount)
{
	//TODO implement
}
