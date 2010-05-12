#ifndef VMSIM_TYPES_H_
#define VMSIM_TYPES_H_

#define TRUE 1
#define FALSE 0
typedef int BOOL;

typedef unsigned char BYTE;

typedef enum {ecSuccess, ecFail, ecNotFound} errcode_t;

typedef unsigned procid_t;

typedef struct
{
	procid_t pid;
	unsigned page;
}virt_addr_t; //virtual(logical) address

typedef struct
{
	unsigned page;
	unsigned offset;
}disk_addr_t; //disk address;

typedef unsigned phys_addr_t; //physical address

typedef struct
{
	disk_addr_t disk_block_start;
	unsigned disk_block_npages;
}process_t;

#endif /* VMSIM_TYPES_H_ */
