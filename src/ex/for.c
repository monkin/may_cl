#ifndef MAY_MCLEX_C_INCLUDE
#	error This file should be included from /may/cl/ex.c
#endif

typedef struct {
	mcl_ex_t init;
	mcl_ex_t condition;
	mcl_ex_t increment;
	mcl_ex_t expression;
	mcl_ex_s self;
} for_data_s;

typedef for_data_s *for_data_t;

void for_push_arguments(void *data, void (*push_fn)(void *, mcl_arg_t), void *dt) {
	for_data_t fd = data;
	if(fd->init)
		mcl_push_arguments(fd->init, push_fn, dt);
	if(fd->condition)
		mcl_push_arguments(fd->condition, push_fn, dt);
	if(fd->increment)
		mcl_push_arguments(fd->increment, push_fn, dt);
	if(fd->expression)
		mcl_push_arguments(fd->expression, push_fn, dt);
}
void for_global_source(void *data, map_t m, ios_t s) {
	for_data_t fd = data;
	if(fd->init)
		mcl_global_source(fd->init, m, s);
	if(fd->condition)
		mcl_global_source(fd->condition, m, s);
	if(fd->increment)
		mcl_global_source(fd->increment, m, s);
	if(fd->expression)
		mcl_global_source(fd->expression, m, s);
}
void for_local_source(void *data, map_t m, ios_t s) {
	for_data_t fd = data;
	if(fd->init)
		mcl_local_source(fd->init, m, s);
	if(fd->condition)
		mcl_local_source(fd->condition, m, s);
	if(fd->increment)
		mcl_local_source(fd->increment, m, s);
	if(fd->expression)
		mcl_local_source(fd->expression, m, s);
}
void for_value_source(void *data, ios_t s) {
	for_data_t fd = data;
	ios_write(s, "for(", 4);
	if(fd->init)
		mcl_value_source(fd->init, s);
	ios_write(s, "; ", 2);
	if(fd->condition)
		mcl_value_source(fd->condition, s);
	ios_write(s, "; ", 2);
	if(fd->increment)
		mcl_value_source(fd->increment, s);
	ios_write(s, ") {\n", 4);
	if(fd->expression)
		mcl_value_source(fd->expression, s);
	ios_write(s, ";\n}", 3);
}

static mcl_ex_vtable_s for_vtable = {
	for_push_arguments,
	for_global_source,
	for_local_source,
	for_value_source
};

mcl_ex_t mcl_for(heap_t h, mcl_ex_t var_init, mcl_ex_t var_cond, mcl_ex_t var_inc, mcl_ex_t ex) {
	for_data_t r = heap_alloc(h, sizeof(for_data_s));
	r->init = var_init;
	r->condition = var_cond;
	r->increment = var_inc;
	r->expression = ex;
	r->self.vtable = &for_vtable;
	r->self.return_type = MCLT_VOID;
	r->self.data = r;
	return &r->self;
}

