#include "cunit.h"
#include "util/queue.h"
#include "util/vmsim_types.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

typedef struct _cunit_suite_t
{
	struct _queue_t* tests;
	int passed;
	int failed;
}cunit_suite_t;

typedef struct _cunit_test_rec_t
{
	cunit_test_t test;
	const char* name;
	int idx;
}cunit_test_rec_t;

static cunit_suite_t g_suite = {NULL,0,0};

cunit_err_t cunit_add_test(cunit_test_t test, const char* name)
{
	cunit_test_rec_t* test_rec = malloc(sizeof(cunit_test_rec_t));

	if (g_suite.tests == NULL)
	{
		g_suite.tests = queue_init();
	}

	test_rec->test = test;
	test_rec->name = name;
	test_rec->idx = queue_size(g_suite.tests)+1;

	queue_push(g_suite.tests, test_rec);

	return ceSuccess;
}

static void
run_single_test(void* param)
{
	cunit_test_rec_t* test = param;
	cunit_err_t retcode = test->test();

	printf("%d. %s - ", test->idx, test->name);

	switch (retcode)
	{
	case ceSuccess:	{
			printf("PASS\n");
			++g_suite.passed;
		}break;
	case ceFail: {
			printf("FAIL\n");
			++g_suite.failed;
		}break;
	default:
		assert(FALSE);
	}

}

cunit_err_t cunit_run_suite()
{
	if (g_suite.tests == NULL)
	{
		return ceSuccess;
	}
	g_suite.passed = g_suite.failed = 0;
	queue_for_each(g_suite.tests, run_single_test);
	printf("Passed: %d/%d\n", g_suite.passed, g_suite.passed + g_suite.failed);
	return ceSuccess;
}
