/*
 * ui_util.c
 *
 *  Created on: May 19, 2010
 *      Author: Freifeld Royi
 */

#include "app_util.h"
#include "sim/prm.h"
#include "sim/aging_daemon.h"
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
	int num_of_pages = MM_NUM_OF_PAGES(mm);
	int	page_size = MM_PAGE_SIZE(mm);

	printf("DATA:\n");
	print_BYTE_binary(MM_DATA(mm));

	printf("NUM OF PAGES: %d\n", num_of_pages);
	printf("PAGE SIZE: %d\n", page_size);

	//printf("BITMAP: ");
	//print_bitmap_binary(&MM_BITMAP(&mm));
}

BOOL load_app_data(char* file_name, app_data_t* app_data)
{
	errcode_t err;
	FILE* f;

	unsigned nproc;
	unsigned page_size;
	unsigned n_page_mm;
	unsigned n_page_disk;
	unsigned n_proc_pages;
	unsigned shift_clock;

	mm_t* mm = (mm_t*)malloc(sizeof(mm_t));
	disk_t* disk = (disk_t*)malloc(sizeof(disk_t));;
	mmu_t* mmu = (mmu_t*)malloc(sizeof(mmu_t));;

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

	disk_init(disk, n_page_disk, page_size,  n_proc_pages/* TODO block size maybe a different size??*/);
	mm_init(mm, n_page_mm, page_size);
	mmu_init(mmu, mm, disk, shift_clock);

	APP_DATA_PROC_CONT(app_data) = init_proc_cont(nproc, mmu);
	//TODO if any more fields are added to the app_data struct, don't forget to handle here
	err = prm_init(mmu);
	assert(err == ecSuccess);

	err = aging_daemon_start(mmu);
	assert(err == ecSuccess);

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
		printf("Process id %u is not within range\n", pid);
	}
	else if (err == ecNotFound)
	{
		printf("Process %u was not found\n", pid);
	}
	else
	{
		printf("Process %u was deleted\n", pid);
	}

	//we just deal with error printing here
	//no need for success print
}

void read_process(proc_cont_t* proc_cont, int vaddr, int id, int amount)
{
	errcode_t err;
	post_t* post;
	void** args = (void**)malloc(2 * sizeof (void*));
	virt_addr_t* vAddr = (virt_addr_t*)malloc(sizeof(virt_addr_t));
	int* amnt = (int*)malloc(sizeof(int));

	assert(args != NULL);
	assert(vAddr != NULL);
	assert(amnt != NULL);

	vAddr -> page = vaddr;
	vAddr -> pid = id;
	(*amnt) = amount;

	args[0] = vAddr;
	args[1] = amnt;

	post = create_post(fcRead, args, 2);
	assert(post != NULL);

	err = compose_mail(&PROC_CONT_SPEC_PROC(proc_cont, id),post);
	assert(err != ecFail);
}

void loop_read_process(proc_cont_t* proc_cont, int vaddr, int id, int off, int amount)
{
	errcode_t err;
	post_t* post;
	void** args = (void**)malloc(3 * sizeof (void*));
	virt_addr_t* vAddr = (virt_addr_t*)malloc(sizeof(virt_addr_t));
	int* offset = (int*)malloc(sizeof(int));
	int* amnt = (int*)malloc(sizeof(int));

	assert(args != NULL);
	assert(vAddr != NULL);
	assert(offset != NULL);
	assert(amnt != NULL);

	vAddr -> page = vaddr;
	vAddr -> pid = id;
	(*offset) = off;
	(*amnt) = amount;

	args[0] = vAddr;
	args[1] = offset;
	args[2] = amnt;

	post = create_post(fcLoopRead, args, 3);
	assert(post != NULL);

	err = compose_mail(&PROC_CONT_SPEC_PROC(proc_cont, id),post);
	assert(err != ecFail);
}

