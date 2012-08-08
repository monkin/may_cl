
#include "iloader.h"
#include "error.h"

iloader_t iloader_create(floader_t fl, cl_context ctx) {
	iloader_t r = mem_alloc(sizeof(iloader_node_s));
	r->context = ctx;
	r->file_loader = fl;
	r->node = 0;
	return r;
}
iloader_t iloader_delete(iloader_t fl) {
	if(fl) {
		assert(!fl->node);
		mem_free(fl);
	}
	return 0;
}

static void iloader_destroy_node(cl_mem memobj, void *nd) {
	iloader_node_t node = nd,
		parent = node->parent,
		i = 0;
	if(node->children[0] && node->children[1]) {
		for(i=node->children[0]; i->children[1];)
			i = i->children[1];
		i->children[1] = node->children[1];
		node->children[1]->parent = i;
		node->children[1] = 0;
	}
	if(parent) {	
		if(node->children[0] || node->children[1]) {
			iloader_node_t cnode = node->children[0] ? node->children[0] : node->children[1];
			parent->children[parent->children[0]==node ? 0 : 1] = cnode;
			cnode->parent = parent;
		} else
			parent->children[0] = parent->children[1] = 0;
	} else
		node->iloader->node = node->children[0] ? node->children[0] : node->children[1]; 
	heap_delete(node->heap);	
}

static cl_mem iloader_load_internal(iloader_t il, const void *name, size_t name_length) {
	iloader_node_t i = il->node;
	while(i) {
		switch(str_compare_bin(i->name, name, name_length)) {
		case 0:
			mcl_throw_if_error(clRetainMemObject(i->image));
			return i->image;
		case 1:
			i = i->children[0];
			break;
		case -1:
			i = i->children[1];
			break;
		}
	}
	heap_t h = heap_create(1024);
	heap_t tmp_heap = 0;
	iloader_node_t nnode = 0;
	err_try {
		tmp_heap = heap_create(0);
		nnode = heap_alloc(h, sizeof(iloader_node_s));
		nnode->heap = h;
		nnode->name = str_from_bin(h, name, name_length);
		nnode->iloader = il;
		nnode->parent = nnode->children[0] = nnode->children[1] = 0;
		nnode->image = 0;
		nnode->image = mcl_image_create(il->context, floader_get_str(il->file_loader, tmp_heap, nnode->name));
		tmp_heap = heap_delete(tmp_heap);
		mcl_throw_if_error(clSetMemObjectDestructorCallback(nnode->image, iloader_destroy_node, nnode));
		if(!il->node)
			il->node = nnode;
		else {
			i = il->node;
			while(1) {
				int index = str_compare(i->name, nnode->name)>0 ? 0 : 1;
				if(i->children[index])
					i = i->children[index];
				else {
					i->children[index] = nnode;
					nnode->parent = i;
					return nnode->image;
				}
			}
		}
	} err_catch {
		if(nnode ? nnode->image : false)
			mcl_image_delete(nnode->image);
		h = heap_delete(h);
		tmp_heap = heap_delete(tmp_heap);
		err_throw_down();	
	}
}

cl_mem iloader_load(iloader_t il, str_t fname) {
	return iloader_load_internal(il, str_begin(fname), str_length(fname));
}
cl_mem iloader_load_cs(iloader_t il, const char *fname) {
	return iloader_load_internal(il, fname, strlen(fname));
}
cl_mem iloader_unload(iloader_t il, cl_mem img) {
	mcl_throw_if_error(clReleaseMemObject(img));
	return 0;
}
