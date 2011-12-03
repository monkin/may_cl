
#include "ex.h"
#include <assert.h>

ERR_DEFINE(e_mcl_ex_invalid_operand, "Invalid operand type", e_mcl_error);


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

typedef enum {
	MCLFT_FUNCTION = 0,
	MCLFT_OPERATOR = 1,
	MCLFT_CUSTOM = 2
} mcl_function_type_t;

typedef struct {
	char *name;
	int args_count;
	mclt_t (*return_type)(size_t, const mclt_t *, mclt_t *);
	mcl_function_type_t type;
} mcl_stdfn_s;

typedef mcl_stdfn_s *mcl_stdfn_t;

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
	mcl_rule(mclt_is_bool(t1), mclt_is_numeric(t2) || mclt_is_pointer(t2));
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
	mcl_rule(mclt_is_bool(t1) && mclt_is_pointer(t2), true);
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

#define type_is_arithmetic(t) (mclt_is_numeric(t) || mclt_is_vector(t))

static mclt_t ret_type_op_set(size_t argc, const mclt_t *args, mclt_t *cast_to) {
	assert(argc==2);
	if(type_compatible(args[0], args[1]))
		return cast_to[0] = args[0];
	else
		err_throw(e_mcl_ex_invalid_operand);
}

static mclt_t ret_type_neg(size_t argc, const mclt_t *args, mclt_t *cast_to) {
	assert(argc==1);
	if(mclt_is_vector(args[0])) {
		if(mclt_is_bool(mclt_vector_of(args[0])))
			return cast_to[0] = mclt_vector(MCLT_INT, mclt_vector_size(args[0]));
		else
			return args[0];
	} else if(mclt_is_numeric(args[0])) {
		if(mclt_is_bool(args[0]))
			return cast_to[0] = MCLT_INT;
		else
			return args[0];
	} else
		err_throw(e_mcl_ex_invalid_operand);
}

static mclt_t ret_type_op_numeric(size_t argc, const mclt_t *args, mclt_t *cast_to) {
	assert(argc==2);
	err_try {
		return cast_to[0] = cast_to[1] = type_max(args[0], args[1]);
	} err_catch {
		if(err_is(e_mclt_error))
			err_replace(e_mcl_ex_invalid_operand);
		err_throw_down();
	}
}

static mclt_t ret_type_op_plus(size_t argc, const mclt_t *args, mclt_t *cast_to) {
	assert(argc==2);
	if((mcl_is_pointer(args[0]) && mcl_is_integer(args[1])) ? !mclt_is_void(mclt_pointer_to(args[0])) : false)
		return args[0];
	if((mcl_is_pointer(args[1]) && mcl_is_integer(args[0])) ? !mclt_is_void(mclt_pointer_to(args[1])) : false)
		return args[1];
	return ret_type_op_numeric(argc, args, cast_to);
}

static mclt_t ret_type_op_minus(size_t argc, const mclt_t *args, mclt_t *cast_to) {
	assert(argc==2);
	if(mcl_is_pointer(args[0])) {
		if(mclt_is_void(mclt_pointer_to(args[0])))
			err_throw(e_mcl_ex_invalid_operand);
		if(args[0]==args[1])
			return MCLT_LONG;
		if((mcl_is_pointer(args[0]) && mcl_is_integer(args[1])) ? !mclt_is_void(mclt_pointer_to(args[0])) : false)
			return args[0];
		err_throw(e_mcl_ex_invalid_operand);
	}
	return ret_type_op_numeric(argc, args, cast_to);
}

static mclt_t ret_type_op_not(size_t argc, const mclt_t *args, mclt_t *cast_to) {
	assert(argc==1);
	mcl_rule(mclt_is_vector(args[0]), mclt_vector(MCLT_BOOL, mclt_vector_size(args[0])));
	mcl_rule(mclt_is_pointer(args[0]) || mclt_is_numeric(args[0]), MCLT_BOOL);
	err_throw(e_mcl_ex_invalid_operand);
}

