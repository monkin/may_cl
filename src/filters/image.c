
#include "image.h"

static void flt_init_image(filter_t f) {
	
}
static void flt_destroy_image(filter_t f) {
	
}
static mcl_ex_t flt_get_expression_image(heap_t h, filter_t f, mcl_ex_t point, filter_arguments_t args) {
	
}

filter_controller_s flt_controller_image = { "image", flt_init_image, flt_destroy_image, flt_get_expression_image };