
#ifndef MAY_MCL_ILOADER_H
#define MAY_MCL_ILOADER_H

#include <may/core/floader.h>
#include <may/core/map.h>
#include "image.h"

struct iloader_ss;
typedef struct iloader_ss iloader_s;
typedef iloader_s *iloader_t;

typedef struct iloader_node_ss {
	heap_t heap;
	str_t name;
	cl_mem image;
	iloader_t iloader;
	struct iloader_node_ss *parent;
	struct iloader_node_ss *children[2];
} iloader_node_s;

typedef iloader_node_s *iloader_node_t;

struct iloader_ss {
	cl_context context;
	floader_t file_loader;
	iloader_node_t node;
};

iloader_t iloader_create(floader_t fl, cl_context);
iloader_t iloader_delete(iloader_t);
cl_mem iloader_load(iloader_t, str_t);
cl_mem iloader_load_cs(iloader_t, const char *);
cl_mem iloader_unload(iloader_t, cl_mem);


#endif /* MAY_MCL_ILOADER_H */
