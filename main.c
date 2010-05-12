#include "cunit/cunit.h"

int main()
{
	add_queue_tests();

	return cunit_run_suite();;
}
