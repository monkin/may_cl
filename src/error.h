#ifndef MAY_MCL_ERROR_H
#define MAY_MCL_ERROR_H

#include <may/core/err.h>
#include <CL/cl.h>

#define MCL_OPENCL_ERROR(lname, uname) ERR_DECLARE(e_opencl_ ## lname)
#include "error_codes.h"
#undef MCL_OPENCL_ERROR

void mcl_throw(cl_int);
void mcl_throw_if_error(cl_int);


#endif /* MAY_MCL_ERROR_H */

