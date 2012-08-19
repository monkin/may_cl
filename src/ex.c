
#include "ex.h"
#include <assert.h>

static __thread mclex_program_t program;

static str_t mclex_var_name() {
	return str_cat(program->heap, str_from_cs(program->heap, "val"), str_from_int(program->heap, program->counter));
}

mclex_t mclex_ex(mclt_t t, long mem_type) {
	mclex_t ex = heap_alloc(program->heap, sizeof(mclex_s));
	ex->type = t;
	ex->source = sb_create(program->heap);
	ex->mem_type = mem_type;
	return ex;
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

void mclex_ret(mclex_block_t b, mclex_t ex) {
	if(!b)
		b = program->block;
	if(!b->var_name) {
		b->var_name = mclex_var_name();
		b->var_type = ex->type;
		sb_append(b->source, b->var_name);
		sb_append_cs(b->source, " = ");
		sb_append_sb(b->source, ex->source);
		sb_append_cs(b->source, ";\n");
	} else {
		if(b->var_type != ex->type)
			b->var_type = mclt_promote_std(b->var_type, ex->type);
		sb_append(b->source, b->var_name);
		sb_append_cs(b->source, " = ");
		sb_append_sb(ex->source);
		sb_append_cs(b->source, ";\n");
	}
}

mclex_t mclex_end(mclex_t ex) {
	if(ex)
		mclex_ret(0, ex);

	mclex_block_t b = program->block;
	sb_append_cs(b->source, "};\n");

	sb_t var_sb = sb_create(program->heap);
	sb_append(var_sb, mclt_name(b->var_type));
	sb_append(var_sb, b->var_name);
	sb_append_cs(var_sb, ";\n");
	sb_preppend_sb(b->source, var_sb);

	mclex_t r = mclex_ex(b->var_type, 0);
	sb_append(r->source, b->var_name);

	if(b->parent)
		sb_append_sb(b->parent->source, b->source);
	else
		sb_append_sb(program->local_source, b->source);
	program->block = b->parent;

	return r;
}

mclex_t mclex_cast(mclt_t t, mclex_t ex) {
	mclex_t r = mclex_ex(t, 0);
	if(mclt_is_vector(t) || mclt_is_scalar(t)) {
		sb_append_cs(r->source, "convert_");
		sb_append(r->source, mclt_name(cd->cast_to));
		sb_append_cs(r->source, "(");
		sb_append_sb(r->source, s);
		sb_append_cs(r->source, ")");
	} else {
		sb_append_cs(r->source, "((");
		sb_append(r->source, mclt_name(cd->cast_to));
		sb_append_cs(r->source, ") (");
		sb_append_sb(r->source, ex->source);
		sb_append_cs(r->source, "))");
	}
	return r;
}


