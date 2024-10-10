#include "heap.h"

void *alloc_early_boot_memory(size_t size) {
	size = (size + (sizeof(uint8_t) - 1)) & ~(sizeof(uint8_t) - 1);
	if (early_boot_next_free + size > EARLY_KMALLOC_END)
		return NULL;
	void *allocated = (void *)early_boot_next_free;
	early_boot_next_free += size;
	return allocated;
}
