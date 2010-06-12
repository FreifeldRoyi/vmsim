#include "cunit/cunit.h"

BOOL int_comparator(void* k1, void* k2)
{
	return (*(int*)k1) - (* (int*)k2);
}

cunit_err_t test_map_set_get_sanity()
{
	map_t map;
	int key = 0xAA, value = 0xDD;

	ASSERT_EQUALS(ecSuccess, map_init(&map, sizeof(int), sizeof(int), int_comparator));

	ASSERT_EQUALS(ecNotFound, map_get(&map, &key, &value));
	ASSERT_EQUALS(ecSuccess, map_set(&map, &key, &value));
	value = 0;
	ASSERT_EQUALS(ecSuccess, map_get(&map, &key, &value));
	ASSERT_EQUALS(0xDD, value);

	map_destroy(&map);

	return ceSuccess;
}

cunit_err_t test_map_remove_sanity()
{
	map_t map;
	int key = 0xAA, value = 0xDD;

	ASSERT_EQUALS(ecSuccess, map_init(&map, sizeof(int), sizeof(int), int_comparator));

	ASSERT_EQUALS(ecSuccess, map_set(&map, &key, &value));
	ASSERT_EQUALS(ecSuccess, map_remove(&map, &key));
	ASSERT_EQUALS(ecNotFound, map_get(&map, &key, &value));

	map_destroy(&map);

	return ceSuccess;
}

void add_map_tests()
{
	ADD_TEST(test_map_set_get_sanity);
	ADD_TEST(test_map_remove_sanity);
}
