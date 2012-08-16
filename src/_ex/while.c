#ifndef MAY_MCLEX_C_INCLUDE
#	error This file should be included from /may/cl/ex.c
#endif

typedef struct {
	mcl_ex_t condition;
	mcl_ex_t expression;
	mcl_ex_s self;
} while_data_s;

typedef while_data_s *while_data_t;

void while_push_arguments(void *data, void (*push_fn)(void *, mcl_arg_t), void *dt) {
	while_data_t wd = data;
	mcl_push_arguments(wd->condition, push_fn, dt);
	mcl_push_arguments(wd->expression, push_fn, dt);
}
void while_global_source(void *data, map_t m, ios_t s) {
	while_data_t wd = data;
	mcl_global_source(wd->condition, m, s);
	mcl_global_source(wd->expression, m, s);
}
void while_local_source(void *data, map_t m, ios_t s) {
	while_data_t wd = data;
	mcl_local_source(wd->condition, m, s);
	mcl_local_source(wd->expression, m, s);
}
void while_value_source(void *data, ios_t s) {
	while_data_t wd = data;
	ios_write_cs(s, "while(");
	mcl_value_source(wd->condition, s);
	ios_write(s, ") {\n", 4);
	mcl_value_source(wd->expression, s);
	ios_write(s, ";\n}", 3);
}

static mcl_ex_vtable_s while_vtable = {
	while_push_arguments,
	while_global_source,
	while_local_source,
	while_value_source
};

mcl_ex_t mcl_while(heap_t h, mcl_ex_t condition, mcl_ex_t expression) {
	while_data_t r = heap_alloc(h, sizeof(while_data_s));
	r->condition = condition;
	r->expression = expression;
	r->self.vtable = &while_vtable;
	r->self.return_type = MCLT_VOID;
	r->self.data = r;
	return &r->self;
}

