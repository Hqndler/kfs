#pragma once

#include "heap.h"
#include "paging.h"
#define NUM_AREA 1024

typedef struct vm_area {
	uint32_t start;
	uint32_t end;
} vm_area_t;

typedef struct vm_manager {
	vm_area_t *areas;
	size_t num_area;
	struct vm_manager *next;
} vm_manager_t;

void init_vm_manager();
void print_area();
void add_free(void *ptr, size_t size);
void *find_free(size_t size);
void *get_cpages(size_t count);
void *get_pages(size_t count);
