#ifndef BITMAP_H_
#define BITMAP_H_

#include "util/vmsim_types.h"

/**Bitmap ADT.*/

/**The bitmap stores it's bits in blocks of the following type.
 * This type must always be unsigned. changing this type will
 * change the performance characteristics of the bitmap.
 * */
typedef BYTE bitmap_block_t;

typedef struct
{
	bitmap_block_t* data;
	int size; //number of BITS in the bitmap
}bitmap_t;

/**Initialize a bitmap.
 * All the bits in the bitmap are initialized to 0.
 * This function must be called before any of the other functions are called.
 * @param bitmap the bitmap to initialize.
 * @param nbits the number of bits to store in the bitmap.
 *
 * @return ecSuccess on success, some other code on failure.
 * */
errcode_t bitmap_init(bitmap_t* bitmap, int nbits);

/**Find the index of the first '1' bit in the bitmap.
 * @param bitmap the bitmap to use
 * @param idx a pointer to an int that will receive the index.
 * @return ecSuccess on success, ecNotFound if all the bits in the bitmap are '0'
 * */
errcode_t bitmap_first_set(bitmap_t* bitmap, int* idx);

/**Find the index of the first '0' bit in the bitmap.
 * @param bitmap the bitmap to use
 * @param idx a pointer to an int that will receive the index.
 * @return ecSuccess on success, ecNotFound if all the bits in the bitmap are '1'
 * */
errcode_t bitmap_first_clear(bitmap_t* bitmap, int* idx);

/**Set a bit in the bitmap to '1'.
 * idx is assumed to be a valid index.
 * @param bitmap the bitmap to use
 * @param idx the index of the bit to change
 * @return ecSuccess on success, some other code on failure.
 * */
errcode_t bitmap_set(bitmap_t* bitmap, int idx);

/**Set a bit in the bitmap to '0'.
 * idx is assumed to be a valid index.
 * @param bitmap the bitmap to use
 * @param idx the index of the bit to change
 * @return ecSuccess on success, some other code on failure.
 * */
errcode_t bitmap_clear(bitmap_t* bitmap, int idx);

/**Get the value of a given bit in the bitmap
 * idx is assumed to be a valid index.
 * @param bitmap the bitmap to use
 * @param idx the index of the bit to change
 *
 * @return TRUE if the bit at the given index is '1', FALSE if it's '0'.
 * */
BOOL bitmap_get(bitmap_t* bitmap, int idx);


/**Finalize a bitmap. No other functions may be called on the bitmap after
 * this function was called on it, except bitmap_init.
 * @param bitmap the bitmap to finalize
 * */
void bitmap_destroy(bitmap_t* bitmap);

#endif /* BITMAP_H_ */
