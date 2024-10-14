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
	for (uint32_t page = (uint32_t)KERNEL_START - KERNEL_VIRTUAL_BASE;
		 page < KERNEL_END - KERNEL_VIRTUAL_BASE; page++)
		BITMAP_SET(&physical_bitmap, page);
}

void *alloc_early_boot_memory(size_t size) {
	size = (size + 7) & ~(0b111); // just to confuse you
	if (early_boot_next_free + size > EARLY_KMALLOC_END)
		return NULL;
	void *allocated = (void *)early_boot_next_free;
	early_boot_next_free += size;
	return allocated;
}
