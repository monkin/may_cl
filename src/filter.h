
#ifndef MAY_MCL_FILTER_H
#define MAY_MCL_FILTER_H

#include "filter_context.h"
#include "ex.h"
#include "mcl.h"
#include <may/core/json.h>
#include <may/core/map.h>
#include <may/core/floader.h>
#include "iloader.h"
#include <stdbool.h>
#include <CL/cl.h>

/* filter_arguments */

typedef struct filter_arguments_event_ss {
	cl_event event;
	struct filter_arguments_event_ss *next;
} filter_arguments_event_s;

typedef filter_arguments_event_s *filter_arguments_event_t;

typedef struct filter_arguments_arg_ss {
	mcl_arg_t argument;
	void *value;
	size_t value_size;
	struct filter_arguments_arg_ss *next;
} filter_arguments_arg_s;

typedef filter_arguments_arg_s *filter_arguments_arg_t;

typedef struct {
	heap_t heap;
	filter_arguments_event_t events;
	size_t events_count;
	filter_arguments_arg_t arguments;
	size_t arguments_count;
} filter_arguments_s;

typedef filter_arguments_s *filter_arguments_t;

filter_arguments_t filter_arguments_create(heap_t);
void filter_arguments_push_event(filter_arguments_t, cl_event);
void filter_arguments_push_argument(filter_arguments_t, mcl_arg_t, const void *, size_t);

/* filter */

ERR_DECLARE(e_filter_not_found);
ERR_DECLARE(e_filter_invalid_arguments);

struct filter_ss;
typedef struct filter_ss filter_s;
typedef filter_s *filter_t;

struct filter_controller_ss;
typedef struct filter_controller_ss filter_controller_s;
typedef filter_controller_s *filter_controller_t;

struct filter_controller_ss {
	const char *name;
	void (*init)(filter_t);
	void (*destroy)(filter_t);
	mcl_ex_t (*get_expression)(heap_t, filter_t, mcl_ex_t point, filter_arguments_t);
};

struct filter_ss {
	mclt_t type;
	heap_t heap;
	flcontext_t context;
	filter_controller_t controller;
	void *controller_data;
	json_value_t config;
	map_t arguments;
};

filter_t filter_create(str_t, flcontext_t, json_value_t config, map_t arguments);
filter_t filter_create_cs(const char *, flcontext_t, json_value_t config, map_t arguments);
filter_t filter_delete(filter_t);
mcl_ex_t filter_get_expression(heap_t, filter_t, mcl_ex_t point, filter_arguments_t);

void filter_init();
void filter_register(filter_controller_t);

#endif /* MAY_MCL_FILTER_H */

