#ifndef MAY_MCLEX_C_INCLUDE
#	error This file should be included from /may/cl/ex.c
#endif

typedef struct {
	mcl_arg_t arg;
	mcl_ex_s self;
} arg_data_s;

void arg_push_arguments(void *data, void (*push_fn)(void *, mcl_arg_t), void *push_fn_data) {
	push_fn(push_fn_data, ((arg_data_s *) data)->arg);
}
void arg_global_source(void *data, map_t m, ios_t s) {}
void arg_local_source(void *data, map_t m, ios_t s) {}
void arg_value_source(void *data, ios_t s) {
	str_t name = ((arg_data_s *) data)->arg->name;
	ios_write(s, str_begin(name), str_length(name));
}

static mcl_ex_vtable_s arg_vtable = {
	arg_push_arguments,
	arg_global_source,
	arg_local_source,
	arg_value_source
};

mcl_ex_t mcl_arg(heap_t h, mclt_t tp, mcl_arg_s *arg) {
	arg_data_s *r = heap_alloc(h, sizeof(arg_data_s));
	arg->name = pointer_to_name(h, 'a', r);
	arg->type = tp;
	arg->position = 0;
	r->arg = arg;
	r->self.return_type = tp;
	r->self.data = r;
	r->self.vtable = &arg_vtable;
	return &r->self;
}
