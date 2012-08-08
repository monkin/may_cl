#ifndef MAY_MCLEX_C_INCLUDE
#	error This file should be included from /may/cl/ex.c
#endif

#include <stdarg.h>

typedef struct {
	size_t ex_count;
	mcl_ex_t *ex;
	mcl_ex_s self;
} seq_data_s;

typedef seq_data_s *seq_data_t;

static void seq_push_arguments(void *data, void (*push_fn)(void *, mcl_arg_t), void *dt) {
	seq_data_t sd = data;
	size_t i;
	for(i=0; i<sd->ex_count; i++)
		mcl_push_arguments(sd->ex[i], push_fn, dt);
}
static void seq_global_source(void *data, map_t m, ios_t s) {
	seq_data_t sd = data;
	size_t i;
	for(i=0; i<sd->ex_count; i++)
		mcl_global_source(sd->ex[i], m, s);
}
static void seq_local_source(void *data, map_t m, ios_t s) {
	seq_data_t sd = data;
	size_t i;
	for(i=0; i<sd->ex_count; i++)
		mcl_local_source(sd->ex[i], m, s);
}

static void seq_value_source(void *data, ios_t s) {
	seq_data_t sd = data;
	size_t i;
	ios_write(s, "{\n", 2);
	for(i=0; i<sd->ex_count; i++) {
		mcl_value_source(sd->ex[i], s);
		ios_write(s, ";\n", 2);
	}
	ios_write(s, "}\n", 2);
}

static mcl_ex_vtable_s seq_vtable = {
	seq_push_arguments,
	seq_global_source,
	seq_local_source,
	seq_value_source
};

mcl_ex_t mcl_seq(heap_t h, size_t n, ...) {
	size_t i;
	va_list ap;
	seq_data_t r = heap_alloc(h, sizeof(seq_data_s));
	r->ex_count = n;
	r->ex = heap_alloc(h, sizeof(mcl_ex_t[n]));
	va_start(ap, n);
	for(i=0; i<n; i++)
		r->ex[i] = va_arg(ap, mcl_ex_t);
	va_end(ap);
	r->self.vtable = &seq_vtable;
	r->self.return_type = MCLT_VOID;
	r->self.data = r;
	return &r->self;
}

