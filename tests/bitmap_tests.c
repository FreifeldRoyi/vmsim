#include "cunit/cunit.h"

static const int BITMAP_SIZE = 33;

cunit_err_t test_bitmap_init()
{
	bitmap_t bitmap;
	int i;

	ASSERT_EQUALS(ecSuccess, bitmap_init(&bitmap, BITMAP_SIZE));
	for (i=0; i< BITMAP_SIZE; ++i)
	{
		ASSERT_EQUALS(FALSE, bitmap_get(&bitmap, i));
	}

	bitmap_destroy(&bitmap);
	return ceSuccess;
}

cunit_err_t test_bitmap_set_clear()
{
	bitmap_t bitmap;
	int step, idx;

	bitmap_init(&bitmap, BITMAP_SIZE);

	for (step = 2; step < 16; ++step){
		for (idx = 0; idx < BITMAP_SIZE; idx += step)
		{
			ASSERT_EQUALS(FALSE, bitmap_get(&bitmap, idx));
			ASSERT_EQUALS(ecSuccess, bitmap_set(&bitmap,idx));
			ASSERT_EQUALS(TRUE, bitmap_get(&bitmap, idx));
			ASSERT_EQUALS(ecSuccess, bitmap_clear(&bitmap,idx));
			ASSERT_EQUALS(FALSE, bitmap_get(&bitmap, idx));
		}
	}
	bitmap_destroy(&bitmap);
	return ceSuccess;
}

cunit_err_t test_bitmap_first_set()
{
	bitmap_t bitmap;
	int step, idx;
	int result;

	bitmap_init(&bitmap, BITMAP_SIZE);

	for (step = 16; step >= 2 ; --step){
		for (idx = step; idx < BITMAP_SIZE; idx += step)
		{
			ASSERT_EQUALS(ecSuccess, bitmap_set(&bitmap,idx));
		}
		ASSERT_EQUALS(ecSuccess, bitmap_first_set(&bitmap, &result));
		ASSERT_EQUALS(step, result);
	}
	bitmap_destroy(&bitmap);
	return ceSuccess;
}

cunit_err_t test_bitmap_first_clear()
{
	bitmap_t bitmap;
	int step, idx;
	int result;

	bitmap_init(&bitmap, BITMAP_SIZE);

	for (idx = 0; idx < BITMAP_SIZE; ++idx)
	{
		ASSERT_EQUALS(ecSuccess, bitmap_set(&bitmap,idx));
	}

	for (step = 16; step >= 2 ; --step){
		for (idx = step; idx < BITMAP_SIZE; idx += step)
		{
			ASSERT_EQUALS(ecSuccess, bitmap_clear(&bitmap,idx));
		}
		ASSERT_EQUALS(ecSuccess, bitmap_first_clear(&bitmap, &result));
		ASSERT_EQUALS(step, result);
	}

	bitmap_destroy(&bitmap);
	return ceSuccess;
}

void add_bitmap_tests()
{
	ADD_TEST(test_bitmap_init);
	ADD_TEST(test_bitmap_set_clear);
	ADD_TEST(test_bitmap_first_set);
	ADD_TEST(test_bitmap_first_clear);
}
