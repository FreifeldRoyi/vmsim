#ifndef VMSIM_TYPES_H_
#define VMSIM_TYPES_H_

/**General utilities and data types for the VM sim project.*/

//error code for vmsim functions.
typedef enum {ecSuccess, ecFail, ecNotFound} errcode_t;

//convert a posix return code to a vmsim error code
#define POSIX_ERRCODE(_retcode) (((_retcode) == 0)?ecSuccess:ecFail)

//get the array size of a _statically allocated_ array
//THIS DOES NOT WORK FOR POINTERS!
#define ARRSIZE(_arr) (sizeof(_arr)/sizeof(_arr[0]))

//set the MSB of a value.
#define SET_MSB(_var, _bit)  (_var) |= ((_bit)<<((sizeof(_var)*8)-1))

//boolean implementation
#define TRUE 1
#define FALSE 0
typedef int BOOL;

typedef unsigned char BYTE;
#define BYTE_SIZE (sizeof(BYTE) * 8)
#define UNSIGNED_SIZE (sizeof(unsigned) * 8)

//a process ID
typedef unsigned procid_t;

//a virtual page.
typedef struct
{
	procid_t pid;
	unsigned page;
	unsigned offset;
}virt_addr_t;

//accessors for a virtual address
#define VIRT_ADDR_PID(_v) (_v).pid
#define VIRT_ADDR_PAGE(_v) (_v).page
#define VIRT_ADDR_OFFSET(_v) (_v).offset

//check vaddress equality
#define VIRT_ADDR_PAGE_EQ(_v1, _v2) ((VIRT_ADDR_PID(_v1) == VIRT_ADDR_PID(_v2)) && (VIRT_ADDR_PAGE(_v1) == VIRT_ADDR_PAGE(_v2)))

//an address on the disk
typedef struct
{
	unsigned page;
	unsigned offset;
}disk_addr_t; //disk address;

//a physical address in the main memory
typedef unsigned phys_addr_t; //physical address

#endif /* VMSIM_TYPES_H_ */
