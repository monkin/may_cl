
#ifndef MAY_CL_EX_H
#define MAY_CL_EX_H

#include "mcl.h"
#include <may/core/lib.h>
#include <stdbool.h>

ERR_DECLARE(e_mclex_error);

struct mclex_ss;
typedef struct mclex_ss mclex_s;
typedef mclex_s *mclex_t;

typedef struct mclex_block_ss {
	sb_t source;
	mclt_t var_type;
	sb_t var_name;
	struct mclex_block_ss *parent;
} mclex_block_s;

typedef mclex_block_s *mclex_block_t;

typedef struct mclex_program_ss {
	heap_t heap;
	mclex_block_t block;
	sb_t global_source;
	sb_t local_source;
	sb_t arguments_source;
	long arguments_count;
	str_t source;
	map_t global_flags;
	unsigned long counter; /* Used to generate variables names. */
	mclex_t v_items[16];
	int v_count;
} mclex_program_s;

typedef mclex_program_s *mclex_program_t;

extern __thread mclex_program_t mclex_program;

#define mclex_heap() (mclex_program->heap)
#define mclex_local_source() (mclex_program->local_source)
#define mclex_global_source() (mclex_program->global_source)
#define mclex_block() (mclex_program->block)
#define mclex_block_source() (mclex_block()->source)

void mclex_program_begin();
void mclex_program_reset(); /* Remove all current program data and stop building. */
mclex_program_t mclex_program_end();
mclex_program_t mclex_program_delete(mclex_program_t);

/* void *mclex_global_flag(str_t); */
#define mclex_global_get(f) (map_get(mclex_program->global_flags, (f)))
void *mclex_global_set(str_t, void *);

typedef struct mclex_ss {
	mclt_t type;
	sb_t source;
	long mem_type; /* Only for lvalues, in other cases should be 0 */
	long argument_index;
};

mclex_t mclex_ex(mclt_t);

void mclex_begin(); /* block */
mclex_block_t mclex_current_block();
/**
 * If block is null, current block used.
 */
void mclex_ret(mclex_block_t, mclex_t); /* set block result */
mclex_t mclex_end(mclex_t); /* ~block */

void mclex_if(mclex_t); /* block */
void mclex_unless(mclex_t); /* block */
void mclex_elsif(mclex_t);
void mclex_else();

void mclex_while(mclex_t); /* block */
/**
 * Gets two integer expressions and return iterator value.
 */
mclex_t mclex_for(mclex_t, mclex_t); /* block */

mclex_t mclex_cast(mclt_t, mclex_t);

mclex_t mclex_null(mclt_t);
mclex_t mclex_var(mclex_t);
mclex_t mclex_const(mclex_t);

mclex_t mclex_global_var(mclex_t);
mclex_t mclex_global_const(mclex_t);

mclex_t mclex_literal(mclt_t tp, const void *val);
mclex_t mclex_char(cl_char);
mclex_t mclex_uchar(cl_uchar);
mclex_t mclex_short(cl_short);
mclex_t mclex_ushort(cl_ushort);
mclex_t mclex_int(cl_int);
mclex_t mclex_uint(cl_uint);
mclex_t mclex_long(cl_long);
mclex_t mclex_ulong(cl_ulong);
mclex_t mclex_float(cl_float);


mclex_t mclex_array(mclt_t tp, size_t sz, const void *val);
mclex_t mclex_global_array(str_t name, mclt_t tp, size_t sz, const void *val);

mclex_t mclex_neg(mclex_t);
mclex_t mclex_add(mclex_t, mclex_t);
mclex_t mclex_sub(mclex_t, mclex_t);
mclex_t mclex_mul(mclex_t, mclex_t);
mclex_t mclex_div(mclex_t, mclex_t);
mclex_t mclex_mod(mclex_t, mclex_t);

mclex_t mclex_band(mclex_t, mclex_t);
mclex_t mclex_bor(mclex_t, mclex_t);
mclex_t mclex_bxor(mclex_t, mclex_t);
mclex_t mclex_bnot(mclex_t);

mclex_t mclex_shl(mclex_t, mclex_t);
mclex_t mclex_shr(mclex_t, mclex_t);

mclex_t mclex_and(mclex_t, mclex_t);
mclex_t mclex_or(mclex_t, mclex_t);
mclex_t mclex_not(mclex_t);

mclex_t mclex_cmp_e(mclex_t, mclex_t);
mclex_t mclex_cmp_g(mclex_t, mclex_t);
mclex_t mclex_cmp_ge(mclex_t, mclex_t);
mclex_t mclex_cmp_l(mclex_t, mclex_t);
mclex_t mclex_cmp_le(mclex_t, mclex_t);
mclex_t mclex_cmp_ne(mclex_t, mclex_t);
#define mclex_cmp_ng(op1, op2) mclex_cmp_le(op1, op2)
#define mclex_cmp_nl(op1, op2) mclex_cmp_ge(op1, op2)

/* reference/deference operators */
mclex_t mclex_ref(mclex_t);
mclex_t mclex_def(mclex_t);
mclex_t mclex_index(mclex_t, mclex_t);

void mclex_set(mclex_t lv, mclex_t rv);

mclex_t mclex_arg(mclt_t);

void mclex_v_begin();
void mclex_v(mclex_t);
mclex_t mclex_v_end();
mclex_t mclex_v_index(mclex_t, str_t);
#define mclex_v_index_cs(ex, s) mclex_v_index((ex), str_form_cs(mclex_heap(), (s)))
mclex_t mclex_v_first_n(mclex_t, int n);

