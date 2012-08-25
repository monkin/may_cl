
#include "ex.h"
#include <assert.h>

__thread mclex_program_t mclex_program;

static str_t mclex_var_name() {
	return str_cat(mclex_heap(), str_from_cs(mclex_heap(), "val"), str_from_int(mclex_heap(), mclex_program->counter));
}

mclex_t mclex_ex(mclt_t t, long mem_type) {
	mclex_t ex = heap_alloc(mclex_heap(), sizeof(mclex_s));
	ex->type = t;
	ex->source = sb_create(mclex_heap());
	ex->mem_type = mem_type;
	return ex;
}

/* Program */

void mclex_program_begin() {
	assert(!mclex_program);
	heap_t h = heap_create(0);
	err_try {
		mclex_program = heap_alloc(h, sizeof(mclex_program_s));
		mclex_program->heap = h;
		mclex_program->block = 0;
		mclex_program->global_source = sb_create(h);
		mclex_program->local_source = sb_create(h);
		mclex_program->arguments_source = sb_create(h);
		mclex_program->arguments = map_create(h);
		mclex_program->source = 0;
		mclex_program->global_flags = map_create(h);
		mclex_program->counter = 0;
	} err_catch {
		h = heap_delete(h);
		err_throw_down();
	}
}

void mclex_program_reset() {
	assert(mclex_program);
	heap_delete(mclex_heap());
	mclex_program = 0;
}

mclex_program_t mclex_program_end() {
	assert(mclex_program);
	assert(!mclex_program->block);
	mclex_program_t r = mclex_program;
	mclex_program = 0;
	return r;
}

mclex_program_t mclex_program_delete(mclex_program_t p) {
	if(p)
		heap_delete(p->heap);
	return 0;
}

/* Block */

void mclex_begin() {
	mclex_block_t block = heap_alloc(mclex_heap(), sizeof(mclex_block_s));
	block->source = sb_create(mclex_heap());
	sb_append_cs(block->source, "{\n")
	block->var_type = MCLT_VOID;
	block->var_name = 0;
	block->parent = mclex_program->block;
	program->block = block;
}

void mclex_ret(mclex_block_t b, mclex_t ex) {
	if(!b)
		b = mclex_block();
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

	mclex_block_t b = mclex_block();
	sb_append_cs(b->source, "};\n");

	sb_t var_sb = sb_create(mclex_heap());
	sb_append(var_sb, mclt_name(b->var_type));
	sb_append(var_sb, b->var_name);
	sb_append_cs(var_sb, ";\n");
	sb_preppend_sb(b->source, var_sb);

	mclex_t r = mclex_ex(b->var_type, 0);
	sb_append(r->source, b->var_name);

	if(b->parent)
		sb_append_sb(b->parent->source, b->source);
	else
		sb_append_sb(mclex_local_source(), b->source);
	mclex_program->block = b->parent;

	return r;
}

mclex_t mclex_cast(mclt_t t, mclex_t ex) {
	mclt_t ex_t = ex->type;
	if(t==ex_t)
		return ex;
	if((mclt_is_vector(t) && ((mclt_is_vector(ex_t) ? mclt_vector_size(ex_t)==mclt_vector_size(t) : false) || mclt_is_numeric(ex_t)))
	   		|| (mclt_is_numeric(t) && mclt_is_numeric(ex_t))
	   		|| ((mclt_is_poiter(t) && mclt_is_pointer(t)) ? mclt_pointer_type(t)==mclt_pointer_type(ex_t) : false)) {
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
	} else
		err_throw(e_mclex_casting_error);
}

void mclex_if(mclex_t ex) {
	if(ex->type == MCLT_VOID)
		err_throw(e_mclex_casting_error);
	mclex_begin();
	sb_t sb = mclex_block_source();
	sb_preppend_cs(sb, ") ");
	sb_preppend_sb(sb, ex->source);
	sb_preppend_cs(sb, "if(");
}
void mclex_unless(mclex_t ex) {
	if(ex->type == MCLT_VOID)
		err_throw(e_mclex_casting_error);
	mclex_begin();
	sb_t sb = mclex_block_source();
	sb_preppend_cs(sb, ")) ");
	sb_preppend_sb(sb, ex->source);
	sb_preppend_cs(sb, "if(!(");
}
void mclex_elsif(mclex_t ex) {
	if(ex->type == MCLT_VOID)
		err_throw(e_mclex_casting_error);
	sb_t sb = mclex_block_source();
	sb_append_cs(sb, "} else if(");
	sb_append_sb(sb, ex->source);
	sb_append_cs(sb, ") {\n");
}
void mclex_else() {
	sb_append_cs(mclex_block_source(), "} else {\n");
}

void mclex_while(mclex_t ex) {
	if(ex->type == MCLT_VOID)
		err_throw(e_mclex_casting_error);
	mclex_begin();
	sb_t sb = mclex_block_source();
	sb_preppend_cs(sb, ") ");
	sb_preppend_sb(sb, ex->source);
	sb_preppend_cs(sb, "while(");
}

mclex_t mclex_null(mclt_t t) {
	if(ex->type == MCLT_VOID)
		err_throw(e_mclex_casting_error);
	mclex_t r = mclex_ex(t, 0);
	sb_append_cs(r->source, "((");
	sb_append(r->source, mclt_name(t));
	sb_append_cs(r->source, ") 0)");
	return r;
}

