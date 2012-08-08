
#include <may/maylib.h>
#include <may/lib/json.h>
#include <may/lib/stream.h>
#include <may/lib/test.h>



#include "units/heap.h"
#include "units/str.h"
#include "units/map.h"
#include "units/parser.h"
#include "units/stream.h"
#include "units/json.h"
#include "units/tar.h"
#include "units/mcl/types.h"
#include "units/mcl/program.h"
#include "units/mcl/image.h"


int main() {
	maylib_init();
	test_str();
	test_map();
	test_parser();
	test_json();
	test_tar();
	test_mcl_types();
	test_mcl_program();
	test_mcl_image();
	fprintf(stderr, "Results: (%i/%i) %i%%\n", (int) tests_success, (int) (tests_success + tests_failed), (int) (tests_success*100/(tests_success + tests_failed)));
	return 0;
}
