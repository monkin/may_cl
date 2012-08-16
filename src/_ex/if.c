#ifndef MAY_MCLEX_C_INCLUDE
#	error This file should be included from /may/cl/ex.c
#endif

typedef struct {
	mcl_ex_t condition;
	mcl_ex_t ex_true;
	mcl_ex_t ex_false;
	mcl_ex_s self;
} if_data_s;

typedef if_data_s *if_data_t;

static void if_push_arguments(void *data, void (*push_fn)(void *, mcl_arg_t), void *dt) {
	if_data_t ifd = data;
	mcl_push_arguments(ifd->condition, push_fn, dt);
	if(ifd->ex_true)
		mcl_push_arguments(ifd->ex_true, push_fn, dt);
	if(ifd->ex_false)
		mcl_push_arguments(ifd->ex_false, push_fn, dt);
}
static void if_global_source(void *data, map_t m, ios_t s) {
	if_data_t ifd = data;
	mcl_global_source(ifd->condition, m, s);
	if(ifd->ex_true)
		mcl_global_source(ifd->ex_true, m, s);
	if(ifd->ex_false)
		mcl_global_source(ifd->ex_false, m, s);
}
static void if_local_source(void *data, map_t m, ios_t s) {
	if_data_t ifd = data;
	mcl_local_source(ifd->condition, m, s);
	if(ifd->ex_true)
		mcl_local_source(ifd->ex_true, m, s);
	if(ifd->ex_false)
		mcl_local_source(ifd->ex_false, m, s);
}
static void if_value_source(void *data, ios_t s) {
	if_data_t ifd = data;
	if(ifd->ex_true) {
		ios_write(s, "if(", 3);
		mcl_value_source(ifd->condition, s);
		ios_write_cs(s, ") {\n\t");
		mcl_value_source(ifd->ex_true, s);
		ios_write(s, ";\n", 2);
		ios_write(s, "}", 1);
		if(ifd->ex_false) {
			ios_write_cs(s, " else {\n\t");
			mcl_value_source(ifd->ex_false, s);
			ios_write_cs(s, ";\n}");
		}
	} else {
		ios_write(s, "if(!", 4);
		mcl_value_source(ifd->condition, s);
		ios_write_cs(s, ") {\n\t");
		mcl_value_source(ifd->ex_false, s);
		ios_write(s, ";\n}", 3);
	}
}

static mcl_ex_vtable_s if_vtable = {
	if_push_arguments,
	if_global_source,
	if_local_source,
	if_value_source
};

mcl_ex_t mcl_if(heap_t h, mcl_ex_t condition, mcl_ex_t ex_true, mcl_ex_t ex_false) {
	assert(ex_true || ex_false);
	if_data_t r = heap_alloc(h, sizeof(if_data_s));
	r->condition = condition;
	r->ex_true = ex_true;
	r->ex_false = ex_false;
	r->self.vtable = &if_vtable;
	r->self.return_type = MCLT_VOID;
	r->self.data = r;
	return &r->self;
}
