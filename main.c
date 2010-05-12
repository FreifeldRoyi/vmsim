#include "cunit/cunit.h"

int main()
{
	add_queue_tests();
	add_bitmap_tests();
	add_disk_tests();
	add_ipt_tests();

	cunit_run_suite();
	cunit_destroy();
	return 0;
}
