#ifndef VMSIM_TYPES_H_
#define VMSIM_TYPES_H_

#define POSIX_ERRCODE(_retcode) (((_retcode) == 0)?ecSuccess:ecFail)

#define ARRSIZE(_arr) (sizeof(_arr)/sizeof(_arr[0]))

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

#define VIRT_ADDR_PID(_v) (_v).pid
#define VIRT_ADDR_PAGE(_v) (_v).page
#define VIRT_ADDR_EQ(_v1, _v2) ((VIRT_ADDR_PID(_v1) == VIRT_ADDR_PID(_v2)) && (VIRT_ADDR_PAGE(_v1) == VIRT_ADDR_PAGE(_v2)))

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
