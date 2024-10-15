#include "heap.h"

void init_bitmaps(struct multiboot_info *mbi) {
	uint32_t total_memory = (mbi->mem_lower + mbi->mem_upper) * 1024;
	uint32_t total_pages = total_memory / PAGE_SIZE;
	uint32_t size_bitmap = (total_pages + 31) / 32;

	physical_bitmap.data =
		alloc_early_boot_memory(size_bitmap * sizeof(uint32_t));
	physical_bitmap.size = size_bitmap;

	kmemset(physical_bitmap.data, 0xFF,
			physical_bitmap.size * sizeof(uint32_t));

	multiboot_memory_map_t *mmap = (multiboot_memory_map_t *)mbi->mmap_addr;
	multiboot_memory_map_t *mmap_end =
		(multiboot_memory_map_t *)(mbi->mmap_addr + mbi->mmap_length);

	while (mmap < mmap_end) {
		if (mmap->type == MULTIBOOT_MEMORY_AVAILABLE && mmap->addr != 0) {
			uint32_t start_page = (uint32_t)(mmap->addr / PAGE_SIZE);
			uint32_t end_page =
				(uint32_t)((mmap->addr + mmap->len) / PAGE_SIZE);

			for (uint32_t page = start_page; page < end_page; page++)
				BITMAP_CLEAR(&physical_bitmap, page);
		}

		mmap = (multiboot_memory_map_t *)((uint32_t)mmap + mmap->size +
										  sizeof(mmap->size));
	}

	uint32_t start_page = (KERNEL_PHYSICAL_BASE) / PAGE_SIZE;
	uint32_t end_page = ((KERNEL_END - KERNEL_VIRTUAL_BASE)) / PAGE_SIZE;

	for (uint32_t page = start_page; page < end_page; page++)
		BITMAP_SET(&physical_bitmap, page);
}

void *alloc_early_boot_memory(size_t size) {
	size = (size + (sizeof(uint8_t) - 1)) & ~(sizeof(uint8_t) - 1);
	if (early_boot_next_free + size > EARLY_KMALLOC_END)
		return NULL;
	void *allocated = (void *)early_boot_next_free;
	early_boot_next_free += size;
	return allocated;
}

static slab_cache_t caches[NUM_CACHES];

void init_slab_allocator() {
	size_t sizes[] = {16, 32, 64, 128, 256, 512, 1024, 2048};
	for (int i = 0; i < NUM_CACHES; i++) {
		caches[i].size = sizes[i];
		caches[i].max_obj = (PAGE_SIZE - sizeof(slab_t)) / sizes[i];
		caches[i].slabs = NULL;
	}
}

static void *allocate_slab(slab_cache_t *cache) {
	slab_t *new_slab = get_pages(1);
	if (!new_slab)
		return NULL;

	new_slab->next = cache->slabs;
	cache->slabs = new_slab;

	uint8_t *slab_mem = (uint8_t *)new_slab + sizeof(slab_t);
	new_slab->free_list = slab_mem;

	for (size_t i = 0; i < cache->max_obj - 1; i++) {
		void **obj = (void **)(slab_mem + i * cache->size);
		*obj = slab_mem + (i + 1) * cache->size;
	}
	void **last_obj = (void **)(slab_mem + (cache->max_obj - 1) * cache->size);
	*last_obj = NULL;

	return new_slab;
}

static void *slab_alloc(size_t size, int contig) {

	size += sizeof(slab_cache_t);

	slab_cache_t *cache = NULL;
	size_t cache_index = LOG2(UPPER_2_POWER(size)) - 4;
	if (cache_index < NUM_CACHES)
		cache = &caches[cache_index];

	if (!cache) {
		return contig ? get_cpages((size / PAGE_SIZE) + 1) :
						get_pages((size / PAGE_SIZE) + 1);
	}

	if (!cache->slabs || !cache->slabs->free_list) {
		if (!allocate_slab(cache))
			return NULL;
	}

	slab_object_t *object = cache->slabs->free_list;
	cache->slabs->free_list = *(void **)object;
	object->cache = cache;
	return object->data;
}

static void slab_free(void *ptr) {
	if (!ptr)
		return;

	slab_object_t *object =
		(slab_object_t *)((uint8_t *)ptr - offsetof(slab_object_t, data));
	slab_t *slab = (slab_t *)((uintptr_t)object & ~(PAGE_SIZE - 1));
	*(void **)object = slab->free_list;
	slab->free_list = object;
}

void *kmalloc(size_t size) {
	return slab_alloc(size, 1);
}

void kfree(void *ptr) {
	slab_free(ptr);
}

void *vmalloc(size_t size) {
	return slab_alloc(size, 0);
}
