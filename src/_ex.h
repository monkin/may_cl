
#ifndef MAY_MCLEX_H
#define MAY_MCLEX_H

#include "mcl.h"
#include <may/core/map.h>
#include <may/core/str.h>
#include <may/core/stream.h>
#include <CL/cl.h>

ERR_DECLARE(e_mcl_ex_invalid_operand);
ERR_DECLARE(e_mcl_ex_invalid_function);
ERR_DECLARE(e_mcl_ex_invalid_type);

struct mcl_ex_ss;
typedef struct mcl_ex_ss mcl_ex_s;
typedef mcl_ex_s *mcl_ex_t;

enum {
	MCL_EXC_CALL,
	MCL_EXC_VAR,
	MCL_EXC_ARG,
	MCL_EXC_CONST
};

typedef struct {
	str_t name;
	mclt_t type;
	cl_uint position;
} mcl_arg_s;

typedef mcl_arg_s *mcl_arg_t;


bool mcl_insert_ptr(map_t, void *);

typedef struct {
	void (*push_arguments)(void *, void (*push_fn)(void *, mcl_arg_t), void *dt);
	void (*global_source)(void *, map_t, ios_t);
	void (*local_source)(void *, map_t, ios_t);
	void (*value_source)(void *, ios_t);
} mcl_ex_vtable_s;

typedef mcl_ex_vtable_s *mcl_ex_vtable_t;

struct mcl_ex_ss {
	mclt_t return_type;
	mcl_ex_vtable_t vtable;
	void *data;
};

void mcl_init();

bool mcl_is_lvalue(mcl_ex_t);

mcl_ex_t mcl_call(heap_t h, str_t nm);
mcl_ex_t mcl_call_cs(heap_t h, const char *nm);
mcl_ex_t mcl_call_1(heap_t h, str_t nm, mcl_ex_t);
mcl_ex_t mcl_call_1_cs(heap_t h, const char *nm, mcl_ex_t);
mcl_ex_t mcl_call_2(heap_t h, str_t nm, mcl_ex_t, mcl_ex_t);
mcl_ex_t mcl_call_2_cs(heap_t h, const char *nm, mcl_ex_t, mcl_ex_t);
mcl_ex_t mcl_call_3(heap_t h, str_t nm, mcl_ex_t, mcl_ex_t, mcl_ex_t);
mcl_ex_t mcl_call_3_cs(heap_t h, const char *nm, mcl_ex_t, mcl_ex_t, mcl_ex_t);

mcl_ex_t mcl_var(heap_t h, mcl_ex_t);
mcl_ex_t mcl_arg(heap_t h, mclt_t tp, mcl_arg_s *);
mcl_ex_t mcl_seq(heap_t h, size_t n, ...);
mcl_ex_t mcl_init_ex(heap_t, mcl_ex_t init_ex, mcl_ex_t value_ex);
mcl_ex_t mcl_if(heap_t h, mcl_ex_t, mcl_ex_t, mcl_ex_t);
mcl_ex_t mcl_for(heap_t h, mcl_ex_t var_init, mcl_ex_t var_cond, mcl_ex_t var_inc, mcl_ex_t ex);
mcl_ex_t mcl_while(heap_t h, mcl_ex_t condition, mcl_ex_t expression);
mcl_ex_t mcl_cast(heap_t h, mclt_t, mcl_ex_t);

mcl_ex_t mcl_const(heap_t h, mclt_t tp, const void *val);
mcl_ex_t mcl_const_maxfloat();
mcl_ex_t mcl_const_hugeval();
mcl_ex_t mcl_const_infinity();
mcl_ex_t mcl_const_nan();
mcl_ex_t mcl_const_e();
mcl_ex_t mcl_const_log2e();
mcl_ex_t mcl_const_log10e();
mcl_ex_t mcl_const_ln2();
mcl_ex_t mcl_const_ln10();
mcl_ex_t mcl_const_pi();
mcl_ex_t mcl_const_pi_2();
mcl_ex_t mcl_const_pi_4();
mcl_ex_t mcl_const_1_pi();
mcl_ex_t mcl_const_2_pi();
mcl_ex_t mcl_const_2pi();
mcl_ex_t mcl_const_2_sqrtpi();
mcl_ex_t mcl_const_sqrt2();
mcl_ex_t mcl_const_1_sqrt2();

mcl_ex_t mcl_vector_select(heap_t h, mcl_ex_t ex, unsigned int rsz, const unsigned int *indexes);
mcl_ex_t mcl_vector_select_1(heap_t h, mcl_ex_t ex, unsigned int i);
mcl_ex_t mcl_vector_select_2(heap_t h, mcl_ex_t ex, unsigned int i0, unsigned int i1);
mcl_ex_t mcl_vector_select_4(heap_t h, mcl_ex_t ex, unsigned int i0, unsigned int i1, unsigned int i2, unsigned int i3);
mcl_ex_t mcl_vector_select_8(heap_t h, mcl_ex_t ex, unsigned int i0, unsigned int i1, unsigned int i2, unsigned int i3, unsigned int i4, unsigned int i5, unsigned int i6, unsigned int i7);
mcl_ex_t mcl_vector_lo(heap_t h, mcl_ex_t ex);
mcl_ex_t mcl_vector_hi(heap_t h, mcl_ex_t ex);
mcl_ex_t mcl_vector_odd(heap_t h, mcl_ex_t ex);
mcl_ex_t mcl_vector_even(heap_t h, mcl_ex_t ex);

mcl_ex_t mcl_random(heap_t h, size_t vector_size);



#endif /* MAY_MCLEX_H */

