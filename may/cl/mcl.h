
#ifndef MAY_MCL_H
#define MAY_MCL_H

#include "../lib/parser.h"
#include <stdbool.h>

/* mclt = mCL type */

ERR_DECLARE(e_mcl_error);
ERR_DECLARE(e_mclt_error);

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
	MCLT_IMAGE_R = 0x080000,
	MCLT_IMAGE_W = 0x100000
};

enum mclt_type_shortcuts {
	MCLT_BOOL = MCLT_INTEGER,
	MCLT_CHAR = MCLT_INTEGER | 1,
	MCLT_UCHAR = MCLT_UNSIGNED | MCLT_INTEGER | 1,
	MCLT_SHORT = MCLT_INTEGER | 2,
	MCLT_USHORT = MCLT_UNSIGNED | MCLT_INTEGER | 2,
	MCLT_INT = MCLT_INTEGER | 3,
	MCLT_UINT = MCLT_UNSIGNED | MCLT_INTEGER | 3,
	MCLT_LONG = MCLT_INTEGER | 4,
	MCLT_ULONG = MCLT_UNSIGNED | MCLT_INTEGER | 4
};

enum {
	MCL_MEM_GLOBAL = MCLT_P_GLOBAL,
	MCL_MEM_LOCAL = MCLT_P_LOCAL,
	MCL_MEM_PRIVATE = MCLT_P_PRIVATE
};

typedef long mclt_t;

#define mclt_is_integer(t) (((t) & MCLT_INTEGER) && !((t) & MCLT_V_SIZE))
#define mclt_is_float(t) ((t)==MCLT_FLOAT)
#define mclt_is_bool(t) ((t)==MCLT_BOOL)
#define mclt_is_pointer(t) ((t) & MCLT_POINTER)
#define mclt_is_vector(t) (((t) & MCLT_V_SIZE) && !mclt_is_pointer(t))
#define mclt_is_image(t) ((t & MCLT_IMAGE_R) || (t & MCLT_IMAGE_W))
#define mclt_is_void(t) (!(t))
#define mclt_is_numeric(t) (mclt_is_float(t) || mclt_is_integer(t))
#define mclt_is_unsigned(t) ((t)&MCLT_UNSIGNED)
#define mclt_is_signed(t) (!((t)&MCLT_UNSIGNED))
#define mclt_integer_size(t) (1<<((MCLT_I_SIZE & (t))-1))
#define mclt_pointer_type(t) ((t) & (MCL_MEM_GLOBAL | MCL_MEM_LOCAL | MCL_MEM_PRIVATE))
mclt_t mclt_vector(mclt_t t1, int vector_size);
mclt_t mclt_pointer(mclt_t t, long mem_type);
mclt_t mclt_vector_of(mclt_t t1);
mclt_t mclt_vector_size(mclt_t t1);
mclt_t mclt_pointer_to(mclt_t t1);
mclt_t mclt_op_result(mclt_t, mclt_t);
str_t mclt_name(mclt_t);
void mclt_init();

#include "ex.h"


#endif /* MAY_MCL_H */

