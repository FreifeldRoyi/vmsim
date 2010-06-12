/*
 * ui_util.c
 *
 *  Created on: May 19, 2010
 *      Author: Freifeld Royi
 */

#include "app_util.h"
#include "sim/prm.h"
#include "sim/aging_daemon.h"
#include "logger.h"
#include <string.h>
#include <malloc.h>
#include <assert.h>

static unsigned page_shift(int page_size)
{
	unsigned shift = 0;
	while (page_size > 1)
	{
		++shift;
		page_size>>=1;
	}
	return shift;
}

static unsigned page_mask(int page_size)
{
	unsigned mask = 0xFFFFFFFF;
	int i;
	for (i=0; i<page_shift(page_size) ;++i)
	{
		mask <<= 1;
	}
	return mask;
}

static unsigned offset_mask(int page_size)
{
	return ~page_mask(page_size);
}

static int page_from_address(int page_size, int address)
{
	unsigned mask = page_mask(page_size),
			 shift = page_shift(page_size);
	return (address & mask)>>shift;
}

static int offset_from_address(int page_size, int address)
{
	unsigned mask = offset_mask(page_size);
	return address & mask;
}

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

	for (i = BYTE_SIZE - 1; i >= 0; --i)
	{
		if ((1 << i) & (*byte))
		{
			printf("1");
		}
		else
		{
			printf("0");
		}
	}
}

static void print_BYTE_hexa(BYTE* byte)
{
	BYTE print = *byte;
	printf("0x%x", print);
}

static void print_BYTE_int(BYTE* byte)
{
	BYTE print = *byte;
	printf("%d", print);
}

static void print_BYTE_char(BYTE* byte)
{
	printf("%c",(*byte));
}

void print_MMU_table(ipt_t* table)
{
	int i,tbl_size = table->size;
	ipt_entry_t *entries = table->entries;
	printf("NOTE\n----\nentries will be printed as follows: (pid, page_num, dirty_bit, aging_reference_bit, next, prev)"
			"\nfor free (unused) pages \"(free)\" will be printed\n");

	for (i = 0; i < tbl_size; ++i)
	{
		print_ipt_entry(entries[i]);
	}
}

void print_MM(mm_t* mm)
{
	int i = 0;
	int num_of_pages = MM_NUM_OF_PAGES(mm);
	int	page_size = MM_PAGE_SIZE(mm);

	printf("\nMain Memory\n-----------\n\n");
	printf("DATA:\n\n");
	printf("Binary     Hex    Char   Integer\n");
	printf("--------   ----   ----   -------\n");
	for (i = 0; i < num_of_pages; ++i)
	{
		print_BYTE_binary(&(MM_DATA(mm))[i]);
		printf("   ");
		print_BYTE_hexa(&(MM_DATA(mm))[i]);
		printf("   ");
		print_BYTE_char(&(MM_DATA(mm))[i]);
		printf("      ");
		print_BYTE_int(&(MM_DATA(mm))[i]);
		printf("\n");
	}

	printf("\nNUM OF PAGES: %d\n\nPAGE_SIZE: %d\n\n", num_of_pages,page_size);
}

void print_hit_rate(mmu_t* mmu)
{
	mmu_stats_t stats = mmu_get_stats(mmu);
	float hitrate = (float)stats.hits/ (float)stats.nrefs;
	assert(hitrate <= 1.0f);
	assert(hitrate >= 0.0f);
	printf("Hitrate is %f\n", hitrate);
}

void monitor_on()
{
	log_set_level(lvInfo);
	INFO("Logger level was set to Info\n");
}

void monitor_off()
{
	INFO("Logger level will now set to Error");
	log_set_level(lvError);
}

void debug_on()
{
	log_set_level(lvDebug);
	INFO("Logger level was set to Debug\n");
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
	disk_t* disk = (disk_t*)malloc(sizeof(disk_t));
	mmu_t* mmu = (mmu_t*)malloc(sizeof(mmu_t));

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

	disk_init(disk, n_page_disk, page_size,  n_proc_pages);
	mm_init(mm, n_page_mm, page_size);
	mmu_init(mmu, mm, disk, shift_clock);

	APP_DATA_PROC_CONT(app_data) = init_proc_cont(nproc, n_proc_pages, mmu);
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

	prm_destroy();
	aging_daemon_stop();

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
	int page_size = PROC_CONT_MMU(proc_cont)->mem->page_size;

	assert(args != NULL);
	assert(vAddr != NULL);
	assert(amnt != NULL);

	vAddr -> offset = offset_from_address(page_size, vaddr);
	vAddr -> page = page_from_address(page_size, vaddr);
	vAddr -> pid = id;
	(*amnt) = amount;

	args[0] = vAddr;
	args[1] = amnt;

	post = create_post(fcRead, args, 2);
	assert(post != NULL);

	err = compose_mail(proc_cont, id, post);
	if (err != ecSuccess)
	{
		printf("ERROR: couldn't send message\n");
		post_destroy(post);
	}
}