void read_to_file_process(proc_cont_t* proc_cont, int vaddr, int id, int amount, char* file_name)
{
	errcode_t err;
	post_t* post;
	void** args = (void**)malloc(3 * sizeof (void*));
	virt_addr_t* vAddr = (virt_addr_t*)malloc(sizeof(virt_addr_t));
	int* amnt = (int*)malloc(sizeof(int));

	assert(args != NULL);
	assert(vAddr != NULL);
	assert(amnt != NULL);
	assert(file_name != NULL);

	vAddr -> page = vaddr;
	vAddr -> pid = id;
	(*amnt) = amount;

	args[0] = vAddr;
	args[1] = amnt;
	args[2] = file_name; //TODO might be a problem....since file_name may be defined on stack

	post = create_post(fcReadToFile, args, 3);
	assert(post != NULL);

	err = compose_mail(&PROC_CONT_SPEC_PROC(proc_cont, id),post);
	assert(err != ecFail);
}

void loop_read_to_file_process(proc_cont_t* proc_cont, int vaddr, int id, int off, int amount, char* file_name)
{
	errcode_t err;
	post_t* post;
	void** args = (void**)malloc(4 * sizeof (void*));
	virt_addr_t* vAddr = (virt_addr_t*)malloc(sizeof(virt_addr_t));
	int* offset = (int*)malloc(sizeof(int));
	int* amnt = (int*)malloc(sizeof(int));

	assert(args != NULL);
	assert(vAddr != NULL);
	assert(offset != NULL);
	assert(amnt != NULL);
	assert(file_name != NULL);

	vAddr -> page = vaddr;
	vAddr -> pid = id;
	(*offset) = off;
	(*amnt) = amount;

	args[0] = vAddr;
	args[1] = offset;
	args[2] = amnt;
	args[3] = file_name; //TODO might be a problem....since file_name may be defined on stack

	post = create_post(fcLoopReadToFile, args, 4);
	assert(post != NULL);

	err = compose_mail(&PROC_CONT_SPEC_PROC(proc_cont, id),post);
	assert(err != ecFail);
}

void write_process(proc_cont_t* proc_cont, int vaddr, int id, char* s)
{
	errcode_t err;
	post_t* post;
	void** args = (void**)malloc(3 * sizeof (void*));
	virt_addr_t* vAddr = (virt_addr_t*)malloc(sizeof(virt_addr_t));
	int* amnt = (int*)malloc(sizeof(int));

	assert(args != NULL);
	assert(vAddr != NULL);
	assert(s != NULL);
	assert(amnt != NULL);

	vAddr -> page = vaddr;
	vAddr -> pid = id;
	(*amnt) = strlen(s); //TODO really??

	args[0] = vAddr;
	args[1] = s; //TODO might be a problem....since file_name may be defined on stack
	args[2] = amnt;

	post = create_post(fcWrite, args, 3);
	assert(post != NULL);

	err = compose_mail(&PROC_CONT_SPEC_PROC(proc_cont, id),post);
	assert(err != ecFail);
}

void loop_write_process(proc_cont_t* proc_cont, int vaddr, int id, char c, int off, int amount)
{
	errcode_t err;
	post_t* post;
	void** args = (void**)malloc(4 * sizeof (void*));
	virt_addr_t* vAddr = (virt_addr_t*)malloc(sizeof(virt_addr_t));
	char* ch = (char*)malloc(sizeof(char));
	int* offset = (int*)malloc(sizeof(int));
	int* amnt = (int*)malloc(sizeof(int));

	assert(args != NULL);
	assert(vAddr != NULL);
	assert(ch != NULL);
	assert(offset != NULL);
	assert(amnt != NULL);

	vAddr -> page = vaddr;
	vAddr -> pid = id;
	(*ch) = c;
	(*offset) = off;
	(*amnt) = amount;

	args[0] = vAddr;
	args[1] = ch;
	args[2] = offset;
	args[3] = amnt;

	post = create_post(fcLoopWrite, args, 4);
	assert(post != NULL);

	err = compose_mail(&PROC_CONT_SPEC_PROC(proc_cont, id),post);
	assert(err != ecFail);
}
