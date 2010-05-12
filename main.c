#include "cunit/cunit.h"

cunit_err_t passing_test()
{
	return ceSuccess;
}

cunit_err_t failing_test()
{
	return ceFail;
}

int main()
{
	ADD_TEST(passing_test);
	ADD_TEST(failing_test);

	return cunit_run_suite();;
}
