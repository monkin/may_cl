
#include <may/cl/mcl.h>

void test_mcl_types() {
	TEST_MODULE("mcl_types");
	heap_t h = heap_create(0);
	err_try {
		TEST_CHECK("simple") {
			if(str_compare(mclt_name(MCLT_FLOAT), str_from_cs(h, "float"))!=0)
				TEST_FAIL;
		} TEST_END;
		TEST_CHECK("pointer") {
			if(str_compare(mclt_name(mclt_pointer(MCLT_ULONG, MCL_MEM_GLOBAL)), str_from_cs(h, "global ulong *"))!=0)
				TEST_FAIL;
		} TEST_END;
		TEST_CHECK("vector") {
			if(str_compare(mclt_name(mclt_vector(MCLT_USHORT, 2)), str_from_cs(h, "ushort2"))!=0)
				TEST_FAIL;
		} TEST_END;
		TEST_CHECK("pointer_to_vector") {
			if(str_compare(mclt_name(mclt_pointer(mclt_vector(MCLT_UCHAR, 8), MCL_MEM_PRIVATE)), str_from_cs(h, "private uchar8 *"))!=0)
				TEST_FAIL;
		} TEST_END;
		TEST_CHECK("parse_long") {
			if(mclt_parse_cs(" long")!=MCLT_LONG)
				TEST_FAIL;
		} TEST_END;
		TEST_CHECK("parse_pointer") {
			if(mclt_parse_cs("global uchar8\t* ")!=mclt_pointer(mclt_vector(MCLT_UCHAR, 8), MCL_MEM_GLOBAL))
				TEST_FAIL;
		} TEST_END;
		TEST_CHECK("parse_image") {
			if(mclt_parse_cs(" read_only image2d_t")!=MCLT_IMAGE_R)
				TEST_FAIL;
		} TEST_END;
		TEST_CHECK("type_size") {
			if(mclt_size(mclt_vector(MCLT_USHORT, 4))!=8)
				TEST_FAIL;
		} TEST_END;
		h = heap_delete(h);
	} err_catch {
		h = heap_delete(h);
	}
}
