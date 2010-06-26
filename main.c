#include "cunit/cunit.h"
#include "vmsim/ui_app.h"
#include "util/logger.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#ifdef VMSIM_RUN_TESTS

#pragma GCC diagnostic ignored "-Wimplicit-function-declaration"


int main(int argc, char* argv[])
{
	if (argc > 1)
	{
		if (!strcmp(argv[1], "debug"))
		{
			log_set_level(lvDebug);
		}
		if (!strcmp(argv[1], "info"))
		{
			log_set_level(lvInfo);
		}
	}
	add_queue_tests();
	add_bitmap_tests();
	add_ipt_tests();
	add_worker_thread_tests();
	add_disk_tests();
	add_map_tests();
	add_mmu_tests();
	add_app_util_tests();

	cunit_run_suite();
	cunit_destroy();
	return 0;
}


#else

int main(int argc, char** argv)
{
	printf("Program name: %s\nFile name: %s\n",argv[0], argv[1]);

	app_main(argc,argv);

	return 0;
}

#endif