static mclt_t ret_type_op_equal(size_t argc, const mclt_t *args, mclt_t *cast_to) {
	assert(argc==2);
	if(!type_compatible(args[0], args[1]))
		err_throw(e_mcl_ex_invalid_operand);
	cast_to[0] = cast_to[1] = type_max(args[0], args[1]);
	mcl_rule(mclt_is_vector(args[0]), mclt_vector(MCLT_BOOL, mclt_vector_size(args[0])));
	mcl_rule(mclt_is_vector(args[1]), mclt_vector(MCLT_BOOL, mclt_vector_size(args[1])));
	return MCLT_BOOL;
}

static mclt_t ret_type_op_compare(size_t argc, const mclt_t *args, mclt_t *cast_to) {
	assert(argc==2);
	if(!type_compatible(args[0], args[1]) || mclt_is_image(args[0]) || (mclt_is_pointer(args[0]) && args[0]!=args[1]))
		err_throw(e_mcl_ex_invalid_operand);
	cast_to[0] = cast_to[1] = type_max(args[0], args[1]);
	if(mclt_is_vector(args[0]))
		return mclt_vector(MCLT_BOOL, mclt_vector_size(args[0]));
	if(mclt_is_vector(args[1]))
		return mclt_vector(MCLT_BOOL, mclt_vector_size(args[1]));
	return MCLT_BOOL;
}

static mclt_t ret_type_op_compose(size_t argc, const mclt_t *args, mclt_t *cast_to) {
	assert(argc==2);
	if(type_is_arithmetic(args[0]) && type_is_arithmetic(args[1])) {
		mclt_t t = cast_to[0] = cast_to[1] = type_max(args[0], args[1]);
		return mclt_is_vector(t) ? mclt_vector(MCLT_BOOL, mclt_vector_size(t)) : MCLT_BOOL;
	}
	err_throw(e_mcl_ex_invalid_operand);
}

static mclt_t ret_type_op_binary(size_t argc, const mclt_t *args, mclt_t *cast_to) {
	assert(argc==2);
	if((mclt_is_integer(args[0]) || mclt_is_vector_of_integer(args[0]))
			&& (mclt_is_integer(args[1]) || mclt_is_vector_of_integer(args[1]))) {
		return cast_to[0] = cast_to[1] = type_max(args[0], args[1]);
	}
	err_throw(e_mcl_ex_invalid_operand);
}

static mclt_t ret_type_op_invert(size_t argc, const mclt_t *args, mclt_t *cast_to) {
	assert(argc==1);
	if(mclt_is_integer(args[0]) || mclt_is_vector_of_integer(args[0]))
		return args[0];
	else
		err_throw(e_mcl_ex_invalid_operand);
}

static mclt_t ret_type_op_ternary(size_t argc, const mclt_t *args, mclt_t *cast_to) {
	assert(argc==3);
	int i;
	int vector_size = 0;
	for(i=0; i<3; i++) {
		if(mclt_is_vector(args[i])) {
			if(!vector_size)
				vector_size = mclt_vector_size(args[i]);
			else {
				if(vector_size!=mclt_vector_size(args[i]))
					err_throw(e_mcl_ex_invalid_operand);
			}
		} else if(!mclt_is_numeric(args[0]))
			err_throw(e_mcl_ex_invalid_operand);
	}
	if(vector_size && mclt_is_vector_of_float(args[0]))
		err_throw(e_mcl_ex_invalid_operand);
	cast_to[0] = vector_size ? mclt_vector(MCLT_BOOL, vector_size) : MCLT_BOOL;
	return cast_to[1] = cast_to[2] = type_max(args[1], args[2]);
}

static mclt_t ret_type_op_index(size_t argc, const mclt_t *args, mclt_t *cast_to) {
	assert(argc==2);
	if(mclt_is_pointer(args[0]) ? mclt_pointer_to(args[0])!=MCLT_VOID && mclt_is_integer(args[1]) : false)
		return mclt_pointer_to(args[0]);
	err_throw(e_mcl_ex_invalid_operand);
}

