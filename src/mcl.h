
#ifndef MAY_MCL_H
#define MAY_MCL_H

#include <may/core/err.h>
#include <may/core/str.h>
#include <stdbool.h>

/* mclt = mCL type */

ERR_DECLARE(e_mcl_error);
ERR_DECLARE(e_mclt_error);
ERR_DECLARE(e_mclt_parsing_error);
ERR_DECLARE(e_mclt_size_undefined);

enum {
	MCLT_VOID = 0,
	MCLT_FLOAT = 8,
	MCLT_INTEGER = 16,
	MCLT_UNSIGNED = 32,
	MCLT_POINTER = 64,
	MCLT_I_SIZE = 7, /* Mask, int size. size_in_bits = 1 << (3*size) */
	MCLT_V_SIZE = 0xFF00, /* Mask, vector size. */
	MCLT_P_GLOBAL = 0x010000, /* Pointer to global memory */
	MCLT_P_LOCAL = 0x020000,  /* Pointer to local memory */
	MCLT_P_PRIVATE = 0x040000, /* Pointer to private memory */
	MCLT_P_CONSTANT = 0x080000, /* Pointer to contant memory */
	MCLT_IMAGE_R = 0x100000,
	MCLT_IMAGE_W = 0x200000
};

#define MCLT_VECTOR_SHORTCUT(tp) tp ## _2 = tp | 0x0200, tp ## _3 = tp | 0x0300, tp ## _4 = tp | 0x0400, tp ## _8 = tp | 0x0800, tp ## _16 = tp | 0x1000

enum mclt_type_shortcuts {
	MCLT_CHAR = MCLT_INTEGER | 1,
	MCLT_UCHAR = MCLT_UNSIGNED | MCLT_INTEGER | 1,
	MCLT_SHORT = MCLT_INTEGER | 2,
	MCLT_USHORT = MCLT_UNSIGNED | MCLT_INTEGER | 2,
	MCLT_INT = MCLT_INTEGER | 3,
	MCLT_UINT = MCLT_UNSIGNED | MCLT_INTEGER | 3,
	MCLT_LONG = MCLT_INTEGER | 4,
	MCLT_ULONG = MCLT_UNSIGNED | MCLT_INTEGER | 4,
	MCLT_VECTOR_SHORTCUT(MCLT_CHAR),
	MCLT_VECTOR_SHORTCUT(MCLT_UCHAR),
	MCLT_VECTOR_SHORTCUT(MCLT_SHORT),
	MCLT_VECTOR_SHORTCUT(MCLT_USHORT),
	MCLT_VECTOR_SHORTCUT(MCLT_INT),
	MCLT_VECTOR_SHORTCUT(MCLT_UINT),
	MCLT_VECTOR_SHORTCUT(MCLT_LONG),
	MCLT_VECTOR_SHORTCUT(MCLT_ULONG),
	MCLT_VECTOR_SHORTCUT(MCLT_FLOAT)
};

#undef MCLT_VECTOR_SHORTCUT

enum {
	MCL_MEM_GLOBAL = MCLT_P_GLOBAL,
	MCL_MEM_LOCAL = MCLT_P_LOCAL,
	MCL_MEM_PRIVATE = MCLT_P_PRIVATE,
	MCL_MEM_CONSTANT = MCLT_P_CONSTANT
};

typedef long mclt_t;

#define mclt_is_integer(t) (((t) & MCLT_INTEGER) && !((t) & MCLT_V_SIZE))
#define mclt_is_float(t) ((t)==MCLT_FLOAT)
#define mclt_is_pointer(t) ((t) & MCLT_POINTER)
#define mclt_is_vector(t) (((t) & MCLT_V_SIZE) && !mclt_is_pointer(t))
#define mclt_is_vector_of_integer(t) (mclt_is_vector(t) ? mclt_is_integer(mclt_vector_of(t)) : false)
#define mclt_is_vector_of_float(t) (mclt_is_vector(t) ? mclt_is_float(mclt_vector_of(t)) : false)
#define mclt_is_image(t) ((t & MCLT_IMAGE_R) || (t & MCLT_IMAGE_W))
#define mclt_is_void(t) (!(t))
#define mclt_is_numeric(t) (mclt_is_float(t) || mclt_is_integer(t))
#define mclt_is_scalar(t) mclt_is_numeric(t)
#define mclt_is_unsigned(t) ((t)&MCLT_UNSIGNED)
#define mclt_is_signed(t) (!((t)&MCLT_UNSIGNED))
#define mclt_integer_size(t) (1<<((MCLT_I_SIZE & (t))-1))
#define mclt_pointer_type(t) ((t) & (MCL_MEM_GLOBAL | MCL_MEM_LOCAL | MCL_MEM_PRIVATE | MCL_MEM_CONSTANT))
size_t mclt_sizeof(mclt_t);
mclt_t mclt_vector(mclt_t t1, int vector_size);
mclt_t mclt_pointer(mclt_t t, long mem_type);
mclt_t mclt_vector_of(mclt_t t1);
mclt_t mclt_vector_size(mclt_t t1);
mclt_t mclt_pointer_to(mclt_t t1);
/* Type promotion that can convert vectors with different types and the same size. */  
mclt_t mclt_promote(mclt_t t1, mclt_t t2);
/* Type promotion as in OpenCL standard, used for block result */
mclt_t mclt_promote_std(mclt_t t1, mclt_t t2);
/* Result type for boolean operator */ 
mclt_t mclt_promote_bool(mclt_t t);
bool mclt_convertable_to(mclt_t t1, mclt_t t2);
str_t mclt_name(mclt_t);
mclt_t mclt_parse(str_t);
mclt_t mclt_parse_cs(const char *);
long mclt_size(mclt_t);
void mclt_init();

#include "ex.h"


#endif /* MAY_MCL_H */

