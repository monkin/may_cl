
#include <may/core/lib.h>
#include <may/core/json.h>
#include <may/core/stream.h>
#include <may/core/test.h>

#include "../src/mcl.h"
#include "../src/ex.h"
#include "../src/program.h"
#include "../src/error.h"
#include "../src/image.h"

#include "units/types.h"
#include "units/program.h"
#include "units/image.h"


int main() {
	may_core_init();
	may_cl_init();
	test_mcl_types();
	test_mcl_program();
	test_mcl_image();
	fprintf(stderr, "Results: (%i/%i) %i%%\n", (int) tests_success, (int) (tests_success + tests_failed), (int) (tests_success*100/(tests_success + tests_failed)));
	return 0;
}
