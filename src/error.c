
#include "error.h"
#include "mcl.h"

ERR_DEFINE(e_opencl_error, "OpenCL error", e_mcl_error);

#define MCL_OPENCL_ERROR(lname, uname) ERR_DEFINE(e_opencl_ ## lname, "OpenCL error: " # lname, e_opencl_error)
#include "error_codes.h"
#undef MCL_OPENCL_ERROR

#define MCL_OPENCL_ERROR(lname, uname) \
	case CL_ ## uname:                 \
		err_throw(e_opencl_ ## lname); \
		break

void mcl_throw(cl_int code) {
	switch(code) {
#	include "error_codes.h"
	default:
		err_throw(e_opencl_error);
		break;
	}
}

#undef MCL_OPENCL_ERROR

void mcl_throw_if_error(cl_int code) {
	if(code!=CL_SUCCESS)
		mcl_throw(code);
}
