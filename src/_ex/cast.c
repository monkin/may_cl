#ifndef MAY_MCLEX_C_INCLUDE
#	error This file should be included from /may/cl/ex.c
#endif

typedef struct {
	mclt_t cast_to;
	mcl_ex_t expr;
	mcl_ex_s res_expression;
} cast_data_s;

typedef cast_data_s *cast_data_t;

static void cast_push_arguments(void *data, void (*push_fn)(void *, mcl_arg_t), void *push_fn_dt) {
	mcl_push_arguments(((cast_data_t)data)->expr, push_fn, push_fn_dt);
}
static void cast_global_source(void *data, map_t m, ios_t s) {
	mcl_global_source(((cast_data_t)data)->expr, m, s);
}
static void cast_local_source(void *data, map_t m, ios_t s) {
	mcl_local_source(((cast_data_t)data)->expr, m, s);
}
static void cast_value_source(void *data, ios_t s) {
	cast_data_t cd = data;
	if(mclt_is_vector(cd->cast_to)) {
		str_t cast_to_name = mclt_name(cd->cast_to);
		ios_write_cs(s, "convert_");
		ios_write(s, str_begin(cast_to_name), str_length(cast_to_name));
		ios_write(s, "(", 1);
		mcl_value_source(cd->expr, s);
		ios_write(s, ")", 1);
	} else if(mclt_is_pointer(cd->cast_to) || mclt_is_scalar(cd->cast_to)) {
		str_t cast_to_name = mclt_name(cd->cast_to);
		ios_write(s, "((", 2);
		ios_write(s, str_begin(cast_to_name), str_length(cast_to_name));
		ios_write(s, ")", 1);
		mcl_value_source(cd->expr, s);
		ios_write(s, ")", 1);
	}
}

static mcl_ex_vtable_s cast_vtable = {
	cast_push_arguments,
	cast_global_source,
	cast_local_source,
	cast_value_source
};

mcl_ex_t mcl_cast(heap_t h, mclt_t t, mcl_ex_t ex) {
	if(ex->return_type==t)
		return ex;
	else if(type_compatible(t, ex->return_type)) {
		cast_data_t r = heap_alloc(h, sizeof(cast_data_s));
		r->cast_to = t;
		r->expr = ex;
		r->res_expression.return_type = t;
		r->res_expression.vtable = &cast_vtable;
		r->res_expression.data = r;
		return &r->res_expression;
	} else
		err_throw(e_mcl_ex_invalid_operand);
}