mclex_t mclex_for(mclex_t b, mclex_t e) {
	b = mclex_cast(MCLT_INT, b);
	e = mclex_cast(MCLT_INT, e);
	str_t var_name = mclex_var_name();
	sb_t for_src = sb_create(mclex_heap());
	sb_append_cs(for_src, "int ");
		sb_append(for_src, var_name);
		sb_append_cs(for_src, "_i = ");
		sb_append_sb(for_src, b->source);
		sb_append_cs(for_src, ";\n");
	sb_append_cs(for_src, "const int ");
		sb_append(for_src, var_name);
		sb_append_cs(for_src, "_e = ");
		sb_append_sb(for_src, e->source);
		sb_append_cs(for_src, ";\n");
	sb_append_cs(for_src, "const int ");
		sb_append(for_src, var_name);
		sb_append_cs(for_src, "_d = ");
		sb_append(for_src, var_name);
		sb_append_cs(for_src, "_i < ");
		sb_append(for_src, var_name);
		sb_append_cs(for_src, "_e ? 1 : -1;\n");
	sb_append_cs(for_src, "for(; ");
		sb_append(for_src, var_name);
		sb_append_cs(for_src, "_i != ");
		sb_append(for_src, var_name);
		sb_append_cs(for_src, "_e; ");
		sb_append(for_src, var_name);
		sb_append_cs(for_src, "_i += ");
		sb_append(for_src, var_name);
		sb_append_cs(for_src, "_d) ");
	mclex_begin();
	sb_preppend_sb(mclex_block_source(), for_src);
	mclex_t r = mclex_ex(MCLT_INT, 0);
	sb_append(r->source, var_name);
	sb_append_cs(r->source, "_i");
	return r;
}


static mclex_t mclex_var_i(mclex_t ex, sb_t sb, long mem_type) {
	if(ex->type == MCLT_VOID)
		err_throw(e_mclex_casting_error);
	mclex_t r = mclex_ex(ex->type, mem_type);
	str_t name = mclex_var_name();
	sb_append(r->source, name);
	sb_append(bs, mclt_name(r->type));
	sb_append_cs(bs, " ");
	sb_append_sb(bs, name);
	sb_append_cs(bs, " = ");
	sb_append_sb(bs, ex->source);
	sb_append_cs(bs, ";\n");
	return r;
}

mclex_t mclex_var(mclex_t ex) {
	return mclex_var_i(ex, mclex_block_source(), MCL_MEM_PRIVATE);
}

mclex_t mclex_global_var(str_t name, mclex_t ex) {
	mclex_t r = (mclex_t) map_get(mclex_program->global_flags, name);
	if(!r) {
		r = (mclex_t) mclex_var_i(ex, mclex_global_source(), 0);
		map_set(mclex_program->global_flags, name, r);
	}
	return r;
}

static mclex_t mclex_const_i(mclex_t ex, sb_t sb) {	
	if(ex->type == MCLT_VOID)
		err_throw(e_mclex_casting_error);
	mclex_t r = mclex_ex(ex->type, mem_type);
	str_t name = mclex_var_name();
	sb_append(r->source, name);
	if(!mclt_is_pointer(ex->type))
		sb_append_cs(bs, "const ");
	sb_append(bs, mclt_name(r->type));
	sb_append_cs(bs, " ");
	sb_append_sb(bs, name);
	if(mclt_is_pointer(ex->type))
		sb_append_cs(bs, " const ");
	sb_append_cs(bs, " = ");
	sb_append_sb(bs, ex->source);
	sb_append_cs(bs, ";\n");
	return r;
}

mclex_t mclex_const(mclex_t ex) {
	return mclex_const_i(ex, mclex_block_source());
}

mclex_t mclex_global_const(str_t name, mclex_t ex) {
	mclex_t r = (mclex_t) map_get(mclex_program->global_flags, name);
	if(!r) {
		r = (mclex_t) mclex_const_i(ex, mclex_global_source());
		map_set(mclex_program->global_flags, name, r);
	}
	return r;
}

static void mclex_literal_bytes(sb_t sb, const char *data, size_t sz) {
	static char letters[] = "0123456789abcdef";
	str_t rs = str_create(mclex_heap(), sz*2);
	str_it_t s = str_begin(r);
	size_t i;
	for(i=0; i<sz; i++) {
		*(s++) = letters[(data[i]>>4) & 0x0f];
		*(s++) = letters[data[i] & 0x0f];
	}
	sb_append(sb, rs);
}

mclex_t mclex_literal(mclt_t tp, const void *val) {
	mclex_t r = mclex_ex(tp, 0);
	if (mclt_is_vector(tp)) {
		int i;
		const mclt_t vt = mclt_vector_type(tp);
		const int vs = mclt_vector_size(tp);
		const int vts = mclt_size(vt);
		sb_append_cs(r->source, "{");
		for(i=0; i<vs; i++) {
			if(i!=0)
				sb_append_cs(r->source, ", ");
			mclex_literal(vt, ((const char *) val) + i*vts);
		}
		sb_append_cs(r->source, "}");
	} else if(mclt_is_float(tp)) {
		sb_append_cs(r->source, "as_float(0x");
		mclex_literal_bytes(r->source, (const char *) val, mclt_sizeof(tp));
		sb_append_cs(r->source, ")");
	} else if(mclt_is_integer(tp)) {
		sb_append_cs(r->source, "((");
		sb_append(r->source, mclt_name(ex->type));
		sb_append_cs(r->source, ") 0x");
		mclex_literal_bytes(r->source, (const char *) val, mclt_sizeof(tp));
		sb_append_cs(r->source, ")");
	} else
		err_throw(e_mclex_casting_error);
	return r;
}

