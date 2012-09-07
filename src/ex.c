
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
	ex->argument_index = 0;
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
		mclex_program->arguments_count = 0;
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
		err_throw(e_mclex_error);
}

void mclex_if(mclex_t ex) {
	if(ex->type == MCLT_VOID)
		err_throw(e_mclex_error);
	mclex_begin();
	sb_t sb = mclex_block_source();
	sb_preppend_cs(sb, ") ");
	sb_preppend_sb(sb, ex->source);
	sb_preppend_cs(sb, "if(");
}
void mclex_unless(mclex_t ex) {
	if(ex->type == MCLT_VOID)
		err_throw(e_mclex_error);
	mclex_begin();
	sb_t sb = mclex_block_source();
	sb_preppend_cs(sb, ")) ");
	sb_preppend_sb(sb, ex->source);
	sb_preppend_cs(sb, "if(!(");
}
void mclex_elsif(mclex_t ex) {
	if(ex->type == MCLT_VOID)
		err_throw(e_mclex_error);
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
		err_throw(e_mclex_error);
	mclex_begin();
	sb_t sb = mclex_block_source();
	sb_preppend_cs(sb, ") ");
	sb_preppend_sb(sb, ex->source);
	sb_preppend_cs(sb, "while(");
}

mclex_t mclex_null(mclt_t t) {
	if(ex->type == MCLT_VOID)
		err_throw(e_mclex_error);
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
		err_throw(e_mclex_error);
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
		err_throw(e_mclex_error);
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

void mclex_literal_i(sb_t sb, mclt_t tp, const void *val) {
	if (mclt_is_vector(tp)) {
		int i;
		const mclt_t vt = mclt_vector_type(tp);
		const int vs = mclt_vector_size(tp);
		const int vts = mclt_size(vt);
		sb_append_cs(sb, "((");
		sb_append(sb, mclt_name(tp));
		sb_append_cs(sb, ") (");
		for(i=0; i<vs; i++) {
			if(i!=0)
				sb_append_cs(sb, ", ");
			mclex_literal(vt, ((const char *) val) + i*vts);
		}
		sb_append_cs(sb, "))");
	} else if(mclt_is_float(tp)) {
		sb_append_cs(sb, "as_float(0x");
		mclex_literal_bytes(sb, (const char *) val, mclt_sizeof(tp));
		sb_append_cs(sb, ")");
	} else if(mclt_is_integer(tp)) {
		sb_append_cs(sb, "((");
		sb_append(sb, mclt_name(tp));
		sb_append_cs(sb, ") 0x");
		mclex_literal_bytes(sb, (const char *) val, mclt_sizeof(tp));
		sb_append_cs(sb, ")");
	} else
		err_throw(e_mclex_error);
}

mclex_t mclex_literal(mclt_t tp, const void *val) {
	mclex_t r = mclex_ex(tp, 0);
	mclex_literal_i(r->source, tp, val);
	return r;
}

mclex_t mclex_array(mclt_t tp, size_t sz, const void *val) {
	mclex_t r = mclex_ex(tp, 0);
	if(mclt_is_pointer(tp)) {
		size_t i;
		const pt = mclt_pointer_to(tp);
		sb_append_cs(r->source, "{");
		for(i=0; i<sz; i++) {
			if(i!=0)
				sb_append_cs(r->source, ", ");
			mclex_literal_i(r->source, pt, ((const char *) val) + mclt_sizeof(pt)*i);
		}
		sb_append_cs(r->source, "}");
	} else
		err_throw(e_mclex_error);
	return r;
}

/* operators */

mclex_t mclex_neg(mclex_t ex) {
	if(mclt_is_vector(ex->type) || mclt_is_scalar(ex->type)) {
		mclex_t r = mclex_ex(ex->type, 0);
		sb_append_cs(r->source, "(-");
		sb_append_sb(r->source, ex->source);
		sb_append_cs(r->source, ")");
		return r;
	} else
		err_throw(e_mclex_error);
}

static void mclex_binary_op_source(sb_t sb, const char *op, mclex_t a1, mclex_t a2) {
	sb_append_cs(sb, "(");
	sb_append_sb(sb, a1->source);
	sb_append_cs(sb, op);
	sb_append_sb(sb, a2->source);
	sb_append_cs(sb, ")");
}

static mclex_t mclex_binary_op(const char *op, mclex_t a1, mclex_t a2) {
	mclt_t rt = mclt_promote(a1->type, a2->type);
	mclex_t r = mclex_ex(rt, 0);
	sb_t sb = r->source;
	mclex_binary_op_source(r->source, op, mcex_cast(rt, a1), mcex_cast(rt, a2));
	return r;
}

mclex_t mclex_add(mclex_t op1, mclex_t op2) {
	if(mclt_is_pointer(op1) && mclt_is_integer(op2)) {
		mclex_t r = mclex_ex(op1->type, 0);
		mclex_binary_op_source(r->source, " + ", op1, op2);
		return r;
	} else if(mclt_is_pointer(op2))
		return mclex_add(op2, op1);
	else
		return mclex_binary_op(" + ", op1, op2);
}
mclex_t mclex_sub(mclex_t op1, mclex_t op2) {
	if(mclt_is_pointer(op1) && mclt_is_integer(op2)) {
		mclex_t r = mclex_ex(op1->type, 0);
		mclex_binary_op_source(r->source, " - ", op1, op2);
		return r;
	} else if(mclt_is_pointer(op1) && mclt_is_pointer(op2)) {
		if((op1->type == op2->type) ? mclt_pointer_to(op1)!=MCLT_VOID : false) {
			mclex_t r = mclex_ex(MCLT_LONG, 0);
			sb_append_cs(r->source, "((long) (");
			mclex_binary_op_source(r->source, " - ", op1, op2);
			sb_append_cs(r->source, "))");
			sb_t sb = r->source;
		} else
			err_throw(e_mclex_error);
	} else
		return mclex_binary_op(" - ", op1, op2);
}
mclex_t mclex_mul(mclex_t op1, mclex_t op2) {
	return mclex_binary_op(" * ", op1, op2);
}
mclex_t mclex_div(mclex_t op1, mclex_t op2) {
	return mclex_binary_op(" / ", op1, op2);
}

static mclex_t  mclex_binary_int_op(const char *op, mclex_t a1, mclex_t a2) {
	if((mclt_is_integer(a1) || mclt_is_vector_of_integer(a1)) && (mclt_is_integer(a2) || mclt_is_vector_of_integer(a2)))
		return mclex_binary_op(op, a1, a2);
	else
		err_throw(e_mclex_error);
}
mclex_t mclex_mod(mclex_t op1, mclex_t op2) {
	return mclex_binary_int_op(" % ", op1, op2);
}

mclex_t mclex_band(mclex_t op1, mclex_t op2) {
	return mclex_binary_int_op(" & ", op1, op2);
}
mclex_t mclex_bor(mclex_t op1, mclex_t op2) {
	return mclex_binary_int_op(" | ", op1, op2);
}
mclex_t mclex_bxor(mclex_t op1, mclex_t op2) {
	return mclex_binary_int_op(" ^ ", op1, op2);
}

mclex_t mclex_bnot(mclex_t op) {
	if(mclt_is_vector_of_integer(ex->type) || mclt_is_integer(ex->type)) {
		mclex_t r = mclex_ex(ex->type, 0);
		sb_append_cs(r->source, "(~");
		sb_append_sb(r->source, ex->source);
		sb_append_cs(r->source, ")");
		return r;
	} else
		err_throw(e_mclex_error);
}

mclex_t mclex_shr(mclex_t op1, mclex_t op2) {
	return mclex_binary_int_op(" >> ", op1, op2);
}
mclex_t mclex_shl(mclex_t op1, mclex_t op2) {
	return mclex_binary_int_op(" << ", op1, op2);
}

static mclex_t mclex_binary_logic_op(const char *op, mclex_t a1, mclex_t a2) {
	mclex_t r = mclex_binary_int_op(op, a1, a2);
	r->type = mclt_promote_bool(r);
	return r;
}

mclex_t mclex_and(mclex_t op1, mclex_t op2) {
	return mclex_binary_logic_op(" && ", op1, op2);
}
mclex_t mclex_or(mclex_t op1, mclex_t op2) {
	return mclex_binary_logic_op(" || ", op1, op2);
}
mclex_t mclex_not(mclex_t ex) {
	mclex_t r = mclex_ex(mclt_promote_bool(ex->type), 0);
	sb_append_cs(r->source, "(!");
	sb_append_sb(r->source, ex->source);
	sb_append_cs(r->source, ")");
	return r;
}

mclex_t mclex_cmp_e(mclex_t, mclex_t) {
	return mclex_binary_logic_op(" == ", op1, op2);
}
mclex_t mclex_cmp_g(mclex_t, mclex_t) {
	return mclex_binary_logic_op(" > ", op1, op2);
}
mclex_t mclex_cmp_ge(mclex_t, mclex_t) {
	return mclex_binary_logic_op(" >= ", op1, op2);
}
mclex_t mclex_cmp_l(mclex_t, mclex_t) {
	return mclex_binary_logic_op(" < ", op1, op2);
}
mclex_t mclex_cmp_le(mclex_t, mclex_t) {
	return mclex_binary_logic_op(" <= ", op1, op2);
}
mclex_t mclex_cmp_ne(mclex_t, mclex_t) {
	return mclex_binary_logic_op(" != ", op1, op2);
}

/* pointer operators */

mclex_t mclex_ref(mclex_t op) {
	mclt_t ot = op->type;
	if(op->mem_type && (mclt_is_vector(ot) || mclt_is_scalar(ot))) {
		mclex_t r = mclex_ex(mclt_pointer(ot, op->mem_type), 0);
		sb_append_cs(r->source, "(&");
		sb_append_sb(r->source, op->source);
		sb_append_cs(r->source, ")");
		return r;
	} else
		err_throw(e_mclex_error);
}
mclex_t mclex_def(mclex_t op) {
	mclt_t ot = op->type;
	if(mclt_is_pointer(ot) && mclt_pointer_to(ot)!=MCLT_VOID) {
		mclex_t r = mclex_ex(mclt_pointer_to(ot), mclt_pointer_type(ot));
		sb_append_cs(r->source, "(*");
		sb_append_sb(r->source, op->source);
		sb_append_cs(r->source, ")");
		return r;
	} else
		err_throw(e_mclex_error);
}
mclex_t mclex_index(mclex_t op1, mclex_t op2) {
	mclt_t ot = op1->type;
	if(mclt_is_pointer(ot) && mclt_pointer_to(ot)!=MCLT_VOID && mclt_is_integer(op2->type)) {
		mclex_t r = mclex_ex(mclt_pointer_to(ot), mclt_pointer_type(ot));
		sb_append_cs(r->source, "(");
		sb_append_sb(r->source, op1->source);
		sb_append_cs(r->source, "[");
		sb_append_sb(r->source, op2->source);
		sb_append_cs(r->source, "])");
		return r;
	} else
		err_throw(e_mclex_error);
}

void mclex_set(mclex_t lv, mclex_t rv) {
	if(lv->mem_type && lv->mem_type!=MCL_MEM_CONSTANT) {
		rv = mclex_cast(lv->type, rv);
		sb_t  sb = mclex_local_source();
		sb_append_sb(sb, lv->source);
		sb_append_cs(sb, " = ");
		sb_append_sb(sb, rv->source);
		sb_append_cs(sb, ";\n");
	} else
		err_throw(e_mclex_error);
}

mclex_t mclex_arg(mclt_t t) {
	if(t!=MCLT_VOID) {
		str_t nm = mclex_var_name();
		mclex_t r = mclex_ex(t, 0);
		r->argument_index = mclex_program->arguments_count++;
		if(r->argument_index)
			sb_append_cs(mclex_program->arguments_source, ", ");
		sb_append(mclex_program->arguments_source, mclt_name(t));
		sb_append_cs(mclex_program->arguments_source, " ");
		sb_append(mclex_program->arguments_source, nm);
		sb_append(r->source, nm);
		return r;
	} else
		err_throw(e_mclex_error);
}

/* vector functions */
void mclex_v_begin() {
	if(mclex_program->v_count!=0)
		err_throw(e_mclex_error);
}
void mclex_v(mclex_t ex) {
	if(mclex_program->v_count>15 || !(mclt_is_scalar(ex->type) || mclt_is_vector(ex->type)))
		err_throw(e_mclex_error);
	mclex_program->v_items[mclex_program->v_count] = ex;
}
mclex_t mclex_v_end() {
	int i, vsz = 0;
	mclt_t vt;
	mclex_t r;
	for(i=0; i<mclex_program->v_count; i++) {
		mclt_t t = mclex_program->v_items[i];
		mclt_t st = mclt_is_scalar(t) ? t : mclt_vector_of(t)
		vsz += t==st ? 1 : mclt_vector_size(t);
		vt = i ? mclt_promote(vt, st) : st;
	}
	r = mclex_ex(mclt_vector(vt, vsz), 0);
	sb_append_cs(r->source, "((");
	sb_append(r->source, mclt_name(r->type));
	sb_append_cs(r->source, ") (");
	for(i=0; i<mclex_program->v_count; i++)
		sb_append_sb(r->source, mclex_program->v_items[i]->source);
	sb_append_cs(r->source, "))");
	return r;
}

mclex_t mclex_v_index(mclex_t ex, str_t s) {
	mclex_t r = mclex_ex(mclex_vector(mclex_vector_of(ex->type), s->length), ex->mem_type);
	str_it_t i;
	bool is_xyzw = false;
	bool is_s = false;
	for(i=str_begin(s); i<str_end(s); s++) {
		if(((*i>='0' && *i<='9') || (*i>='a' && *i<='f') || (*i>='A' && *i<='F')) && !is_xyzw)
			is_s = true;
		else if((*i=='x' || *i=='y' || *i=='z' || *i=='w') && !is_s)
			is_xyzw = true;
		else
			err_throw(e_mclex_error);
	}
	sb_append_sb(r->source, ex->source);
	if(is_xyzw)
		sb_append_cs(r->source, ".");
	else
		sb_append_cs(r->source, ".s");
	sb_append(r->source, s);
	return r;
}

/* work item functions */

mclex_t mclex_get_work_dim() {
	mclex_t r = mclex_ex(MCLT_UINT, 0);
	sb_sppend_cs(r->source, "get_work_dim()");
	return r;
}

static mclex_t mclex_work_item_fn(const char *nm, int d) {
	if(d<0 && d>2)
		err_throw(e_mclex_error);
	mclex_t r = mclex_ex(MCLEX_LONG, 0);
	sb_append_cs(r->source, "((long) "
	sb_append_cs(r->source, nm)
	sb_append_cs(r->source, "(");
	sb_append(r->source, str_from_int(mclex_heap(), d));
	sb_append_cs(r->source, "))");
	return r;
}

mclex_t mclex_get_local_id(int d) {
	return mclex_work_item_fn("get_local_id", d);
}
mclex_t mclex_get_global_size(int d) {
	return mclex_work_item_fn("get_global_size", d);
}
mclex_t mclex_get_num_groups(int d) {
	return mclex_work_item_fn("get_num_groups", d);
}
mclex_t mclex_get_global_id(int d) {
	return mclex_work_item_fn("get_global_id", d);
}
mclex_t mclex_get_group_id(int d) {
	return mclex_work_item_fn("get_group_id", d);
}
mclex_t mclex_get_local_size(int d) {
	return mclex_work_item_fn("get_local_size", d);
}
mclex_t mclex_get_global_offset(int d) {
	return mclex_work_item_fn("get_global_offset", d);
}

mclex_t mclex_abs(mclex_t ex) {
	mclex_t r = mclex_ex(mclt_unsigned(ex->type), 0);
	sb_append_cs(r->source, "abs(");
	sb_append_sb(ex->source);
	sb_append_cs(r->source, ")");
	return r;
}
mclex_t mclex_abs_diff(mclex_t a1, mclex_t a2) {
	mclt_t at = mclt_promote(a1->type, a2->type);
	mclex_t r = mclex_ex(mclt_unsigned(at), 0);
	a1 = mclt_cast(at, a1);
	a2 = mclt_cast(at, a2);
	sb_append_cs(r->source, "abs_diff(");
	sb_append_sb(a1->source);
	sb_append_cs(r->source, ", ");
	sb_append_sb(a2->source);
	sb_append_cs(r->source, ")");
	return r;
}

mclex_t mclex_fn_int_2(const char *nm, mclex_t a1, mclex_t a2) {
	mclt_t rt = mclt_promote(a1->type, a2->type);
	if(mclt_is_integer(rt) || mclt_is_vector_of_integer(rt)) {
		mclex_t r = mclex_ex(rt, 0);
		a1 = mclex_cast(rt, a1);
		a2 = mclex_cast(rt, a2);
		sb_append_cs(r->source, nm);
		sb_append_cs(r->source, "(");
		sb_append_sb(r->source, a1->source);
		sb_append_cs(r->source, ", ");
		sb_append_sb(r->source, a2->source);
		sb_append_cs(r->source, ")");
	} else
		err_throw(e_mclex_error);
}

mclex_t mclex_fn_int_3(const char *nm, mclex_t a1, mclex_t a2, mclex_t a3) {
	mclt_t rt = mclt_promote(mclt_promote(a1->type, a2->type), a3->type);
	if(mclt_is_integer(rt) || mclt_is_vector_of_integer(rt)) {
		mclex_t r = mclex_ex(rt, 0);
		a1 = mclex_cast(rt, a1);
		a2 = mclex_cast(rt, a2);
		a3 = mclex_cast(rt, a3);
		sb_append_cs(r->source, nm);
		sb_append_cs(r->source, "(");
		sb_append_sb(r->source, a1->source);
		sb_append_cs(r->source, ", ");
		sb_append_sb(r->source, a2->source);
		sb_append_cs(r->source, ", ");
		sb_append_sb(r->source, a3->source);
		sb_append_cs(r->source, ")");
	} else
		err_throw(e_mclex_error);
}

mclex_t mclex_add_sat(mclex_t a1, mclex_t a2) {
	return mclex_fn_int_2("add_sat", a1, a2);
}
mclex_t mclex_hadd(mclex_t a1, mclex_t a2) {
	return mclex_fn_int_2("hadd", a1, a2);
}
mclex_t mclex_rhadd(mclex_t a1, mclex_t a2) {
	return mclex_fn_int_2("rhadd", a1, a2);
}
mclex_t mclex_clz(mclex_t a) {
	if(mclt_is_integer(a->type) || mclt_is_vector_of_integer(a->type)) {
		r = mclex_ex(a->type, 0);
		sb_append_cs(r->source, "clz(");
		sb_append_sb(r->source, a->source);
		sb_append_cs(r->source, ")");
	} else
		err_throw(e_mclex_error);
}
mclex_t mclex_clamp(mclex_t a1, mclex_t a2, mclex_t a3) {
	mclt_t rt, rt2;
	mclt_t rt = mclt_promote(mclt_promote(a1->type, a2->type), a3->type);
	if(mclt_is_vector(a1->type) && mclt_is_scalar(a2->type) && mclt_is_scalar(a3->type))
		rt2 = mclt_vector_of(rt);
	else
		rt2 = rt;
	r = mclex_ex(rt, 0);
	a1 = mclex_cast(rt, a1);
	a2 = mclex_cast(rt2, a2);
	a3 = mclex_cast(rt2, a3);
	sb_append_cs(r->source, "clamp(");
	sb_append_sb(r->source, a1->source);
	sb_append_cs(r->source, ", ");
	sb_append_sb(r->source, a2->source);
	sb_append_cs(r->source, ", ");
	sb_append_sb(r->source, a3->source);
	sb_append_cs(r->source, ")");
	return r;
}

mclex_t mclex_mad_hi(mclex_t a1, mclex_t a2, mclex_t a3) {
	return mclex_fn_int_3("mad_hi", a1, a2, a3);
}
mclex_t mclex_mad_sat(mclex_t a1, mclex_t a2, mclex_t a3) {
	return mclex_fn_int_3("mad_sat", a1, a2, a3);
}

mclex_t mclex_min_max(const char *nm, mclex_t a1, mclex_t a2) {
	mclt_t rt = mclt_promote(a1->type, a2->type);
	mclex_t a2t = rt;
	mclex_t r = mclex_ex(rt, 0);
	if(mclt_is_vector(rt) && mclt_is_scalar(a2->type))
		a2t = mclt_vector_of(rt);
	a1 = mclex_cast(rt, a1);
	a2 = mclex_cast(a2t, a2);
	sb_append_cs(r->source, nm);
	sb_append_cs(r->source, "(");
	sb_append_sb(r->source, a1->source);
	sb_append_cs(r->source, ", ");
	sb_append_sb(r->source, a2->source);
	sb_append_cs(r->source, ")");
}

mclex_t mclex_min(mclex_t a1, mclex_t a2) {
	return mclex_min_max("min", a1, a2);
}
mclex_t mclex_max(mclex_t a1, mclex_t a2) {
	return mclex_min_max("max", a1, a2);
}

mclex_t mclex_mul_hi(mclex_t a1, mclex_t a2) {
	return mclex_fn_int_2("mul_hi", a1, a2);
}
mclex_t mclex_rotate(mclex_t a1, mclex_t a2) {
	return mclex_fn_int_2("rotate", a1, a2);
}
mclex_t mclex_sub_sat(mclex_t a1, mclex_t a2) {
	return mclex_fn_int_2("sub_sat", a1, a2);
}

mclex_t mclex_mad24 (mclex_t a1, mclex_t a2, mclex_t a3) {
	return mclex_fn_int_3("mad24", a1, a2, a3);
}
mclex_t mclex_mul24 (mclex_t a1, mclex_t a2) {
	return mclex_fn_int_2("mul24", a1, a2);
}

#define mclex_unary_float_fn(nm) \
mclex_t mclex_ ## nm (mclex_t a) { \
	mclt_t rt = mclt_promote(a->type, MCLT_FLOAT); \
	mclex_t r = mclex_ex(rt, 0); \
	a = mclex_cast(rt, a); \
	sb_append_cs(r->source, # nm "("); \
	sb_append_sb(r->source, a->source); \
	sb_append_cs(r->source, ")"); \
	return r; \
}

#define mclex_binary_float_fn(nm)
mclex_t mclex_ ## nm (mclex_t a1, mclex_t a2) {
	mclt_t rt = mclt_promote(mclt_promote(a1->type, a2->type), MCLT_FLOAT);
	mclex_t r = mclex_ex(rt, 0);
	sb_append_cs(r->source, # nm "(");
	sb_append_sb(r->source, a1->source); \
	sb_append_cs(r->source, ", "); \
	sb_append_sb(r->source, a2->source); \
	sb_append_cs(r->source, ")"); \
	return r;
}

mclex_unary_float_fn(radians);
mclex_unary_float_fn(degrees);
mclex_unary_float_fn(sign);

mclex_t mclex_mix(mclex_t x1, mclex_t x2, mclex_t a) {
	mclt_t rt = mclt_promote(mclt_promote(mclt_promote(x1->type, x2-type), a), MCLT_FLOAT);
	mclex_t r = mclex_ex(rt, 0);
	mclt_t a2t = rt;
	if(mclt_is_scalar(x2->type) && mclt_is_scalar(a->type))
		a2t = MCLT_FLOAT;
	x1 = mclt_cast(rt, x1);
	x2 = mclt_cast(a2t, x2);
	a = mclt_cast(a2t, a);
	sb_append_cs(r->source, "mix(");
	sb_append_sb(r->source, x1->source);
	sb_append_cs(r->source, ", ");
	sb_append_sb(r->source, x2->source);
	sb_append_cs(r->source, ", ");
	sb_append_sb(r->source, a->source);
	sb_append_cs(r->source, ")");
	return r;
}

mclex_t mclex_step(mclex_t a1, mclex_t a2) {
	mclt_t rt = mclt_promote(mclt_promote(a1->type, a2->type), MCLT_FLOAT);
	mclex_t r = mclex_ex(rt, 0);
	a1 = mclex_cast(mclt_is_scalar(a1->type) ? MCLT_FLOAT : rt, a1);
	a2 = mclex_cast(rt, a2);
	sb_append_cs(r->source, "step(");
	sb_append_sb(r->source, a1->source);
	sb_append_cs(r->source, ", ");
	sb_append_sb(r->source, a2->source);
	sb_append_cs(r->source, ")");
	return r;
}
mclex_t mclex_smoothstep(mclex_t a1, mclex_t a2, mclex_t a3) {
	mclt_t rt = mclt_promote(mclt_promote(a1->type, a2->type), MCLT_FLOAT);
	mclt_t a2t = (mclt_is_scalar(a1) && mclt_is_scalar(a2)) ? MCLT_FLOAT : rt; 
	mclex_t r = mclex_ex(rt, 0);
	a1 = mclex_cast(a2t, a1);
	a2 = mclex_cast(a2t, a2);
	a3 = mclex_cast(rt, a3);
	sb_append_cs(r->source, "step(");
	sb_append_sb(r->source, a1->source);
	sb_append_cs(r->source, ", ");
	sb_append_sb(r->source, a2->source);
	sb_append_cs(r->source, ", ");
	sb_append_sb(r->source, a3->source);
	sb_append_cs(r->source, ")");
	return r;
}

mclex_unary_float_fn(mclex_acos);
mclex_unary_float_fn(mclex_acosh);
mclex_unary_float_fn(mclex_acospi);
mclex_unary_float_fn(mclex_asin);
mclex_unary_float_fn(mclex_asinh);
mclex_unary_float_fn(mclex_asinpi);
mclex_unary_float_fn(mclex_atan);
mclex_binary_float_fn(mclex_atan2);
mclex_unary_float_fn(mclex_atanh);
mclex_unary_float_fn(mclex_atanpi);
mclex_binary_float_fn(mclex_atan2pi);
mclex_unary_float_fn(mclex_cbrt);
mclex_unary_float_fn(mclex_ceil);
mclex_binary_float_fn(mclex_copysign);
mclex_unary_float_fn(mclex_cos);
mclex_unary_float_fn(mclex_cosh);
mclex_unary_float_fn(mclex_cospi);
mclex_unary_float_fn(mclex_erfc);
mclex_unary_float_fn(mclex_erf);
mclex_unary_float_fn(mclex_exp);
mclex_unary_float_fn(mclex_exp2);
mclex_unary_float_fn(mclex_exp10);
mclex_unary_float_fn(expm1);
mclex_unary_float_fn(fabs);
mclex_binary_float_fn(fdim);
mclex_unary_float_fn(floor);
mclex_binary_float_fn(fmod);
mclex_binary_float_fn(hypot);
mclex_unary_float_fn(lgamma);
mclex_unary_float_fn(log);
mclex_unary_float_fn(log2);
mclex_unary_float_fn(log10);
mclex_unary_float_fn(log1p);
mclex_unary_float_fn(logb);
mclex_binary_float_fn(maxmag);

mclex_t mad(mclex_t, mclex_t, mclex_t);
mclex_t fma(mclex_t, mclex_t, mclex_t);
mclex_t fmax(mclex_t, mclex_t)
mclex_t fmin(mclex_t, mclex_t)

