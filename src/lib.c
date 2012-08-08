
#include "lib.h"
#include <may/core/lib.h>
#include "mcl.h"
#include "ex.h"
#include "image.h"
#include "filter.h"
#include <stdbool.h>

static bool initialized = false;

void may_cl_init() {
	if(!initialized) {
		mclt_init();
		mcl_init();
		mcl_image_init();
		filter_init();
		initialized = true;
	}
}



