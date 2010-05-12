/*
 * cunit.h
 *
 *  Created on: 12/05/2010
 *      Author: tom
 */

#ifndef CUNIT_H_
#define CUNIT_H_

typedef enum {ceSuccess, ceFail} cunit_err_t;

typedef cunit_err_t (*cunit_test_t)();

#define ASSERT_TRUE(_x) if (!(_x)) {printf("[%s:%d] Assertion failed: `%s' in function %s\n", __FILE__, __LINE__, #_x, __func__); return ceFail;}
#define ASSERT_EQUALS(_x, _y) ASSERT_TRUE((_x) == (_y));

#define ADD_TEST(_testfunc) cunit_add_test(_testfunc, #_testfunc)
cunit_err_t cunit_add_test(cunit_test_t test, const char* name);
cunit_err_t cunit_run_suite();

#endif /* CUNIT_H_ */