#pragma once

#include "paging.h"

typedef struct bitmap {
	uint32_t *data;
	uint32_t size;
} bitmap_t;

typedef struct __attribute__((packed)) block {
	size_t size;
	uint8_t is_used : 1;
	struct block *next;
} block_t;

void *alloc_early_boot_memory(size_t size);

extern bitmap_t virtual_bitmap;
extern bitmap_t physical_bitmap;
extern block_t *free_list;

void init_bitmaps();