static mclt_t ret_type_work_item(size_t argc, const mclt_t *args, mclt_t *cast_to) {
	assert(argc==1);
	if(type_compatible(MCLT_UINT, args[0])) {
		cast_to[0] = MCLT_UINT;
		return MCLT_ULONG;
	}
	err_throw(e_mcl_ex_invalid_operand);
}

static mclt_t ret_type_work_dim(size_t argc, const mclt_t *args, mclt_t *cast_to) {
	assert(argc==0);
	return MCLT_ULONG;
}

static mclt_t ret_type_abs(size_t argc, const mclt_t *args, mclt_t *cast_to) {
	assert(argc==1);
	if(mclt_is_integer(args[0]) || mclt_is_vector_of_integer(args[0]))
		return args[0] | MCLT_UNSIGNED;
	err_throw(e_mcl_ex_invalid_operand);
}

static mclt_t ret_type_abs_diff(size_t argc, const mclt_t *args, mclt_t *cast_to) {
	assert(argc==2);
	mclt_t ret = type_max(args[0], args[1]);
	if((mclt_is_integer(ret) || mclt_is_vector_of_integer(ret)) && !(ret & MCLT_UNSIGNED)) {
		cast_to[0] = cast_to[1] = ret;
		return ret | MCLT_UNSIGNED;
	}
	err_throw(e_mcl_ex_invalid_operand);
}

static mclt_t ret_type_integer_same(size_t argc, const mclt_t *args, mclt_t *cast_to) {
	assert(argc>=1);
	int i;
	mclt_t ret = args[0];
	for(i=1; i<argc; i++)
		ret = type_max(ret, args[i]);
	if(mclt_is_integer(ret) || mclt_is_vector_of_integer(ret)) {
		for(i=0; i<argc; i++)
			cast_to[i] = ret;
		return ret;
	}
	err_throw(e_mcl_ex_invalid_operand);
}

static mclt_t ret_type_float_same(size_t argc, const mclt_t *args, mclt_t *cast_to) {
	assert(argc>=1);
	int i;
	mclt_t ret = args[0];
	for(i=1; i<argc; i++)
		ret = type_max(ret, args[i]);
	if(mclt_is_integer(ret))
		ret = MCLT_FLOAT;
	else if(mclt_is_vector_of_integer(ret))
		ret = mclt_vector(MCLT_FLOAT, mclt_vector_size(ret));
	if(mclt_is_float(ret) || mclt_is_vector_of_float(ret)) {
		for(i=0; i<argc; i++)
			cast_to[i] = ret;
		return ret;
	}
	err_throw(e_mcl_ex_invalid_operand);
}

static mclt_t ret_type_float_length(size_t argc, const mclt_t *args, mclt_t *cast_to) {
	assert(argc==1 && argc==2);
	mclt_t atype = argc==1 ? args[0] : type_max(args[0], args[1]);
	int i;
	if(mclt_is_integer(atype))
		atype = MCLT_FLOAT;
	else if(mclt_is_vector_of_integer(atype))
		atype = mclt_vector(MCLT_FLOAT, mclt_vector_size(atype));
	else if(!mclt_is_float(atype) && !mclt_is_vector_of_float(atype))
		err_throw(e_mcl_ex_invalid_operand);
	if(mclt_is_vector(atype)) {
		int sz = mclt_vector_size(atype);
		if(sz!=2 && sz!=4)
			err_throw(e_mcl_ex_invalid_operand);
	}
	for(i=0; i<argc; i++)
		cast_to[i] = atype;
	return MCLT_FLOAT;
}

static mclt_t ret_type_read_image(size_t argc, const mclt_t *args, mclt_t *cast_to, mclt_t rtype) {
	assert(argc==2);
	if(args[0]!=MCLT_IMAGE_R)
		err_throw(e_mcl_ex_invalid_operand);
	if(mclt_is_vector(args[1]) ? mclt_vector_size(args[1])==2 : false) {
		if(mclt_is_integer(mclt_vector_of(args[1])))
			cast_to[1] = mclt_vector(MCLT_INT, 2);
	} else
		err_throw(e_mcl_ex_invalid_operand);
	return mclt_vector(rtype, 4);
}

