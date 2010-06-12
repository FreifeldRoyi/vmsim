#include "cunit/cunit.h"

cunit_err_t test_address_splitting()
{
	int page_sizes[] 		= {2         ,32        ,1024       ,65536     ,65536*16  };
	unsigned page_shifts[] 	= {1         ,5         ,10         ,16        ,20        };
	unsigned page_masks[] 	= {0xFFFFFFFE,0xFFFFFFE0,0xFFFFFC00,0xFFFF0000,0xFFF00000};
	unsigned addresses[]	= {0x3       ,0x42      ,0xC03      ,0x40004   ,0x500005  };
	int i;

	for (i=0; i< ARRSIZE(page_sizes); ++i)
	{
		ASSERT_EQUALS(page_shifts[i], page_shift(page_sizes[i]));
		ASSERT_EQUALS(page_masks[i], page_mask(page_sizes[i]));
		ASSERT_EQUALS(i+1, page_from_address(page_sizes[i], addresses[i]));
		ASSERT_EQUALS(i+1, offset_from_address(page_sizes[i], addresses[i]));
	}

	return ceSuccess;
}

void add_app_util_tests()
{
	ADD_TEST(test_address_splitting);
}
