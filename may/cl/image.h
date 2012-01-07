
#ifndef MAY_MCL_IMAGE_H
#define MAY_MCL_IMAGE_H

#include "../lib/str.h"
#include "../lib/err.h"
#include <CL/cl.h>

ERR_DECLARE(e_mcl_image_loading);

cl_mem mcl_image_create(cl_context context, str_t data);
cl_mem mcl_image_create_bin(cl_context context, const void *data, size_t len);
cl_mem mcl_image_delete(cl_mem);

#endif /* MAY_MCL_IMAGE_H */
