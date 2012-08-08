
#include <may/cl/image.h>

void test_mcl_image() {
	TEST_MODULE("mcl_image");
	static char image_data[] = "GIF87a\020\000\020\000\241\003\000\000\000\000\310" \
		"NNv\272U\377\377\377,\000\000\000\000\020\000\020\000\000\002 \204" \
		"\217\t\302\355/\224<\241\332\213\303\234\260\267-e\242\005*\236W&"  \
		"\343\230\"g\327RkV\000\000;";
	static size_t image_length = 71;
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
				cl_mem img = 0;
				TEST_CHECK(device_name) {
					cl_int code;
					cl_context context = 0;
					err_try {
						context = clCreateContext(0, 1, devices+j, 0, 0, &code);
						mcl_throw_if_error(code);
						img = mcl_image_create_bin(context, image_data, image_length);
						img = mcl_image_delete(img);
						clReleaseContext(context);
					} err_catch {
						img = mcl_image_delete(img);
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