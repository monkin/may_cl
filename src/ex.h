
#ifndef MAY_CL_EX_H
#define MAY_CL_EX_H

#include "mcl.h"
#include <may/core/lib.h>
#include <stdbool.h>

ERR_DECLARE(e_mclex_casting_error);

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
	map_t arguments;
	str_t source;
	unsigned long counter; // Used to generate variables names.
} mclex_program_s;

typedef mclex_program_s *mclex_program_t;

void mclex_program_begin();
void mclex_program_reset(); // Remove all current program data and stop building.
mclex_program_t mclex_program_end();
mclex_program_t mclex_program_delete(mclex_program_t);

typedef struct {
	mclt_t type;
	sb_t expression;
} mclex_s;

typedef mclex_s *mclex_t;

void mclex_begin(); // block
mclex_block_t mclex_current_block();
/**
 * If block is null, current block used.
 */
mclex_t mclex_ret(mclex_block_t, mclex_t); // set block result
mclex_t mclex_end(mclex_t); // ~block

void mclex_if(mclex_t); // block
void mclex_elsif(mclex_t);
void mclex_unless(mclex_t); // block
void mclex_else();

void mclex_while(mclex_t); // block
/**
 * Gets two integer expressions and return iterator value.
 */
mclex_t mclex_for(mclex_t, mclex_t); // block

mclex_t mclex_cast(mclt_t, mclex_t);

mclex_t mclex_var(mclex_t);
mclex_t mclex_const(mclex_t);

mclex_t mclex_global_var(str_t name, mclex_t);
mclex_t mclex_global_const(str_t name, mclex_t);

mclex_t mclex_literal(mclt_t tp, const void *val);
mclex_t mclex_array(mclt_t tp, size_t sz, const void *val);


#endif /* MAY_CL_EX_H */
