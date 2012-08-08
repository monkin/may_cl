
#ifndef MAY_MCL_FILTER_CONTEXT_H
#define MAY_MCL_FILTER_CONTEXT_H

#include "ex.h"
#include "mcl.h"
#include <may/core/json.h>
#include <may/core/map.h>
#include <may/core/floader.h>
#include <may/core/mutex.h>
#include <stdbool.h>
#include <CL/cl.h>

typedef struct {
	heap_t heap;
	cl_context opencl_context;
	floader_t file_loader;
	map_t objects;
} flcontext_s;

typedef flcontext_s *flcontext_t;

flcontext_t flcontext_create(cl_context, floader_t);
flcontext_t flcontext_delete(flcontext_t);

cl_mem flcontext_load_image(flcontext_t, str_t);
cl_mem flcontext_unload_image(flcontext_t, cl_mem);

#endif /* MAY_MCL_FILTER_CONTEXT_H */

