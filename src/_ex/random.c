
#ifndef MAY_MCLEX_C_INCLUDE
#	error This file should be included from /may/cl/ex.c
#endif

static void random_push_arguments(void *data, void (*push_fn)(void *, mcl_arg_t), void *dt) {}
static void random_global_source(void *data, map_t m, ios_t s) {
	static const char *sizes[] = {"", "2", "4", "8", "16"};
	static const char *seed_indexes[] = {".s0", ".s01", ".s0123", ".lo", ""};
	if(mcl_insert_ptr(m, random_global_source)) {
		int i;
		ios_write_cs(s, "private uint16 may_random_seed = 0;\n"
			"private bool initialized = false;\n");
		for(i=0; i<5; i++) {
			ios_write_cs(s, "float");
				ios_write_cs(s, sizes[i]);
				ios_write_cs(s, " may_random")
				ios_write_cs(s, sizes[i]);
				ios_write_cs(s, "() {\n");
			ios_write_cs(s, "if(!initialized) {\n"
					"may_random_seed = (uint)get_global_id() * (uint16)(74986, 4751, 76951, 7135, 87963, 120, 8740, 84869, 48118, 72094, 1566, 1961, 73537, 58459, 16800, 87426);\n"
					"initialized = true;\n"
				"}\n");
			ios_write_cs(s, "return may_random_seed");
				ios_write_cs(s, seed_indexes[i]);
				ios_write_cs(s, " = ((float");
				ios_write_cs(s, sizes[i]);
				ios_write_cs(s, ")((uint");
				ios_write_cs(s, sizes[i]);
				ios_write_cs(s, ")(may_random_seed");
				ios_write_cs(s, seed_indexes[i]);
				ios_write_cs(s, "*69069 + 5))) / 4294967295.0;\n};\n");
		}
	}
}
static void random_local_source(void *data, map_t m, ios_t s) {}
static void random_value_source(void *data, ios_t s) {
	ios_write_cs(s, "may_random");
	switch((size_t) data) {
	case 1:
		break;
	case 2:
		ios_write_cs(s, "2");
		break;
	case 4:
		ios_write_cs(s, "4");
		break;
	case 8:
		ios_write_cs(s, "8");
		break;
	case 16:
		ios_write_cs(s, "16");
		break;
	}
	ios_write_cs(s, "()");
}

static mcl_ex_vtable_s random_vtable = {
	random_push_arguments,
	random_global_source,
	random_local_source,
	random_value_source
};

mcl_ex_t mcl_random(heap_t h, size_t vector_size) {
	mcl_ex_t r = heap_alloc(h, sizeof(mcl_ex_s));
	r->return_type = MCLT_FLOAT;
	r->data = (void *) vector_size;
	r->vtable = &random_vtable;
	return r;
}