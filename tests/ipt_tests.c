#include "cunit/cunit.h"

static const int NPAGES = 7;


cunit_err_t test_ipt_add_no_listwalk()
{
	ipt_t ipt;
	virt_addr_t addr[5];
	phys_addr_t paddr = 0xFFFFFFFF;
	int i;

	ASSERT_EQUALS(ecSuccess, ipt_init(&ipt, 10));

	for (i=0; i<ARRSIZE(addr); ++i)
	{
		VIRT_ADDR_PID(addr[i]) = 0;
		VIRT_ADDR_PAGE(addr[i]) = i;

		ASSERT_EQUALS(ecSuccess, ipt_add(&ipt, addr[i]));

		ASSERT_EQUALS(ecSuccess, ipt_translate(&ipt, addr[i], &paddr));

		ASSERT_EQUALS(i, paddr);
	}

	for (i=0; i<ARRSIZE(addr); ++i)
	{
		ASSERT_EQUALS(ecSuccess, ipt_remove(&ipt, addr[i]));
	}

	ipt_destroy(&ipt);

	return ceSuccess;
}

cunit_err_t test_ipt_multiple_add_remove()
{
	ipt_t ipt;
	virt_addr_t addr = {0,0};
	phys_addr_t paddr = 0xFFFFFFFF;
	int i;

	ASSERT_EQUALS(ecSuccess, ipt_init(&ipt, 10));

	for (i=0; i<NPAGES*5; ++i)
	{
		ASSERT_EQUALS(ecSuccess, ipt_add(&ipt, addr));
		ASSERT_EQUALS(ecSuccess, ipt_translate(&ipt, addr, &paddr));
		ASSERT_EQUALS(0, paddr);
		ASSERT_EQUALS(ecSuccess, ipt_remove(&ipt, addr));
	}

	ipt_destroy(&ipt);

	return ceSuccess;
}

cunit_err_t test_ipt_add_listwalk()
{
	ipt_t ipt;
	virt_addr_t addr[3] = {{0,0}, {NPAGES,0}, {0,NPAGES}};
	phys_addr_t paddr = 0xFFFFFFFF;
	int i;

	ASSERT_EQUALS(ecSuccess, ipt_init(&ipt, 10));

	for (i=0;i<ARRSIZE(addr); ++i)
	{
		ASSERT_EQUALS(ecSuccess, ipt_add(&ipt, addr[i]));
	}

	for (i=0;i<ARRSIZE(addr); ++i){
		ASSERT_EQUALS(ecSuccess, ipt_translate(&ipt, addr[i], &paddr));
		ASSERT_EQUALS(i, paddr);
	}

	ipt_destroy(&ipt);

	return ceSuccess;
}

cunit_err_t test_ipt_add_listwalk_and_others()
{
	ipt_t ipt;
	virt_addr_t addr[5] = {{0,0}, {1,0}, {NPAGES,0}, {0,3},{0,NPAGES}};
	phys_addr_t paddr = 0xFFFFFFFF;
	int i;

	ASSERT_EQUALS(ecSuccess, ipt_init(&ipt, 10));

	for (i=0;i<ARRSIZE(addr); ++i)
	{
		ASSERT_EQUALS(ecSuccess, ipt_add(&ipt, addr[i]));
	}

	for (i=0;i<ARRSIZE(addr); ++i){
		ASSERT_EQUALS(ecSuccess, ipt_translate(&ipt, addr[i], &paddr));
		ASSERT_EQUALS(i, paddr);
	}

	ipt_destroy(&ipt);

	return ceSuccess;
}

cunit_err_t test_ipt_remove_no_listwalk()
{
	ipt_t ipt;
	virt_addr_t addr = {0,0};

	ASSERT_EQUALS(ecSuccess, ipt_init(&ipt, 10));

	ASSERT_EQUALS(ecSuccess, ipt_add(&ipt, addr));
	ASSERT_TRUE(ipt_has_translation(&ipt, addr));

	ASSERT_EQUALS(ecSuccess, ipt_remove(&ipt, addr));

	ASSERT_TRUE(!ipt_has_translation(&ipt, addr))

	ipt_destroy(&ipt);

	return ceSuccess;
}

cunit_err_t test_ipt_remove_listwalk_and_others()
{
	ipt_t ipt;
	virt_addr_t addr[5] = {{0,0}, {1,0}, {NPAGES,0}, {0,3},{0,NPAGES}};
	int i;

	ASSERT_EQUALS(ecSuccess, ipt_init(&ipt, 10));

	for (i=0;i<ARRSIZE(addr); ++i)
	{
		ASSERT_EQUALS(ecSuccess, ipt_add(&ipt, addr[i]));
	}

	for (i=ARRSIZE(addr)-1; i>=0; --i)
	{
		ASSERT_EQUALS(ecSuccess, ipt_remove(&ipt, addr[i]));
	}

	for (i=0;i<ARRSIZE(addr); ++i){
		ASSERT_TRUE(!ipt_has_translation(&ipt, addr[i]));
	}

	ipt_destroy(&ipt);

	return ceSuccess;
}

cunit_err_t test_ipt_add_remove_generic()
{
	ipt_t ipt;
	virt_addr_t addr;
	int i;

	ASSERT_EQUALS(ecSuccess, ipt_init(&ipt, NPAGES));
	for (i=0;i<NPAGES; ++i)
	{
		VIRT_ADDR_PAGE(addr) = i % (NPAGES/3);
		VIRT_ADDR_PID(addr) = i / (NPAGES/3);
		ASSERT_EQUALS(ecSuccess, ipt_add(&ipt, addr));
	}

	for (i=0;i<NPAGES; ++i)
	{
		VIRT_ADDR_PAGE(addr) = i % (NPAGES/3);
		VIRT_ADDR_PID(addr) = i / (NPAGES/3);
		ASSERT_EQUALS(ecSuccess, ipt_remove(&ipt, addr));
	}
	ipt_destroy(&ipt);

	return ceSuccess;
}

void add_ipt_tests()
{
	ADD_TEST(test_ipt_add_no_listwalk);
	ADD_TEST(test_ipt_multiple_add_remove);
	ADD_TEST(test_ipt_add_listwalk);
	ADD_TEST(test_ipt_add_listwalk_and_others);
	ADD_TEST(test_ipt_remove_no_listwalk);
	ADD_TEST(test_ipt_remove_listwalk_and_others);
	ADD_TEST(test_ipt_add_remove_generic);
}
