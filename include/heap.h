#pragma once

#include "paging.h"

typedef struct bitmap {
	uint32_t *data;
	uint32_t size;
} bitmap_t;

void *alloc_early_boot_memory(size_t size);

extern bitmap_t virtual_bitmap;
extern bitmap_t physical_bitmap;

void init_bitmaps();
