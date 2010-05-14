#include "cunit/cunit.h"

int main()
{
	add_queue_tests();
	add_bitmap_tests();
	add_ipt_tests();
	add_worker_thread_tests();
	add_disk_tests();

	cunit_run_suite();
	cunit_destroy();
	return 0;
}
