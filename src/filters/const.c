
#include "const.h"
#include "../filter.h"
#include "../mcl.h"

#define flt_const_write_number_i(en, tp) case en: *((tp *)dst) = (tp)n; break;

void flt_const_write_number(void *dst, double n, mclt_t t) {
	if(n<0 && mclt_is_unsigned(t))
		err_throw(e_filter_invalid_arguments);
	switch(t) {
	flt_const_write_number_i(MCLT_FLOAT, cl_float);
	flt_const_write_number_i(MCLT_CHAR, cl_char);
	flt_const_write_number_i(MCLT_UCHAR, cl_uchar);
	flt_const_write_number_i(MCLT_SHORT, cl_short);
	flt_const_write_number_i(MCLT_USHORT, cl_ushort);
	flt_const_write_number_i(MCLT_INT, cl_int);
	flt_const_write_number_i(MCLT_UINT, cl_uint);
	flt_const_write_number_i(MCLT_LONG, cl_long);
	flt_const_write_number_i(MCLT_ULONG, cl_ulong);
	default:
		err_throw(e_filter_invalid_arguments);
	}
}

static void flt_init_const(filter_t f) {
	map_t config = json_to_object(f->config);
	json_value_t value = map_get_cs(config, "value");
	long type_size;
	if(!value)
		err_throw(e_filter_invalid_arguments);
	f->type = mclt_parse(json_to_string(map_get_cs(config, "type")));
	type_size = mclt_size(f->type);
	f->controller_data = str_create(f->heap, type_size);
	if(mclt_is_vector(f->type)) {
		json_array_t arr = json_to_array(value);
		int vector_size = mclt_vector_size(f->type);
		type_size/=vector_size;
		if(arr->size!=vector_size) {
			err_throw(e_filter_invalid_arguments);
		} else {
			json_ait_t i;
			char *p;
			mclt_t item_type = mclt_vector_of(f->type);
			for(i = arr->first, p = (char *)f->controller_data; i; i=i->next, p+=type_size)
				flt_const_write_number(i, json_to_number(&i->value), item_type);
		}
	} else
		flt_const_write_number(f->controller_data, json_to_number(value), f->type);
}

static mcl_ex_t flt_get_expression_const(heap_t h, filter_t f, mcl_ex_t point, filter_arguments_t fa) {
	return mcl_const(h,  f->type, f->controller_data);
}

filter_controller_s flt_controller_const = {
	"const", flt_init_const, 0, flt_get_expression_const
};

