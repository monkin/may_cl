
#include "ex.h"
#include <assert.h>

static __thread mclex_program_t program;

static str_t mclex_var_name() {
	return str_cat(program->heap, str_from_cs(program->heap, "val"), str_from_int(program->heap, program->counter));
}

/* Program */

void mclex_program_begin() {
	assert(!program);
	heap_t h = heap_create(0);
	err_try {
		program = heap_alloc(h, sizeof(mclex_program_s));
		program->heap = h;
		program->block = 0;
		program->global_source = sb_create(h);
		program->local_source = sb_create(h);
		program->arguments_source = sb_create(h);
		program->arguments = map_create(h);
		program->source = 0;
		program->counter = 0;
	} err_catch {
		h = heap_delete(h);
		err_throw_down();
	}
}

void mclex_program_reset() {
	assert(program);
	heap_delete(program->heap);
	program = 0;
}

mclex_program_t mclex_program_end() {
	assert(program);
	assert(!program->block);
	mclex_program_t r = program;
	program = 0;
	return r;
}

mclex_program_t mclex_program_delete(mclex_program_t p) {
	if(p)
		heap_delete(p->heap);
	return 0;
}

/* Block */

void mclex_begin() {
	mclex_block_t block = heap_alloc(program->heap, sizeof(mclex_block_s));
	block->source = sb_create(program->heap);
	sb_append_cs(block->source, "{\n")
	block->var_type = MCLT_VOID;
	block->var_name = 0;
	block->parent = program->block;
	program->block = block;
}

mclex_block_t mclex_current_block() {
	return program->block;
}

mclex_t mclex_ret(mclex_block_t b, mclex_t ex) {
	if(!b)
		b = program->block;
	if(!b->var_name) {
		b->var_name = mclex_var_name();
		b->var_type = ex->type;
		sb_append(b->source, b->var_name);
		sb_append_cs(b->source, " = ");
		sb_append_sb(b->source, ex->expression);
		sb_append_cs(b->source, ";\n");
	} else {
		//!!! TODO: write code
	}
}

mclex_t mclex_end(mclex_t ex) {
	if(ex)
		mclex_ret(0, ex);
	//!!! TODO: write code
}

