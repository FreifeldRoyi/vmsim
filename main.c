#include "cunit/cunit.h"
#include "util/app_util.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/**int main()
{
	add_queue_tests();
	add_bitmap_tests();
	add_ipt_tests();
	add_worker_thread_tests();
	add_disk_tests();

	cunit_run_suite();
	cunit_destroy();
	return 0;
}*/

static char* new_alloc_strcat(char* dest, char* src)
{
	char* to_return = calloc(0,sizeof(dest) + sizeof(src) + 1);

	int i = 0, dest_len = strlen(dest), src_len = strlen(src);
	for (i = 0; i < dest_len; ++i)
	{
		to_return[i] = dest[i];
	}
	for (i = 0; i < src_len; ++i)
	{
		to_return[i + dest_len] = src[i];
	}
	to_return[dest_len + src_len + 1] = '\0';

	return to_return;
}

int main()
{
	//print_prompt();
	char* s = "hello ";
	char* s2 = "world\n";
	char* s3 = new_alloc_strcat(s,s2);
	printf("%s",s3);

	return 0;
}


