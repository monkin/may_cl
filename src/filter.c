
#include "filter.h"

/* filter_arguments */

filter_arguments_t filter_arguments_create(heap_t h) {
	filter_arguments_t fa = heap_alloc(h, sizeof(filter_arguments_s));
	fa->heap = h;
	fa->events = 0;
	fa->arguments = 0;
	fa->events_count = fa->arguments_count = 0;
	return fa;
}
void filter_arguments_push_event(filter_arguments_t fa, cl_event e) {
	filter_arguments_event_t fae = heap_alloc(fa->heap, sizeof(filter_arguments_event_s));
	fae->event = e;
	fae->next = fa->events;
	fa->events = fae;
	fa->events_count++;
}
void filter_arguments_push_argument(filter_arguments_t fa, mcl_arg_t arg, const void *data, size_t data_size) {
	filter_arguments_arg_t faa = heap_alloc(fa->heap, sizeof(filter_arguments_arg_s));
	faa->argument = arg;
	faa->value = heap_alloc(fa->heap, data_size);
	memcpy(faa->value, data, data_size);
	faa->value_size = data_size;
	faa->next = fa->arguments;
	fa->arguments = faa;
	fa->arguments_count++;
}

/* filter */

#include "filters/const.h"
#include "filters/blur.h"

ERR_DEFINE(e_filter_not_found, "Filter not found", e_mcl_error);
ERR_DEFINE(e_filter_invalid_arguments, "Invalid filter arguments", e_mcl_error);

static heap_t filter_heap = 0;
static map_t filter_map = 0;

static void uregister_filters() {
	filter_heap = heap_delete(filter_heap);
}
void filter_init() {
	filter_heap = heap_create(0);
	err_try {
		filter_map = map_create(filter_heap);
		atexit(uregister_filters);
	} err_catch {
		filter_heap = heap_delete(filter_heap);
		err_throw_down();
	}
	filter_register(&flt_controller_blur);
	filter_register(&flt_controller_const);
}
void filter_register(filter_controller_t c) {
	map_set_cs(filter_map, c->name, c);
}

static filter_t filter_create_internal(filter_controller_t controller, flcontext_t context, json_value_t config, map_t arguments) {
	heap_t h = heap_create(16*1024);
	filter_t filter = heap_alloc(h, sizeof(filter_s));
	filter->heap = h;
	filter->controller = controller;
	filter->controller_data = 0;
	filter->context = context;
	filter->config = config;
	filter->arguments = arguments;
	filter_t background = map_get_cs(arguments, "_");
	filter->type = background ? background->type : 0;
	if(controller->init) {
		err_try {
			controller->init(filter);
		} err_catch {
			h = heap_delete(h);
			err_throw_down();
		}
	}
	return filter;
}

filter_t filter_create(str_t s, flcontext_t context, json_value_t config, map_t arguments) {
	filter_controller_t controller = map_get(filter_map, s);
	if(controller)
		return filter_create_internal(controller, context, config, arguments);
	else
		err_throw(e_filter_not_found);
}

filter_t filter_create_cs(const char *s, flcontext_t context, json_value_t config, map_t arguments) {
	filter_controller_t controller = map_get_cs(filter_map, s);
	if(controller)
		return filter_create_internal(controller, context, config, arguments);
	else
		err_throw(e_filter_not_found);
}

filter_t filter_delete(filter_t filter) {
	if(filter) {
		if(filter->controller->destroy)
			filter->controller->destroy(filter);
		heap_delete(filter->heap);
	}
	return 0;
}
mcl_ex_t filter_get_expression(heap_t heap, filter_t filter, mcl_ex_t point, filter_arguments_t fa) {
	return filter->controller->get_expression(heap, filter, point, fa);
}


