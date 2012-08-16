
#include "ex.h"
#include <assert.h>

ERR_DEFINE(e_mcl_ex_invalid_operand, "Invalid operand type", e_mcl_error);
ERR_DEFINE(e_mcl_ex_invalid_function, "Invalid function name", e_mcl_error);
ERR_DEFINE(e_mcl_ex_invalid_type, "Invalid constant type", e_mcl_error);


bool mcl_insert_ptr(map_t m, void *p) {
	char phash[sizeof(void *)];
	int i;
	for(i=0; i<sizeof(void *); i++)
		phash[i] = ((char *)(&p))[sizeof(void *) - i];
	if(map_get_bin(m, phash, sizeof(phash)))
		return false;
	else {
		map_set_bin(m, phash, sizeof(phash), p);
		return true;
	}
}

static char hex_digit(int d) {
	return d<10 ? '0' + d : 'a' + d - 10;
}

static str_t pointer_to_name(heap_t h, char prefix, void *p) {
	int i;
	str_it_t si;
	size_t var_id;
	str_t r;
	var_id = *((size_t *)&p);
	r = str_create(h, sizeof(var_id)*2 + 1);
	si = str_begin(r);
	*(si++) = prefix;
	for(i=0; i<sizeof(var_id)*2; i++, si++, var_id=var_id>>4)
		*si = hex_digit(var_id & 0x0F);
	return r;
}

#define mcl_rule(cnd, result) if(cnd) return (result)
#define mcl_rulex(precnd, cnd, result, exception) if(precnd) { if(cnd) { return (result); } else { err_throw(exception); }; }
#define mcl_else(result) return result

/**
 * t2 can be lossless converted to t1
 */
static bool type_greater(mclt_t t1, mclt_t t2) {
	mcl_rule(t1==t2, true);
	mcl_rule(mclt_is_vector(t1) && mclt_is_vector(t2), mclt_vector_size(t1)==mclt_vector_size(t2) && type_greater(mclt_vector_of(t1), mclt_vector_of(t2)));
	mcl_rule(mclt_is_float(t1), mclt_is_numeric(t2));
	mcl_rule(mclt_is_integer(t1), mclt_integer_size(t1)>=mclt_integer_size(t2));
	mcl_rule(mclt_is_vector(t1), type_greater(mclt_vector_of(t1), t2));
	mcl_rule(mclt_is_pointer(t1) && mclt_is_pointer(t2), (mclt_pointer_to(t1)==mclt_pointer_to(t2) || mclt_is_void(mclt_pointer_to(t1))) && mclt_pointer_type(t1)==mclt_pointer_type(t1));
	mcl_else(false);
}

/**
 * t1 can contains some values from t2
 * t2 can be converted to t1
 */
static bool type_compatible(mclt_t t1, mclt_t t2) {
	mcl_rule(t1==t2, true);
	mcl_rule(mclt_is_vector(t1) && mclt_is_vector(t2), mclt_vector_size(t1)==mclt_vector_size(t2));
	mcl_rule(mclt_is_pointer(t1) && mclt_is_pointer(t2), mclt_pointer_type(t1)==mclt_pointer_type(t1));
	mcl_rule(mclt_is_numeric(t1) && mclt_is_numeric(t2), true);
	mcl_else(false);
}

/**
 * For numerics and vectors only. Returns the greater of both types.
 */
static mclt_t type_max(mclt_t t1, mclt_t t2) {
	mcl_rule(t1==t2, true);
	mcl_rulex(mclt_is_vector(t1) && mclt_is_vector(t2),
		mclt_vector_size(t1)==mclt_vector_size(t2),
		mclt_vector(type_max(mclt_vector_of(t1), mclt_vector_of(t2)), mclt_vector_size(t1)),
		e_mcl_ex_invalid_operand);
	mcl_rule(mclt_is_vector(t1) && mclt_is_numeric(t2),
		mclt_vector(type_max(mclt_vector_of(t1), t2), mclt_vector_size(t1)));
	mcl_rule(mclt_is_numeric(t1) && mclt_is_vector(t2), type_max(t2, t1));
	mcl_rule(mclt_is_float(t1) || mclt_is_float(t2), MCLT_FLOAT);
	mcl_rule((mclt_is_integer(t1) && mclt_is_integer(t2)) ? mclt_integer_size(t1)==mclt_integer_size(t2) : false, t1 | MCLT_UNSIGNED);
	mcl_rule(mclt_is_integer(t1) && mclt_is_integer(t2), mclt_integer_size(t1)>mclt_integer_size(t2) ? t1 : t2);
	err_throw(e_mcl_ex_invalid_operand);
}

static void mcl_push_arguments(mcl_ex_t ex, void (*push_fn)(void *, mcl_arg_t), void *push_fn_arg) {
	ex->vtable->push_arguments(ex->data, push_fn, push_fn_arg);
}
static void mcl_global_source(mcl_ex_t ex, map_t m, ios_t s) {
	ex->vtable->global_source(ex->data, m, s);
}
static void mcl_local_source(mcl_ex_t ex, map_t m, ios_t s) {
	ex->vtable->local_source(ex->data, m, s);
}
static void mcl_value_source(mcl_ex_t ex, ios_t s) {
	ex->vtable->value_source(ex->data, s);
}

#define MAY_MCLEX_C_INCLUDE

#include "ex/call.c"
#include "ex/cast.c"
#include "ex/var.c"
#include "ex/arg.c"
#include "ex/const.c"
#include "ex/if.c"
#include "ex/for.c"
#include "ex/while.c"
#include "ex/seq.c"
#include "ex/random.c"
#include "ex/vector.c"
#include "ex/init.c"

#undef MAY_MCLEX_C_INCLUDE

bool mcl_is_lvalue(mcl_ex_t ex) {
	return ex->vtable==&var_vtable;
}


