
#include "filter_context.h"

flcontext_t flcontext_create(cl_context ctx, floader_t fl) {
	heap_t h = heap_create(0);
	err_try {
		flcontext_t context = heap_alloc(h, sizeof(flcontext_s));
		context->heap = h;
		context->opencl_context = ctx;
		context->file_loader = fl;
		context->objects = map_create(h);
		context->mutex = mutex_create(h, false);
		return context;
	} err_catch {
		h = heap_delete(h);
		err_throw_down();
	}
}
flcontext_t flcontext_delete(flcontext_t ctx) {
	assert(map_length(ctx->objects)==0);
	heap_delete(ctx->heap);
	return 0;
}