mclex_t mclex_get_work_dim(); /* uint get_work_dim() */
mclex_t mclex_get_local_id(int d); /* size_t get_local_id(uint D) */
mclex_t mclex_get_global_size(int d); /* size_t get_global_size(uint D) */
mclex_t mclex_get_num_groups(int d); /* size_t get_num_groups (uint D) */
mclex_t mclex_get_global_id(int d); /* size_t get_global_id (uint D) */
mclex_t mclex_get_group_id(int d); /* size_t get_group_id (uint D) */
mclex_t mclex_get_local_size(int d); /* size_t get_local_size (uint D) */
mclex_t mclex_get_global_offset(int d); /* size_t get_global_offset (uint D) */

mclex_t mclex_abs(mclex_t); /* U abs (T x) | x | */
mclex_t mclex_abs_diff(mclex_t, mclex_t); /* U abs_diff (T x, T y) | x – y | without modulo overflow */

mclex_t mclex_add_sat(mclex_t, mclex_t); /* T add_sat (T x, T y) x + y and saturates the result */
mclex_t mclex_hadd(mclex_t, mclex_t); /* T hadd (T x, T y) (x + y) >> 1 without mod. overflow */
mclex_t mclex_rhadd(mclex_t, mclex_t); /* T rhadd (T x, T y) (x + y + 1) >> 1 */
mclex_t mclex_clz(mclex_t); /* T clz (T x) Number of leading 0-bits in x */
mclex_t mclex_clamp(mclex_t, mclex_t, mclex_t); /* T clamp (T x, T min, T max) */

mclex_t mclex_mad_hi(mclex_t, mclex_t, mclex_t); /* T mad_hi (T a, T b, T c) mul_hi(a, b) + c */
mclex_t mclex_mad_sat(mclex_t, mclex_t, mclex_t); /* T mad_sat (T a, T b, T c) a * b + c and saturates the result */

mclex_t mclex_min(mclex_t, mclex_t);
mclex_t mclex_max(mclex_t, mclex_t);

mclex_t mclex_mul_hi(mclex_t a1, mclex_t a2); /* high half of the product of x and y */
mclex_t mclex_rotate(mclex_t a1, mclex_t a2); /* result[indx] = v[indx] << i[indx] */
mclex_t mclex_sub_sat(mclex_t a1, mclex_t a2); /* x - y and saturates the result */

mclex_t mclex_mad24(mclex_t a1, mclex_t a2, mclex_t a3);
mclex_t mclex_mul24(mclex_t a1, mclex_t a2);

mclex_t mclex_degrees(mclex_t);
mclex_t mclex_radians(mclex_t);
mclex_t mclex_sign(mclex_t);

mclex_t mclex_mix(mclex_t, mclex_t, mclex_t);
mclex_t mclex_step(mclex_t, mclex_t);
mclex_t mclex_smoothstep(mclex_t, mclex_t, mclex_t);

mclex_t mclex_acos(mclex_t);
mclex_t mclex_acosh(mclex_t);
mclex_t mclex_acospi(mclex_t);
mclex_t mclex_asin(mclex_t);
mclex_t mclex_asinh(mclex_t);
mclex_t mclex_asinpi(mclex_t);
mclex_t mclex_atan(mclex_t);
mclex_t mclex_atan2(mclex_t, mclex_t);
mclex_t mclex_atanh(mclex_t);
mclex_t mclex_atanpi(mclex_t);
mclex_t mclex_atan2pi(mclex_t, mclex_t);
mclex_t mclex_cbrt(mclex_t);
mclex_t mclex_ceil(mclex_t);
mclex_t mclex_copysign(mclex_t, mclex_t);
mclex_t mclex_cos(mclex_t);
mclex_t mclex_cosh(mclex_t);
mclex_t mclex_cospi(mclex_t);
mclex_t mclex_erfc(mclex_t);
mclex_t mclex_erf(mclex_t);
mclex_t mclex_exp(mclex_t);
mclex_t mclex_exp2(mclex_t);
mclex_t mclex_exp10(mclex_t);
mclex_t mclex_expm1(mclex_t);
mclex_t mclex_fabs(mclex_t);
mclex_t mclex_fdim(mclex_t, mclex_t);
mclex_t mclex_floor(mclex_t);
mclex_t mclex_fma(mclex_t, mclex_t, mclex_t);
mclex_t mclex_fmax(mclex_t, mclex_t)
mclex_t mclex_fmin(mclex_t, mclex_t)
mclex_t mclex_fmod(mclex_t, mclex_t);
mclex_t mclex_hypot(mclex_t, mclex_t);
mclex_t mclex_lgamma(mclex_t)
mclex_t mclex_log(mclex_t);
mclex_t mclex_log2(mclex_t);
mclex_t mclex_log10(mclex_t);
mclex_t mclex_log1p(mclex_t);
mclex_t mclex_logb(mclex_t);
mclex_t mclex_mad(mclex_t, mclex_t, mclex_t);
mclex_t mclex_maxmag(mclex_t, mclex_t);
mclex_t mclex_log(mclex_t);
mclex_t mclex_log2(mclex_t);
mclex_t mclex_log10(mclex_t);
mclex_t mclex_log1p(mclex_t);
mclex_t mclex_logb(mclex_t);

mclex_t mclex_sincos(mclex_t, mclex_t);

/*
mclex_t mclex_fract(mclex_t, mclex_t);
mclex_t mclex_frexp(mclex_t, mclex_t);
mclex_t mclex_ldexp(mclex_t, mclex_t);
mclex_t mclex_ldexp(mclex_t, mclex_t);
mclex_t mclex_lgamma_r(mclex_t, mclex_t);
mclex_t mclex_ilogb(mclex_t);
*/

mclt_t mclex_random(int n);

#endif /* MAY_CL_EX_H */

