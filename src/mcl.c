
#include "mcl.h"
#include <may/core/parser.h>
#include <may/core/syntree.h>
#include <CL/cl.h>

ERR_DEFINE(e_mcl_error, "mCL error", 0);
ERR_DEFINE(e_mclt_error, "Invalid mCL type operation", e_mcl_error);
ERR_DEFINE(e_mclt_parsing_error, "Invalid type name", e_mclt_error);
ERR_DEFINE(e_mclt_size_undefined, "Can't determine type size", e_mclt_error);

mclt_t mclt_vector(mclt_t t, int vector_size) {
	if(!mclt_is_numeric(t) || !(vector_size==2 || vector_size==3 || vector_size==4 || vector_size==8 || vector_size==16))
		err_throw(e_mclt_error);
	return t | (vector_size<<8);
}

mclt_t mclt_pointer(mclt_t t, long mem_type) {
	if(mclt_is_pointer(t))
		err_throw(e_mclt_error);
	return t | MCLT_POINTER | mem_type;
}

mclt_t mclt_vector_of(mclt_t t) {
	if(!mclt_is_vector(t))
		err_throw(e_mclt_error);
	return t & 0xFF;
}

mclt_t mclt_vector_size(mclt_t t) {
	if(!mclt_is_vector(t))
		err_throw(e_mclt_error);
	return (t >> 8) & 0xFF;
}

mclt_t mclt_pointer_to(mclt_t t) {
	if(!mclt_is_pointer(t))
		err_throw(e_mclt_error);
	return t & 0x3F;
}

size_t mclt_sizeof(mclt_t t) {
	if(mclt_is_pointer(t) || mclt_is_image(t))
		return sizeof(cl_mem);
	else if(mclt_is_float(t))
		return 4;
	else if(mclt_is_integer(t))
		return mclt_integer_size(t);
	else if(mclt_is_vector(t)) {
		mclt_t vt = mclt_vector_of(t);
		int vs = mclt_vector_size(t);
		return (vt==MCLT_CHAR && vs==3) ? 4 : mclt_sizeof(vt) * vs;
	}
}

/**
 * Usual ariphmetic conversion.
 * Convertions between vectors allowed.
 */
mclt_t mclt_promote(mclt_t t1, mclt_t t2) {
	if(!(mclt_is_numeric(t1) || mclt_is_vector(t1)) || !(mclt_is_numeric(t2) || mclt_is_vector(t2)))
		err_throw(e_mclt_error);
	if(t1==t2)
		return t1;
	if(mclt_is_vector(t1)) {
		if(mclt_is_vector(t2)) {
			if(mclt_vector_size(t1)==mclt_vector_size(t2))
				return mclt_vector(mclt_promote(mclt_vector_of(t1), mclt_vector_of(t2)), mclt_vector_size(t1));
			else
				err_throw(e_mclt_error);
		} else
			return mclt_vector(mclt_promote(mclt_vector_of(t1), t2), mclt_vector_size(t1));
	}
	if(mclt_is_vector(t2))
		return mclt_promote(t2, t1);
	if(mclt_is_float(t1) || mclt_is_float(t2))
		return MCLT_FLOAT;
	if(mclt_integer_size(t1)!=mclt_integer_size(t2))
		return mclt_integer_size(t1)>mclt_integer_size(t2) ? t1 : t2;
	return mclt_is_unsigned(t1) ? t1 : t2;
}

/**
 * t1 can be explicitly converted to t2
 */
bool mclt_convertable_to(mclt_t t1, mclt_t t2) {
	if(t1==t2)
		return true;
	if(mclt_is_vector(t1) && mclt_is_vector(t2))
		return mclt_vector_size(t1) == mclt_vector_size(t2);
	if(mclt_is_pointer(t1) && mclt_is_pointer(t2))
		return mclt_pointer_type(t1) == mclt_pointer_type(t1);
	return mclt_is_numeric(t1) && mclt_is_numeric(t2);
}

static map_t type_names = 0;
static heap_t type_heap = 0;
static void type_clear() {
	type_heap = heap_delete(type_heap);
}

#define TYPE_APPEND(t, nm) { mclt_t tp = (t); map_set_bin(type_names, &tp, sizeof(tp), str_from_cs(type_heap, nm)); }

static void insert_name(mclt_t t, const char *nm) {
	TYPE_APPEND(t, nm);
}
static void insert_name_p(mclt_t t, const char *name) {
	char buff[64];
	memset(buff, 0, 64); sprintf(buff, "global %s *", name);  TYPE_APPEND(mclt_pointer(t, MCLT_P_GLOBAL), buff);
	memset(buff, 0, 64); sprintf(buff, "local %s *", name);   TYPE_APPEND(mclt_pointer(t, MCLT_P_LOCAL), buff);
	memset(buff, 0, 64); sprintf(buff, "private %s *", name); TYPE_APPEND(mclt_pointer(t, MCLT_P_PRIVATE), buff);
}
static void insert_name_v(mclt_t t, const char *name) {
	int i;
	TYPE_APPEND(t, name);
	insert_name_p(t, name);
	for(i=1; i<=4; i++) {
		char buff[64];
		long vsz = 1 << i;
		mclt_t vt = mclt_vector(t, vsz);
		memset(buff, 0, 64);
		sprintf(buff, "%s%ld", name, vsz);
		TYPE_APPEND(vt, buff);
		insert_name_p(vt, buff);
	}
}

