#pragma once

#include "multiboot.h"
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

typedef struct slab {
	void *free_list;
	struct slab *next;
} slab_t;

typedef struct slab_cache {
	size_t size;
	size_t max_obj;
	slab_t *slabs;
} slab_cache_t;

typedef struct {
	slab_cache_t *cache;
	uint8_t data[];
} slab_object_t;

#define ALIGN(x, a) (x + (a - 1)) & ~(a - 1);
#define NUM_CACHES 8

#define LOG2(X)                                                                \
	((unsigned)(8 * sizeof(unsigned long long) - __builtin_clzll((X)) - 1))

#define UPPER_2_POWER(x) (1 << (32 - __builtin_clz(x - 1)))

void *alloc_early_boot_memory(size_t size);

#define HEAP_LIMIT (PAGE_SIZE * 1024)

extern bitmap_t virtual_bitmap;
extern bitmap_t physical_bitmap;
extern block_t *free_list;

void init_bitmaps(struct multiboot_info *mbi);
void *kmalloc(size_t size);
void init_slab_allocator();
void kfree(void *ptr);
