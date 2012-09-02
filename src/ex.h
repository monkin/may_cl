
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

mclex_t mclex_global_var(str_t name, mclex_t);
mclex_t mclex_global_const(str_t name, mclex_t);

mclex_t mclex_literal(mclt_t tp, const void *val);
mclex_t mclex_array(mclt_t tp, size_t sz, const void *val);

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

mclex_t mclex_get_work_dim(); /* uint get_work_dim() */
mclex_t mclex_get_local_id(int d); /* size_t get_local_id(uint D) */
mclex_t mclex_get_global_size(int d); /* size_t get_global_size(uint D) */
mclex_t mclex_get_num_groups(int d); /* size_t get_num_groups (uint D) */
mclex_t mclex_get_global_id(int d); /* size_t get_global_id (uint D) */
mclex_t mclex_get_group_id(int d); /* size_t get_group_id (uint D) */
mclex_t mclex_get_local_size(int d); /* size_t get_local_size (uint D) */
mclex_t mclex_get_global_offset(int d); /* size_t get_global_offset (uint D) */

#endif /* MAY_CL_EX_H */

