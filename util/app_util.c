/*
 * ui_util.c
 *
 *  Created on: May 19, 2010
 *      Author: Freifeld Royi
 */

#include "app_util.h"
#include <string.h>
#include <malloc.h>

static void print_ipt_entry(ipt_entry_t entry)
{
	if (!(entry.valid))
	{
		procid_t pid = entry.addr.pid;
		unsigned page_num = entry.addr.page;
		BOOL dirty = entry.dirty;
		BOOL aging_ref = entry.referenced;
		int next = entry.next;
		int prev = entry.prev;

		printf("(%u, %u, %d, %d, %d, %d)\n",pid,page_num,dirty,aging_ref,next,prev);
	}
	else
	{
		printf("(free)\n");
	}
}

static void print_BYTE(BYTE* byte)
{
	//TODO what to print here???
	printf("\n");
}

static void print_bitmap(bitmap_t* bitmap)
{
	//TODO what to print here???
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
	int pcb_size = MM_PCB_SIZE(&mm); //TODO not sure if needed

	printf("DATA: ");
	print_BYTE(MM_DATA(&mm));

	printf("NUM OF PAGES: %d\n", num_of_pages);
	printf("PAGE SIZE: %d\n", page_size);
	printf("PCB SIZE: %d\n",pcb_size); //TODO not sure if needed

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

void load_app_data(char* file_name)
{

}

void free_app_data(app_data_t* app_data)
{
	//TODO implement
}
