#ifndef MAY_MCLEX_C_INCLUDE
#	error This file should be included from /may/cl/ex.c
#endif

typedef struct {
	mcl_ex_t expr;
	unsigned int size;
	unsigned int indexes[16];
	mcl_ex_s rexpr;
} vec_data_s;

typedef vec_data_s *vec_data_t;

static void vec_push_arguments(void *data, void (*push_fn)(void *, mcl_arg_t), void *push_fn_dt) {
	mcl_push_arguments(((vec_data_t)data)->expr, push_fn, push_fn_dt);
}
static void vec_global_source(void *data, map_t m, ios_t s) {
	mcl_global_source(((vec_data_t)data)->expr, m, s);
}
static void vec_local_source(void *data, map_t m, ios_t s) {
	mcl_local_source(((vec_data_t)data)->expr, m, s);
}
static void vec_value_source(void *data, ios_t s) {
	static const char *indexes[] = {"0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "a", "b", "c", "d", "e", "f"};
	mcl_value_source(((vec_data_t) data)->expr, s);
	ios_write_cs(s, ".s");
	int i;
	for(i=0; i<((vec_data_t) data)->size; i++)
		ios_write_cs(s, indexes[((vec_data_t) data)->indexes[i]]);
}

static mcl_ex_vtable_s vec_vtable = {
	vec_push_arguments,
	vec_global_source,
	vec_local_source,
	vec_value_source
};

mcl_ex_t mcl_vector_select(heap_t h, mcl_ex_t ex, unsigned int rsz, const unsigned int *indexes) {
	int vsz = mclt_vector_size(ex->return_type);
	if(mclt_is_vector(ex->return_type) ? rsz>vsz : true)
		err_throw(e_mcl_ex_invalid_operand);
	int i;
	for(i=0; i<rsz; i++)
		if(indexes[i]>=vsz)
			err_throw(e_mcl_ex_invalid_operand);
	vec_data_t res = heap_alloc(h, sizeof(mcl_ex_s));
	res->expr = ex;
	res->size = rsz;
	for(i=0; i<rsz; i++)
		res->indexes[i] = indexes[i];
	if(rsz==1)
		res->rexpr.return_type = mclt_vector_of(ex->return_type);
	else
		res->rexpr.return_type = mclt_vector(mclt_vector_of(ex->return_type), rsz);
	res->rexpr.vtable = &vec_vtable;
	res->rexpr.data = res;
	return &res->rexpr;
}

mcl_ex_t mcl_vector_select_1(heap_t h, mcl_ex_t ex, unsigned int i) {
	return mcl_vector_select(h, ex, 1, &i);
}
mcl_ex_t mcl_vector_select_2(heap_t h, mcl_ex_t ex, unsigned int i0, unsigned int i1) {
	unsigned int i[2];
	i[0] = i0; i[1] = i1;
	return mcl_vector_select(h, ex, 2, i);
}
mcl_ex_t mcl_vector_select_4(heap_t h, mcl_ex_t ex, unsigned int i0, unsigned int i1, unsigned int i2, unsigned int i3) {
	unsigned int i[4];
	i[0] = i0; i[1] = i1; i[2] = i2; i[3] = i3;
	return mcl_vector_select(h, ex, 4, i);
}
mcl_ex_t mcl_vector_select_8(heap_t h, mcl_ex_t ex, unsigned int i0, unsigned int i1, unsigned int i2, unsigned int i3, unsigned int i4, unsigned int i5, unsigned int i6, unsigned int i7) {
	unsigned int i[8];
	i[0] = i0; i[1] = i1; i[2] = i2; i[3] = i3;
	i[4] = i4; i[5] = i5; i[6] = i6; i[7] = i7;
	return mcl_vector_select(h, ex, 8, i);
}
mcl_ex_t mcl_vector_lo(heap_t h, mcl_ex_t ex) {
	static unsigned int i[] = {0, 1, 2, 3, 4, 5, 6, 7};
	return mcl_vector_select(h, ex, mclt_vector_size(ex->return_type)/2, i);
}
mcl_ex_t mcl_vector_hi(heap_t h, mcl_ex_t ex) {
	static unsigned int i[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
	unsigned int vsz = mclt_vector_size(ex->return_type)/2;
	return mcl_vector_select(h, ex, vsz, i+vsz);
}
mcl_ex_t mcl_vector_odd(heap_t h, mcl_ex_t ex) {
	static unsigned int i[] = {1, 3, 5, 7, 9, 11, 13, 15};
	return mcl_vector_select(h, ex, mclt_vector_size(ex->return_type)/2, i);
}
mcl_ex_t mcl_vector_even(heap_t h, mcl_ex_t ex) {
	static unsigned int i[] = {0, 2, 4, 6, 8, 10, 12, 14};
	return mcl_vector_select(h, ex, mclt_vector_size(ex->return_type)/2, i);
}

