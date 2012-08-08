
#include "maylib.h"
#include "lib/json.h"
#include "cl/mcl.h"
#include "cl/ex.h"
#include "cl/image.h"
#include "cl/filter.h"
#include <stdbool.h>

static bool initialized = false;

void maylib_init() {
	if(!initialized) {
		mem_init();
		json_init();
		mclt_init();
		mcl_init();
		mcl_image_init();
		filter_init();
		initialized = true;
	}
}



