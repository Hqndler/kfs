#pragma once

#include "multiboot.h"
#include "paging.h"
#include "panic.h"

typedef struct bitmap {
	uint32_t *data;
	uint32_t size;
} bitmap_t;

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
	size_t size;
	uint8_t data[];
} slab_object_t;

typedef struct big_object {
	size_t size;
	void *ptr;
	struct big_object *next;
} big_object_t;

#define ALIGN(x, a) (x + (a - 1)) & ~(a - 1)
#define NUM_CACHES 7

#define LOG2(X)                                                                \
	((unsigned)(8 * sizeof(unsigned long long) - __builtin_clzll((X)) - 1))

#define UPPER_2_POWER(x) (1 << (32 - __builtin_clz(x - 1)))

void *alloc_early_boot_memory(size_t size);

extern bitmap_t virtual_bitmap;
extern bitmap_t physical_bitmap;
extern big_object_t *big_list;

void init_bitmaps(struct multiboot_info *mbi);
void *kmalloc(size_t size);
void init_slab_allocator();
void kfree(void *ptr);
void *kbrk(int32_t increment);
void init_brk();
void *vbrk(int32_t increment);
size_t vsize(void const *ptr);
size_t ksize(void const *ptr);

void print_big_list();