static mclt_t ret_type_read_image_f(size_t argc, const mclt_t *args, mclt_t *cast_to) {
	return ret_type_read_image(argc, args, cast_to, MCLT_FLOAT);
}
static mclt_t ret_type_read_image_i(size_t argc, const mclt_t *args, mclt_t *cast_to) {
	return ret_type_read_image(argc, args, cast_to, MCLT_INT);
}
static mclt_t ret_type_read_image_ui(size_t argc, const mclt_t *args, mclt_t *cast_to) {
	return ret_type_read_image(argc, args, cast_to, MCLT_UINT);
}

static mcl_stdfn_s stdfn_list[] = {
	{"=", 2, ret_type_op_set, MCLFT_OPERATOR},
	{"+", 2, ret_type_op_plus, MCLFT_OPERATOR},
	{"-", 2, ret_type_op_minus, MCLFT_OPERATOR},
	{"*", 2, ret_type_op_numeric, MCLFT_OPERATOR},
	{"/", 2, ret_type_op_numeric, MCLFT_OPERATOR},
	{"%", 2, ret_type_op_numeric, MCLFT_OPERATOR},
	
	{"==", 2, ret_type_op_equal, MCLFT_OPERATOR},
	{"!=", 2, ret_type_op_equal, MCLFT_OPERATOR},
	{"<", 2, ret_type_op_compare, MCLFT_OPERATOR},
	{">", 2, ret_type_op_compare, MCLFT_OPERATOR},
	{"<=", 2, ret_type_op_compare, MCLFT_OPERATOR},
	{">=", 2, ret_type_op_compare, MCLFT_OPERATOR},
	{"!", 1, ret_type_op_not, MCLFT_OPERATOR},
	{"&&", 2, ret_type_op_compose, MCLFT_OPERATOR},
	{"||", 2, ret_type_op_compose, MCLFT_OPERATOR},
	
	{"&", 2, ret_type_op_binary, MCLFT_OPERATOR},
	{"|", 2, ret_type_op_binary, MCLFT_OPERATOR},
	{"^", 2, ret_type_op_binary, MCLFT_OPERATOR},
	{"<<", 2, ret_type_op_binary, MCLFT_OPERATOR},
	{">>", 2, ret_type_op_binary, MCLFT_OPERATOR},
	{">>", 2, ret_type_op_binary, MCLFT_OPERATOR},
	{"~", 1, ret_type_op_invert, MCLFT_OPERATOR},
	
	{"?", 3, ret_type_op_ternary, MCLFT_CUSTOM},
	{"[]", 2, ret_type_op_index, MCLFT_CUSTOM},
	{"neg", 1, ret_type_neg, MCLFT_CUSTOM}, /* unary minus */
	
	{"get_global_id", 1, ret_type_work_item, 0},
	{"get_global_offset", 1, ret_type_work_item, 0},
	{"get_global_size", 1, ret_type_work_item, 0},
	{"get_group_id", 1, ret_type_work_item, 0},
	{"get_local_id", 1, ret_type_work_item, 0},
	{"get_local_size", 1, ret_type_work_item, 0},
	{"get_num_groups", 1, ret_type_work_item, 0},
	{"get_work_dim", 0, ret_type_work_dim, 0},
	
	{"abs", 1, ret_type_abs, 0},
	{"abs_diff", 2, ret_type_abs_diff, 0},
	{"add_sat", 2, ret_type_integer_same, 0},
	{"hadd", 2, ret_type_integer_same, 0},
	{"rhadd", 2, ret_type_integer_same, 0},
	{"clz", 1, ret_type_integer_same, 0},
	{"clamp", 3, ret_type_integer_same, 0},
	{"mad_hi", 3, ret_type_integer_same, 0},
	{"mad_sat", 3, ret_type_integer_same, 0},
	{"max", 2, ret_type_integer_same, 0},
	{"min", 2, ret_type_integer_same, 0},
	{"mul_hi", 2, ret_type_integer_same, 0},
	{"rotate", 2, ret_type_integer_same, 0},
	{"sub_sat", 2, ret_type_integer_same, 0},
	{"upsample", 2, 0, 0},
	
	{"mad24", 3, ret_type_integer_same, 0},
	{"mul24", 2, ret_type_integer_same, 0},
	
	{"clamp", 3, ret_type_float_same, 0},
	{"degrees", 1, ret_type_float_same, 0},
	{"mix", 3, ret_type_float_same, 0},
	{"radians", 1, ret_type_float_same, 0},
	{"step", 2, ret_type_float_same, 0},
	{"smoothstep", 3, ret_type_float_same, 0},
	{"sign", 1, ret_type_float_same, 0},
	
	{"acos", 1, ret_type_float_same, 0},
	{"acosh", 1, ret_type_float_same, 0},
	{"acospi", 1, ret_type_float_same, 0},
	{"asin", 1, ret_type_float_same, 0},
	{"asinh", 1, ret_type_float_same, 0},
	{"asinpi", 1, ret_type_float_same, 0},
	{"atan", 1, ret_type_float_same, 0},
	{"atan2", 2, ret_type_float_same, 0},
	{"atanh", 1, ret_type_float_same, 0},
	{"atanpi", 1, ret_type_float_same, 0},
	{"atan2pi", 2, ret_type_float_same, 0},
	{"cbrt", 1, ret_type_float_same, 0},
	{"ceil", 1, ret_type_float_same, 0},
	{"copysign", 2, ret_type_float_same, 0},
	{"cos", 1, ret_type_float_same, 0},
	{"cosh", 1, ret_type_float_same, 0},
	{"cospi", 1, ret_type_float_same, 0},
	{"erfc", 1, ret_type_float_same, 0},
	{"erf", 1, ret_type_float_same, 0},
	{"exp", 1, ret_type_float_same, 0},
	{"exp2", 1, ret_type_float_same, 0},
	{"exp10", 1, ret_type_float_same, 0},
	
	{"expm1", 1, ret_type_float_same, 0},
	{"fabs", 1, ret_type_float_same, 0},
	{"fdim", 2, ret_type_float_same, 0},
	{"floor", 1, ret_type_float_same, 0},
	{"fma", 3, ret_type_float_same, 0},
	{"fmax", 2, ret_type_float_same, 0},
	{"fmin", 2, ret_type_float_same, 0},
	{"fmod", 2, ret_type_float_same, 0},
	{"fract", 2, 0, 0},
	{"frexp", 2, 0, 0},
	{"hypot", 2, ret_type_float_same, 0},
	{"ilogb", 1, 0, 0},
	{"ldexp", 2, 0, 0},
	{"ldexp", 2, 0, 0},
	{"lgamma", 1, ret_type_float_same, 0},
	{"lgamma_r", 2, 0, 0},
	{"log", 1, ret_type_float_same, 0},
	{"log2", 1, ret_type_float_same, 0},
	{"log10", 1, ret_type_float_same, 0},
	{"log1p", 1, ret_type_float_same, 0},
	{"logb", 1, ret_type_float_same, 0},
	{"mad", 3, ret_type_float_same, 0},
	{"maxmag", 2, ret_type_float_same, 0},
	{"minmag", 2, ret_type_float_same, 0},
	{"modf", 2, 0, 0},
	{"nan", 1, 0, 0},
	{"nextafter", 2, ret_type_float_same, 0},
	{"pow", 2, ret_type_float_same, 0},
	{"pown", 2, 0, 0},
	{"powr", 2, ret_type_float_same, 0},
	{"remainder", 2, ret_type_float_same, 0},
	{"remquo", 3, 0, 0},
	{"rint", 1, ret_type_float_same, 0},
	{"rootn", 2, 0, 0},
	{"round", 1, ret_type_float_same, 0},
	{"rsqrt", 1, ret_type_float_same, 0},
	{"sin", 1, ret_type_float_same, 0},
	{"sincos", 2, 0, 0},
	{"sinh", 1, ret_type_float_same, 0},
	{"sinpi", 1, ret_type_float_same, 0},
	{"sqrt", 1, ret_type_float_same, 0},
	{"tan", 1, ret_type_float_same, 0},
	{"tanh", 1, ret_type_float_same, 0},
	{"tanpi", 1, ret_type_float_same, 0},
	{"tgamma", 1, ret_type_float_same, 0},
	{"trunc", 1, ret_type_float_same, 0},
	
	{"dot", 2, ret_type_float_length, 0},
	{"cross", 2, 0, 0},
	{"distance", 2, ret_type_float_length, 0},
	{"length", 1, ret_type_float_length, 0},
	{"normalize", 1, ret_type_float_same, 0},
	{"fast_distance", 2, ret_type_float_length, 0},
	{"fast_length", 1, ret_type_float_length, 0},
	{"fast_normalize", 1, ret_type_float_same, 0},
	{"isequal", 2, 0, 0},
	{"isnotequal", 2, 0, 0},
	{"isgreater", 2, 0, 0},
	{"isgreaterequal", 2, 0, 0},
	{"isless", 2, 0, 0},
	{"islessequal", 2, 0, 0},
	{"islessgreater", 2, 0, 0},
	{"isfinite", 1, 0, 0},
	{"isinf", 1, 0, 0},
	{"isnan", 1, 0, 0},
	{"isnormal", 1, 0, 0},
	{"isordered", 2, 0, 0},
	{"isunordered", 2, 0, 0},
	{"signbit", 1, 0, 0},
	{"any", 1, 0, 0},
	{"all", 1, 0, 0},
	{"bitselect", 3, 0, 0},
	{"select", 3, 0, 0},
	{"shufle", 2, 0, 0},
	{"shufle2", 3, 0, 0},
	
	/* suffixes
	* i - integer
	* ui - unsigned integer
	* f - float
	* n - nearest neighbor interpolation
	* l - linear interpolation
	*/
	{"read_image_fn", 2, ret_type_read_image_f, MCLFT_CUSTOM},
	{"read_image_in", 2, ret_type_read_image_i, MCLFT_CUSTOM},
	{"read_image_uin", 2, ret_type_read_image_ui, MCLFT_CUSTOM},
	{"read_image_fl", 2, ret_type_read_image_f, MCLFT_CUSTOM},
	{"read_image_il", 2, ret_type_read_image_i, MCLFT_CUSTOM},
	{"read_image_uil", 2, ret_type_read_image_ui, MCLFT_CUSTOM},
	{"write_image_f", 3, 0, MCLFT_FUNCTION},
	{"write_image_i", 3, 0, MCLFT_FUNCTION},
	{"write_image_ui", 3, 0, MCLFT_FUNCTION},
	{"get_image_width", 1, 0, MCLFT_FUNCTION},
	{"get_image_height", 1, 0, MCLFT_FUNCTION}

};