void loop_read_process(proc_cont_t* proc_cont, int vaddr, int id, int off, int amount)
{
	errcode_t err;
	post_t* post;
	void** args = (void**)malloc(3 * sizeof (void*));
	virt_addr_t* vAddr = (virt_addr_t*)malloc(sizeof(virt_addr_t));
	int* offset = (int*)malloc(sizeof(int));
	int* amnt = (int*)malloc(sizeof(int));
	int page_size = PROC_CONT_MMU(proc_cont)->mem->page_size;

	assert(args != NULL);
	assert(vAddr != NULL);
	assert(offset != NULL);
	assert(amnt != NULL);

	vAddr -> offset = offset_from_address(page_size, vaddr);
	vAddr -> page = page_from_address(page_size, vaddr);
	vAddr -> pid = id;
	(*offset) = off;
	(*amnt) = amount;

	args[0] = vAddr;
	args[1] = offset;
	args[2] = amnt;

	post = create_post(fcLoopRead, args, 3);
	assert(post != NULL);

	err = compose_mail(proc_cont, id, post);
	if (err != ecSuccess)
	{
		printf("ERROR: couldn't send message");
		post_destroy(post);
	}
}

void read_to_file_process(proc_cont_t* proc_cont, int vaddr, int id, int amount, char* file_name)
{
	errcode_t err;
	post_t* post;
	void** args = (void**)malloc(3 * sizeof (void*));
	virt_addr_t* vAddr = (virt_addr_t*)malloc(sizeof(virt_addr_t));
	int* amnt = (int*)malloc(sizeof(int));
	char* f_name = (char*)calloc(strlen(file_name) + 1, sizeof(char));
	int page_size = PROC_CONT_MMU(proc_cont)->mem->page_size;

	assert(args != NULL);
	assert(vAddr != NULL);
	assert(amnt != NULL);
	assert(file_name != NULL);
	assert(f_name != NULL);

	vAddr -> offset = offset_from_address(page_size, vaddr);
	vAddr -> page = page_from_address(page_size, vaddr);
	vAddr -> pid = id;
	(*amnt) = amount;
	f_name = strcpy(f_name, file_name);

	args[0] = vAddr;
	args[1] = amnt;
	args[2] = f_name;

	post = create_post(fcReadToFile, args, 3);
	assert(post != NULL);

	err = compose_mail(proc_cont, id, post);
	if (err != ecSuccess)
	{
		printf("ERROR: couldn't send message");
		post_destroy(post);
	}
}

void loop_read_to_file_process(proc_cont_t* proc_cont, int vaddr, int id, int off, int amount, char* file_name)
{
	errcode_t err;
	post_t* post;
	void** args = (void**)malloc(4 * sizeof (void*));
	virt_addr_t* vAddr = (virt_addr_t*)malloc(sizeof(virt_addr_t));
	int* offset = (int*)malloc(sizeof(int));
	int* amnt = (int*)malloc(sizeof(int));
	char* f_name = (char*)calloc(strlen(file_name) + 1, sizeof(char));

	assert(args != NULL);
	assert(vAddr != NULL);
	assert(offset != NULL);
	assert(amnt != NULL);
	assert(file_name != NULL);
	assert(f_name != NULL);

	vAddr -> page = vaddr;
	vAddr -> pid = id;
	(*offset) = off;
	(*amnt) = amount;
	f_name = strcpy(f_name, file_name);

	args[0] = vAddr;
	args[1] = offset;
	args[2] = amnt;
	args[3] = f_name;

	post = create_post(fcLoopReadToFile, args, 4);
	assert(post != NULL);

	err = compose_mail(proc_cont, id, post);
	if (err != ecSuccess)
	{
		printf("ERROR: couldn't send message");
		post_destroy(post);
	}
}

void write_process(proc_cont_t* proc_cont, int vaddr, int id, char* s)
{
	errcode_t err;
	post_t* post;
	void** args = (void**)malloc(3 * sizeof (void*));
	virt_addr_t* vAddr = (virt_addr_t*)malloc(sizeof(virt_addr_t));
	int* amnt = (int*)malloc(sizeof(int));
	char* st = (char*)calloc(strlen(s) + 1, sizeof(char));
	int page_size = PROC_CONT_MMU(proc_cont)->mem->page_size;

	assert(args != NULL);
	assert(vAddr != NULL);
	assert(st != NULL);
	assert(amnt != NULL);

	vAddr -> offset = offset_from_address(page_size, vaddr);
	vAddr -> page = page_from_address(page_size, vaddr);
	vAddr -> pid = id;
	(*amnt) = strlen(s);
	st = strcpy(st,s);

	args[0] = vAddr;
	args[1] = st;
	args[2] = amnt;

	post = create_post(fcWrite, args, 3);
	assert(post != NULL);

	err = compose_mail(proc_cont, id, post);
	if (err != ecSuccess)
	{
		printf("ERROR: couldn't send message");
		post_destroy(post);
	}
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
	int page_size = PROC_CONT_MMU(proc_cont)->mem->page_size;

	assert(args != NULL);
	assert(vAddr != NULL);
	assert(ch != NULL);
	assert(offset != NULL);
	assert(amnt != NULL);

	vAddr -> offset = offset_from_address(page_size, vaddr);
	vAddr -> page = page_from_address(page_size, vaddr);
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

	err = compose_mail(proc_cont, id, post);
	if (err != ecSuccess)
	{
		printf("ERROR: couldn't send message");
		post_destroy(post);
	}
}

#include "tests/app_util_tests.c"
