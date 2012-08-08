
#include "program.h"
#include "error.h"
#include <stdbool.h>

typedef struct {
	map_t map;
	ios_t stream;
} push_fn_data_s;

typedef push_fn_data_s *push_fn_data_t;

static void mcl_prog_push_fn(void *data, mcl_arg_t argument) {
	push_fn_data_t d = data;
	if(!map_get_bin(d->map, &argument, sizeof(argument))) {
		size_t num = argument->position = map_length(d->map);
		map_set_bin(d->map, &argument, sizeof(argument), (void *) num);
		if(num)
			ios_write(d->stream, ", ", 2);
		str_t type_name = mclt_name(argument->type);
		ios_write(d->stream, str_begin(type_name), str_length(type_name));
		ios_write(d->stream, " ", 1);
		ios_write(d->stream, str_begin(argument->name), str_length(argument->name));
	}
}

cl_program mcl_program_create(cl_context ctx, mcl_ex_t ex) {
	heap_t h = heap_create(0);
	ios_t source_stream = 0;
	cl_program program = 0;
	err_try {
		map_t variables = map_create(h);

		source_stream = ios_mem_create();
		ex->vtable->global_source(ex->data, map_create(h), source_stream);
		ios_write_cs(source_stream, "\nkernel void kernel_function(");

		push_fn_data_t pd = heap_alloc(h, sizeof(push_fn_data_s));
		pd->map = variables;
		pd->stream = source_stream;
		ex->vtable->push_arguments(ex->data, mcl_prog_push_fn, pd);

		ios_write(source_stream, ") {\n", 4);
		ex->vtable->local_source(ex->data, map_create(h), source_stream);
		ex->vtable->value_source(ex->data, source_stream);
		ios_write(source_stream, ";\n}\n", 4);
		str_t source = ios_mem_to_string(source_stream, h);
		source_stream = ios_close(source_stream);

		// Create OpenCL program
		const char *source_cs = str_begin(source);
		const size_t source_len = str_length(source);
		cl_int code = CL_SUCCESS;
		program = clCreateProgramWithSource(ctx, 1, &source_cs, &source_len, &code);
		mcl_throw_if_error(code);
		h = heap_delete(h);
	} err_catch {
		ios_close(source_stream);
		h = heap_delete(h);
		err_throw_down();
	}
	return program;
}

cl_program mcl_program_build(cl_program p) {
	mcl_throw_if_error(clBuildProgram(p, 0, 0, 0, 0, 0));
	return p;
}

str_t mcl_program_source(heap_t h, cl_program p) {
	size_t str_size;
	str_t source = 0;
	mcl_throw_if_error(clGetProgramInfo(p, CL_PROGRAM_SOURCE, 0, 0, &str_size));
	source = str_create(h, str_size-1);
	mcl_throw_if_error(clGetProgramInfo(p, CL_PROGRAM_SOURCE, str_size, str_begin(source), 0));
	return source;
}

cl_device_id mcl_program_device(cl_program p) {
	cl_device_id device[1];
	mcl_throw_if_error(clGetProgramInfo(p, CL_PROGRAM_DEVICES, sizeof(device), &device, 0));
	return device[0];
}

str_t mcl_program_log(heap_t h, cl_program p) {
	str_t log;
	size_t log_size;
	cl_device_id device = mcl_program_device(p);
	mcl_throw_if_error(clGetProgramBuildInfo(p, device, CL_PROGRAM_BUILD_LOG, 0, 0, &log_size));
	log = str_create(h, log_size-1);
	mcl_throw_if_error(clGetProgramBuildInfo(p, device, CL_PROGRAM_BUILD_LOG, log_size, str_begin(log), 0));
	return log;
}

cl_program mcl_program_delete(cl_program p) {
	if(p)
		clReleaseProgram(p);
	return 0;
}

cl_kernel mcl_kernel_create(cl_program p) {
	cl_int code = CL_SUCCESS;
	cl_kernel k = clCreateKernel(p, "kernel_function",  &code);
	mcl_throw_if_error(code);
	return k;
}
cl_kernel mcl_kernel_delete(cl_kernel k) {
	if(k)
		clReleaseKernel(k);
	return 0;
}
void mcl_kernel_setarg(cl_kernel k, mcl_arg_t arg, size_t arg_size, const void *arg_value) {
	mcl_throw_if_error(clSetKernelArg(k, arg->position, arg_size, arg_value));
}
