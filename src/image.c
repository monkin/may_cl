
#include "image.h"
#include "error.h"
#include "mcl.h"
#include <IL/il.h>
#include <CL/cl.h>
#include <stdbool.h>

ERR_DEFINE(e_mcl_image_loading, "Image loading error.", e_mcl_error);

cl_mem mcl_image_create(cl_context context, str_t data) {
	mcl_image_create_bin(context, str_begin(data), str_length(data));
}

static void destroy_queue_list(cl_command_queue *ql, cl_uint sz, cl_int code) {
	int i = 0;
	for(i=0; i<sz; i++)
		if(ql[i])
			clReleaseCommandQueue(ql[i]);
	mcl_throw(code);
}

cl_mem mcl_image_create_bin(cl_context context, const void *data, size_t len) {
	cl_int code;
	ILuint ilim = 0;
	cl_mem clim = 0;
	err_try {
		ilGenImages(1, &ilim);
		ilBindImage(ilim);
		if(!ilLoadL(IL_TYPE_UNKNOWN, data, len))
			err_throw(e_mcl_image_loading);
		ilEnable(IL_ORIGIN_SET);
		ilEnable(IL_FORMAT_SET);
		bool need_convertion = false;
		cl_image_format clformat;
		ILint ilformat = ilGetInteger(IL_IMAGE_FORMAT);
		ILint iltype = ilGetInteger(IL_IMAGE_TYPE);
		cl_channel_order *chanel_order = &clformat.image_channel_order;
		cl_channel_type *chanel_type = &clformat.image_channel_data_type;
		switch(ilformat) {
			case IL_ALPHA:
				*chanel_order = CL_A;
				break;
			case IL_RGBA:
				*chanel_order = CL_RGBA;
				break;
			case IL_BGRA:
				*chanel_order = CL_BGRA;
				break;
			case IL_LUMINANCE:
				*chanel_order = CL_LUMINANCE;
				break;
			default:
				need_convertion = true;
				*chanel_order = CL_RGBA;
				ilformat = IL_RGBA;
				break;
		}
		switch(iltype) {
			case IL_BYTE:
				*chanel_type = CL_SNORM_INT8;
				break;
			case IL_UNSIGNED_BYTE:
				*chanel_type = CL_UNORM_INT8;
				break;
			case IL_SHORT:
				*chanel_type = CL_SNORM_INT16;
				break;
			case IL_UNSIGNED_SHORT:
				*chanel_type = CL_UNORM_INT16;
				break;
			case IL_FLOAT:
				*chanel_type = CL_FLOAT;
				break;
			default:
				need_convertion = true;
				*chanel_type = CL_FLOAT;
				iltype = IL_FLOAT;
				break;
		}
		if(need_convertion) {
			if(!ilConvertImage(ilformat, iltype))
				err_throw(e_mcl_image_loading);
		}
		clim = clCreateImage2D(context, CL_MEM_READ_ONLY, &clformat,
				ilGetInteger(IL_IMAGE_WIDTH), ilGetInteger(IL_IMAGE_HEIGHT), 0, 0, &code);
		mcl_throw_if_error(code);

		cl_uint num_devices;
		mcl_throw_if_error(clGetContextInfo(context, CL_CONTEXT_NUM_DEVICES, sizeof(num_devices), &num_devices, 0));
		heap_t heap = heap_create(0);
		err_try {
			cl_uint i;
			cl_device_id *devices = heap_alloc(heap, sizeof(cl_device_id[num_devices]));
			mcl_throw_if_error(clGetContextInfo(context, CL_CONTEXT_DEVICES, sizeof(cl_device_id[num_devices]), devices, 0));
			cl_command_queue *queue_list = heap_alloc(heap, sizeof(cl_command_queue[num_devices]));
			for(i=0; i<num_devices; i++) {
				queue_list[i] = clCreateCommandQueue(context, devices[i], 0, &code);
				if(code!=CL_SUCCESS)
					destroy_queue_list(queue_list, i, code);
			}
			size_t origin[3] = {0, 0, 0};
			size_t region[3];
			region[0] = ilGetInteger(IL_IMAGE_WIDTH);
			region[1] = ilGetInteger(IL_IMAGE_HEIGHT);
			region[2] = 1;
			for(i=0; i<num_devices; i++) {
				code = clEnqueueWriteImage(queue_list[i], clim, CL_FALSE, origin, region, 0, 0, ilGetData(), 0, 0, 0);
				if(code!=CL_SUCCESS)
					destroy_queue_list(queue_list, num_devices, code);
			}
			for(i=0; i<num_devices; i++) {
				code = clFinish(queue_list[i]);
				if(code!=CL_SUCCESS)
					destroy_queue_list(queue_list, num_devices, code);
			}
			for(i=0; i<num_devices; i++)
				mcl_throw_if_error(clReleaseCommandQueue(queue_list[i]));
			heap = heap_delete(heap);
		} err_catch {
			heap = heap_delete(heap);
		}
		ilDeleteImages(1, &ilim);
		return clim;
	} err_catch {
		if(ilim)
			ilDeleteImages(1, &ilim);
		if(clim)
			clReleaseMemObject(clim);
		err_throw_down();
	}
}
cl_mem mcl_image_delete(cl_mem img) {
	if(img)
		clReleaseMemObject(img);
	return 0;
}
void mcl_image_init() {
	ilInit();
}
