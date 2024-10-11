#include "heap.h"

void init_bitmaps(struct multiboot_info *mbi) {
	uint32_t total_memory = (mbi->mem_lower + mbi->mem_upper) * 1024;
	uint32_t total_pages = total_memory / PAGE_SIZE;
	uint32_t size_bitmap = (total_pages + 31) / 32;
	virtual_bitmap.data =
		alloc_early_boot_memory(size_bitmap * sizeof(uint32_t));
	virtual_bitmap.size = size_bitmap;

	physical_bitmap.data =
		alloc_early_boot_memory(size_bitmap * sizeof(uint32_t));
	physical_bitmap.size = size_bitmap;
}

void *alloc_early_boot_memory(size_t size) {
	size = (size + (sizeof(uint8_t) - 1)) & ~(sizeof(uint8_t) - 1);
	if (early_boot_next_free + size > EARLY_KMALLOC_END)
		return NULL;
	void *allocated = (void *)early_boot_next_free;
	early_boot_next_free += size;
	return allocated;
}