/*** experssion functions ***/
static void mcl_push_arguments(mcl_ex_t ex, str_t (*push_fn)(void *, mcl_arg_t), void *push_fn_arg) {
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

/*** call_internal ***/

typedef struct {
	mcl_stdfn_t fn;
	mcl_ex_t args[4];
	mcl_ex_s ex;
} call_internal_data_s;

typedef call_internal_data_s *call_internal_data_t;

static void call_internal_push_arguments(void *data, str_t (*push_fn)(void *, mcl_arg_t), void *push_fn_arg) {
	int i;
	call_internal_data_t cid = (call_internal_data_t) data;
	for(i=0; i<cid->fn->args_count; i++)
		mcl_push_arguments(cid->args[i], push_fn, push_fn_arg);
}
static void call_internal_global_source(void *data, map_t m, ios_t s) {
	int i;
	call_internal_data_t cid = (call_internal_data_t) data;
	for(i=0; i<cid->fn->args_count; i++)
		mcl_global_source(cid->args[i], m, s);
}
static void call_internal_local_source(void *data, map_t m, ios_t s) {
	int i;
	call_internal_data_t cid = (call_internal_data_t) data;
	for(i=0; i<cid->fn->args_count; i++)
		mcl_local_source(cid->args[i], m, s);
}
static void call_internal_value_source(void *data, ios_t s) {
	call_internal_data_t cid = (call_internal_data_t) data;
	int i;
	switch(cid->fn->type) {
	case MCLFT_FUNCTION:
		ios_write(s, cid->fn->name, strlen(cid->fn->name));
		ios_write(s, "(", 1);
		for(i=0; i<cid->fn->args_count; i++) {
			if(i)
				ios_write(s, ", ", 2);
			mcl_value_source(cid->args[i], s);
		}
		ios_write(s, ")", 1);
		break;
	case MCLFT_OPERATOR:
		switch(cid->fn->args_count) {
		case 1:
			ios_write(s, "(", 1);
			ios_write(s, cid->fn->name, strlen(cid->fn->name));
			mcl_value_source(cid->args[0], s);
			ios_write(s, ")", 1);
			break;
		case 2:
			ios_write(s, "(", 1);
			mcl_value_source(cid->args[0], s);
			ios_write(s, cid->fn->name, strlen(cid->fn->name));
			mcl_value_source(cid->args[1], s);
			ios_write(s, ")", 1);
			break;
		}
		break;
	case MCLFT_CUSTOM:
		switch(cid->fn->name[0]) {
		case '[': /* [] - operator */
			ios_write(s, "(", 1);
			mcl_value_source(cid->args[0], s);
			ios_write(s, "[", 1);
			mcl_value_source(cid->args[1], s);
			ios_write(s, "])", 2);
			break;
		case '?': /* x ? y : z */
			ios_write(s, "(", 1);
			mcl_value_source(cid->args[0], s);
			ios_write(s, " ? ", 3);
			mcl_value_source(cid->args[1], s);
			ios_write(s, " : ", 3);
			mcl_value_source(cid->args[2], s);
			ios_write(s, ")", 1);
			break;
		case 'n': /* neg(x) - unary minus*/
			ios_write_cs(s, "(-");
			mcl_value_source(cid->args[1], s);
			ios_write_cs(s, ")");
			break;
		default: { /* read image function */
				bool is_linear = false;
				switch(cid->fn->name[11]) {
				case 'f': /* read_image_f* */
					ios_write_cs(s, "read_image_f(");
					is_linear = cid->fn->name[12]=='l';
					break;
				case 'i': /* read_image_i* */
					ios_write_cs(s, "read_image_i(");
					is_linear = cid->fn->name[12]=='l';
					break;
				case 'u': /* read_image_ui* */
					ios_write_cs(s, "read_image_ui(");
					is_linear = cid->fn->name[13]=='l';
					break;
				}
				mcl_value_source(cid->args[0], s);
				ios_write_cs(s, is_linear ? ", sampler_linear, " : ", sampler_nearest, ");
				mcl_value_source(cid->args[1], s);
				ios_write_cs(s, ")");
			}
		}
	}
}

/*typedef struct {
	mcl_stdfn_t fn;
	mcl_ex_t args[4];
	mcl_ex_s ex;
} call_internal_data_s;*/
	
static mcl_ex_vtable_s call_internal_vtable = {
	call_internal_push_arguments,
	call_internal_global_source,
	call_internal_local_source,
	call_internal_value_source
};

static mcl_ex_t mcl_call_internal(heap_t h, mcl_stdfn_t fn, mcl_ex_t *args) {
	call_internal_data_t r = heap_alloc(h, sizeof(call_internal_data_s));
	mclt_t types[4];
	mclt_t cast[4];
	int i;
	for(i=0; i<fn->args_count; i++)
		types[i] = cast[i] = args[i]->return_type;

	r->fn = fn;
	r->ex.return_type = r->fn->return_type(fn->args_count, types, cast);
	r->ex.vtable = &call_internal_vtable;
	r->ex.data = r;

	for(i=0; i<fn->args_count; i++)
		r->args[i] = types[i]==cast[i] ? args[i] : mcl_cast(h, cast[i], args[i]);

	return &r->ex;
}

mcl_ex_t mcl_call(heap_t h, str_t nm, ...) {

}

mcl_ex_t mcl_call_cs(heap_t h, const char *nm, ...) {

}
