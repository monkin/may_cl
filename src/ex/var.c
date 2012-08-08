#ifndef MAY_MCLEX_C_INCLUDE
#	error This file should be included from /may/cl/ex.c
#endif

typedef struct {
	str_t name;
	mcl_ex_t expr;
	mcl_ex_s self;
} var_data_s;

typedef var_data_s *var_data_t;

static void var_push_arguments(void *data, void (*push_fn)(void *, mcl_arg_t), void *push_fn_dt) {
	mcl_push_arguments(((var_data_t)data)->expr, push_fn, push_fn_dt);
}
static void var_global_source(void *data, map_t m, ios_t s) {
	mcl_global_source(((var_data_t)data)->expr, m, s);
}
static void var_local_source(void *data, map_t m, ios_t s) {
	if(mcl_insert_ptr(m, data)) {
		var_data_t vd = data;
		str_t type_name = mclt_name(vd->expr->return_type);
		ios_write(s, str_begin(type_name), str_length(type_name));
		ios_write(s, " ", 1);
		ios_write(s, str_begin(vd->name), str_length(vd->name));
		ios_write(s, " = ", 3);
		mcl_local_source(vd->expr, m, s);
		ios_write(s, ";\n", 2);
	}
}
static void var_value_source(void *data, ios_t s) {
	var_data_t vd = data;
	ios_write(s, str_begin(vd->name), str_length(vd->name));
}

static mcl_ex_vtable_s var_vtable = {
	var_push_arguments,
	var_global_source,
	var_local_source,
	var_value_source
};


mcl_ex_t mcl_var(heap_t h, mcl_ex_t ex) {
	assert(h && ex);
	var_data_t data = heap_alloc(h, sizeof(var_data_s));
	data->name = pointer_to_name(h, 'v', data);
	data->expr = ex;
	data->self.vtable = &var_vtable;
	data->self.data = data;
	data->self.return_type = ex->return_type;
	return &data->self;
}

