#include "bitmap.h"
#include <stdlib.h>
#include <assert.h>

#define BLOCK_SIZE (sizeof(bitmap_block_t)*8)

#define BLOCK_IDX(_bit) (_bit/BLOCK_SIZE)
#define BIT_IDX(_bit) (_bit % BLOCK_SIZE)

#define GET_BIT(_block, _bitidx) ((_block & ((bitmap_block_t)1<<_bitidx))?TRUE:FALSE)

#define SET_BIT(_block, _bitidx) (_block |= ((bitmap_block_t)1<<_bitidx))
#define CLEAR_BIT(_block, _bitidx) (_block &= ~((bitmap_block_t)1<<_bitidx))

errcode_t bitmap_init(bitmap_t* bitmap, int nbits)
{
	bitmap->data = calloc(1, ((nbits-1)/BLOCK_SIZE)+1);
	if (bitmap->data == NULL)
	{
		return ecFail;
	}
	bitmap->size = nbits;
	return ecSuccess;
}

static int
find_first(bitmap_t* bitmap, BOOL value, bitmap_block_t skip_block)
{
	unsigned blockidx = 0,
		bitidx = 0,
		maxbitidx;
	bitmap_block_t block;

	//first find the first non-skippable block
	while ( (bitmap->data[blockidx] == skip_block) &&
			(blockidx <= BLOCK_IDX(bitmap->size)))
	{
		++blockidx;
	}
	if (blockidx > BLOCK_IDX(bitmap->size))
		return -1;

	block = bitmap->data[blockidx];

	maxbitidx = bitmap->size - blockidx * BLOCK_SIZE;

	//now find the first relevant bit.
	while ((bitidx < BLOCK_SIZE) && (bitidx < maxbitidx))
	{
		if (GET_BIT(block, bitidx) == value)
		{
			return  blockidx*BLOCK_SIZE + bitidx;
		}
		++bitidx;
	}

	return -1;

}

errcode_t bitmap_first_set(bitmap_t* bitmap, int* idx)
{
	*idx = find_first(bitmap, TRUE, 0x00);
	return (*idx >= 0)? ecSuccess:ecNotFound;
}

errcode_t bitmap_first_clear(bitmap_t* bitmap, int* idx)
{
	*idx = find_first(bitmap, FALSE, 0xFF);
	return (*idx >= 0)? ecSuccess:ecNotFound;
}

errcode_t bitmap_set(bitmap_t* bitmap, int idx)
{
	bitmap_block_t *block;
	assert (idx < bitmap->size);

	block = &bitmap->data[BLOCK_IDX(idx)];
	SET_BIT(*block, BIT_IDX(idx));

	return ecSuccess;
}

errcode_t bitmap_clear(bitmap_t* bitmap, int idx)
{
	bitmap_block_t *block;
	assert (idx < bitmap->size);

	block = &bitmap->data[BLOCK_IDX(idx)];
	CLEAR_BIT(*block, BIT_IDX(idx));

	return ecSuccess;
}

BOOL bitmap_get(bitmap_t* bitmap, int idx)
{
	bitmap_block_t block;
	assert (idx < bitmap->size);

	block = bitmap->data[BLOCK_IDX(idx)];
	return GET_BIT(block, BIT_IDX(idx));
}

void bitmap_destroy(bitmap_t* bitmap)
{
	free(bitmap->data);
	bitmap->data = NULL;
}


#include "tests/bitmap_tests.c"
