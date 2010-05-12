#ifndef BITMAP_H_
#define BITMAP_H_

#include "util/vmsim_types.h"


typedef BYTE bitmap_block_t;

typedef struct
{
	bitmap_block_t* data;
	int size;
}bitmap_t;

errcode_t bitmap_init(bitmap_t* bitmap, int nbits);

errcode_t bitmap_first_set(bitmap_t* bitmap, int* idx);
errcode_t bitmap_first_clear(bitmap_t* bitmap, int* idx);

errcode_t bitmap_set(bitmap_t* bitmap, int idx);
errcode_t bitmap_clear(bitmap_t* bitmap, int idx);

BOOL bitmap_get(bitmap_t* bitmap, int idx);

void bitmap_destroy(bitmap_t* bitmap);

#endif /* BITMAP_H_ */
