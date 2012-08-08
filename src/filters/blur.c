
#include "blur.h"

static mcl_ex_t flt_get_expression_blur(heap_t h, filter_t f, mcl_ex_t point, filter_arguments_t fa) {
	mcl_ex_t n = mcl_var(h, mcl_random(h, 2));
	cl_float mulc = -2.0f;
	mcl_ex_t r = mcl_var(h, mcl_call_2_cs(h, "*",
			filter_get_expression(h, (filter_t) map_get_cs(f->arguments, "radius"), point, fa),
			mcl_call_1_cs(h, "sqrt",
				mcl_call_2_cs(h, "*",
					mcl_const(h, MCLT_FLOAT, &mulc),
					mcl_call_1_cs(h, "log",
						mcl_vector_lo(h, n))))));
	cl_float2 sc = {0, 0};
	mcl_ex_t sc_var = mcl_var(h, mcl_const(h, MCLT_FLOAT_2, &sc));
	mcl_ex_t new_point = mcl_var(h, mcl_call_2_cs(h, "+", point,
		mcl_call_2_cs(h, "*", r,
			mcl_init_ex(h,
				mcl_call_2_cs(h, "=", mcl_vector_lo(h, sc_var),
					mcl_call_2_cs(h, "sincos",
						mcl_call_2_cs(h, "*",
							mcl_const_2pi(),
							mcl_vector_hi(h, n)),
						mcl_vector_hi(h, sc_var))),
				sc_var))));
	return filter_get_expression(h, (filter_t) map_get_cs(f->arguments, "_"), new_point, fa);
}

filter_controller_s flt_controller_blur = {
	"blur", 0, 0, flt_get_expression_blur
};