static parser_t mclt_parser;

void mclt_init() {
	if(!type_heap) {
		err_try {
			/* Init types */
			heap_t h = type_heap = heap_create(0);
			type_names = map_create(type_heap);
			insert_name(MCLT_VOID, "void");
			insert_name_p(MCLT_VOID, "void");
			insert_name_v(MCLT_FLOAT, "float");
			insert_name_v(MCLT_CHAR, "char");
			insert_name_v(MCLT_UCHAR, "uchar");
			insert_name_v(MCLT_SHORT, "short");
			insert_name_v(MCLT_USHORT, "ushort");
			insert_name_v(MCLT_INT, "int");
			insert_name_v(MCLT_UINT, "uint");
			insert_name_v(MCLT_LONG, "long");
			insert_name_v(MCLT_ULONG, "ulong");
			insert_name(MCLT_IMAGE_R, "read_only image2d_t");
			insert_name(MCLT_IMAGE_W, "write_only image2d_t");

			/* Init parser */
			parser_t p_spaces = parser_rep(h, parser_cset(h, " \t\r\n"), 1, 0);

			parser_t p_void = parser_named(h, 0, parser_string(h, "void"));

			parser_t p_char = parser_named(h, MCLT_CHAR, parser_string(h, "char"));
			parser_t p_short = parser_named(h, MCLT_SHORT, parser_string(h, "short"));
			parser_t p_int = parser_named(h, MCLT_INT, parser_string(h, "int"));
			parser_t p_long = parser_named(h, MCLT_LONG, parser_string(h, "long"));
			parser_t p_unsigned = parser_named(h, MCLT_UNSIGNED, parser_string(h, "u"));

			parser_t p_float = parser_named(h, MCLT_FLOAT, parser_string(h, "float"));

			parser_t p_integer = parser_and(h,
				parser_maybe(h, p_unsigned),
				parser_or(h, p_char,
					parser_or(h, p_short,
						parser_or(h, p_int, p_long))));
			parser_t p_number = parser_or(h, p_integer, p_float);
			parser_t p_numeric = parser_and(h, p_number,
				parser_maybe(h,
					parser_or(h, parser_named(h, 0x0200, parser_string(h, "2")),
						parser_or(h, parser_named(h, 0x0400, parser_string(h, "4")),
							parser_or(h, parser_named(h, 0x0800, parser_string(h, "8")),
								parser_named(h, 0x1000, parser_string(h, "16")))))));

			parser_t p_read_only = parser_named(h, MCLT_IMAGE_R, parser_string(h, "read_only"));
			parser_t p_write_only = parser_named(h, MCLT_IMAGE_W, parser_string(h, "write_only"));
			parser_t p_image = parser_and(h,
				parser_or(h, p_read_only, p_write_only),
				parser_and(h, p_spaces, parser_string(h, "image2d_t")));

			parser_t p_global = parser_named(h, MCLT_P_GLOBAL | MCLT_POINTER, parser_string(h, "global"));
			parser_t p_local = parser_named(h, MCLT_P_LOCAL | MCLT_POINTER, parser_string(h, "local"));
			parser_t p_private = parser_named(h, MCLT_P_PRIVATE | MCLT_POINTER, parser_string(h, "private"));
			parser_t p_mem_type = parser_or(h, p_global, parser_or(h, p_local, p_private));
			parser_t p_pointer = parser_and(h, p_mem_type,
				parser_and(h, p_spaces,
					parser_and(h, p_numeric,
						parser_and(h, parser_maybe(h, p_spaces), parser_string(h, "*")))));

			mclt_parser = parser_and(h, parser_maybe(h, p_spaces),
				parser_and(h,
					parser_or(h, p_pointer,
						parser_or(h, p_image,
							parser_or(h, p_void, p_numeric))),
					parser_maybe(h, p_spaces)));
		} err_catch {
			type_heap = heap_delete(type_heap);
			err_throw_down();
		}
		map_optimize(type_names);
		atexit(type_clear);
	}
}

str_t mclt_name(mclt_t t) {
	str_t r = map_get_bin(type_names, &t, sizeof(t));
	if(r)
		return r;
	else
		err_throw(e_mclt_error);
}

mclt_t mclt_parse(str_t s) {
	syntree_t st = syntree_create(s);
	if(parser_process(mclt_parser, st)) {
		mclt_t type = 0;
		syntree_node_t i;
		for(i = syntree_begin(st); i; i=syntree_next(i))
			type |= syntree_name(i);
		syntree_delete(st);
		return type;
	} else {
		syntree_delete(st);
		err_throw(e_mclt_parsing_error);
	}
}

mclt_t mclt_parse_cs(const char *s) {
	heap_t h = heap_create(128);
	err_try {
		mclt_t t = mclt_parse(str_from_cs(h, s));
		heap_delete(h);
		return t;
	} err_catch {
		heap_delete(h);
		err_throw_down();
	}
}

long mclt_size(mclt_t t) {
	if(mclt_is_vector(t))
		return mclt_size(mclt_vector_of(t)) * mclt_vector_size(t);
	else if(mclt_is_integer(t))
		return mclt_integer_size(t);
	else if(mclt_is_float(t))
		return sizeof(cl_float);
	else
		err_throw(e_mclt_size_undefined);
}

