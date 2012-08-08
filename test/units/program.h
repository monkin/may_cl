#include <may/cl/mcl.h>
#include <may/cl/ex.h>
#include <may/cl/program.h>
#include <may/cl/error.h>

void test_mcl_program() {
	TEST_MODULE("mcl_program");
	heap_t h = heap_create(0);
	err_try {
		cl_uint num_platforms;
		int i, j;
		cl_platform_id *platforms;
		mcl_throw_if_error(clGetPlatformIDs(0, 0, &num_platforms));
		platforms = heap_alloc(h, sizeof(cl_platform_id[num_platforms]));
		mcl_throw_if_error(clGetPlatformIDs(num_platforms, platforms, 0));
		for(i=0; i<num_platforms; i++) {
			cl_uint num_devices;
			cl_device_id *devices;
			mcl_throw_if_error(clGetDeviceIDs(platforms[i], CL_DEVICE_TYPE_ALL, 0, 0, &num_devices));
			devices = heap_alloc(h, sizeof(cl_device_id[num_devices]));
			mcl_throw_if_error(clGetDeviceIDs(platforms[i], CL_DEVICE_TYPE_ALL, num_devices, devices, 0));
			for(j=0; j<num_devices; j++) {
				char *device_name;
				size_t device_name_len;
				mcl_throw_if_error(clGetDeviceInfo(devices[j], CL_DEVICE_NAME, 0, 0, &device_name_len));
				device_name = heap_alloc(h, device_name_len);
				mcl_throw_if_error(clGetDeviceInfo(devices[j], CL_DEVICE_NAME, device_name_len, device_name, 0));
				TEST_CHECK(device_name) {
					cl_int code;
					cl_mem buff = 0;
					cl_context context = 0;
					cl_program program = 0;
					cl_kernel kernel = 0;
					cl_command_queue queue = 0;
					size_t items_count = 4;
					err_try {
						context = clCreateContext(0, 1, devices+j, 0, 0, &code);
						mcl_throw_if_error(code);

						mcl_arg_s mul_arg;
						mcl_arg_s buff_arg;
						cl_uint const_param = 0;
						mcl_ex_t global_id = mcl_call_1_cs(h, "get_global_id", mcl_const(h, MCLT_UINT, &const_param));
						cl_program program = mcl_program_create(context, mcl_call_2_cs(h, "=",
							mcl_call_2_cs(h, "[]",
								mcl_arg(h, mclt_pointer(MCLT_FLOAT, MCLT_P_GLOBAL), &buff_arg),
								global_id),
							mcl_call_2_cs(h, "*",
								mcl_arg(h, MCLT_UINT, &mul_arg),
								global_id)));
						err_try {
							mcl_program_build(program);
						} err_catch {
							TEST_LOG("\n--- source ---\n");
							TEST_LOG(str_begin(mcl_program_source(h, program)));
							TEST_LOG("\n--- build log ---\n");
							TEST_LOG(str_begin(mcl_program_log(h, program)));
							TEST_LOG("\n--- end ---\n");
							err_throw_down();
						}
						kernel = mcl_kernel_create(program);
						buff = clCreateBuffer(context, CL_MEM_READ_WRITE, sizeof(float[items_count]), 0, &code);
						mcl_throw_if_error(code);
						cl_uint mul_arg_val = 6;
						mcl_kernel_setarg(kernel, &mul_arg, sizeof(mul_arg_val), &mul_arg_val);
						mcl_kernel_setarg(kernel, &buff_arg, sizeof(buff), &buff);
						queue = clCreateCommandQueue(context, devices[j], 0, &code);
						mcl_throw_if_error(code);
						cl_float *data = heap_alloc(h, sizeof(cl_float[items_count]));
						mcl_throw_if_error(clEnqueueNDRangeKernel(queue, kernel, 1, 0, &items_count, 0, 0, 0, 0));
						mcl_throw_if_error(clEnqueueBarrier(queue));
						mcl_throw_if_error(clEnqueueReadBuffer(queue, buff, CL_TRUE, 0, sizeof(cl_float[items_count]), data, 0, 0, 0));
						mcl_throw_if_error(clFinish(queue));
						int n;
						for(n=0; n<items_count; n++) {
							if(data[n]!=(mul_arg_val*n))
								TEST_FAIL;
						}
						clReleaseCommandQueue(queue); queue = 0;
						kernel = mcl_kernel_delete(kernel);
						clReleaseMemObject(buff);
						mcl_program_delete(program);
						clReleaseContext(context);
					} err_catch {
						if(queue) {
							clReleaseCommandQueue(queue); queue = 0;
						}
						if(kernel)
							kernel = mcl_kernel_delete(kernel);
						if(buff)
							clReleaseMemObject(buff);
						if(program)
							mcl_program_delete(program);
						if(context)
							clReleaseContext(context);
						err_throw_down();
					}
				} TEST_END;
			}
		}
		h = heap_delete(h);
	} err_catch {
		h = heap_delete(h);
		err_throw_down();
	}
}
