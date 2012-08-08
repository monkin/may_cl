#ifndef MAY_MCLEX_C_INCLUDE
#	error This file should be included from /may/cl/ex.c
#endif

typedef struct {
	mcl_ex_t init;
	mcl_ex_t value;
	mcl_ex_s self;
} init_data_s;

typedef init_data_s *init_data_t;

static void init_push_arguments(void *data, void (*push_fn)(void *, mcl_arg_t), void *dt) {
	init_data_t d = data;
	mcl_push_arguments(d->init, push_fn, dt);
	mcl_push_arguments(d->value, push_fn, dt);
}
static void init_global_source(void *data, map_t m, ios_t s) {
	init_data_t d = data;
	mcl_global_source(d->init, m, s);
	mcl_global_source(d->value, m, s);
}
static void init_local_source(void *data, map_t m, ios_t s) {
	if(mcl_insert_ptr(m, data)) {
		init_data_t d = data;
		mcl_local_source(d->init, m, s);
		mcl_local_source(d->value, m, s);
		mcl_value_source(d->init, s);
		ios_write(s, ";\n", 2);
	}
}
static void init_value_source(void *data, ios_t s) {
	mcl_value_source(((init_data_t) data)->value, s);
}

static mcl_ex_vtable_s init_vtable = {
	init_push_arguments,
	init_global_source,
	init_local_source,
	init_value_source
};

mcl_ex_t mcl_init_ex(heap_t h, mcl_ex_t init_ex, mcl_ex_t value_ex) {
	assert(h && init_ex && value_ex);
	init_data_t r = heap_alloc(h, sizeof(init_data_s));
	r->init = init_ex;
	r->value = value_ex;
	r->self.vtable = &init_vtable;
	r->self.return_type = value_ex->return_type;
	r->self.data = r;
	return &r->self;
}


