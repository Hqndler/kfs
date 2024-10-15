#include "paging.h"

static void set_page_directory_entry(uint32_t index, uint32_t address,
									 uint32_t flags) {
	page_directory[index] = address | flags;
}

static void set_page_table_entry(uint32_t index, uint32_t address,
								 uint32_t flags) {
	page_table[index] = address | flags;
}

void init_paging(void) {
	for (size_t i = 0; i < PAGE_ENTRIES; i++)
		set_page_directory_entry(i, 0, 0);

	for (size_t i = 0; i < PAGE_ENTRIES; i++)
		set_page_table_entry(i, i * PAGE_SIZE, PAGE_PRESENT | PAGE_RW);

	set_page_directory_entry(0, V2P(page_table), PAGE_PRESENT | PAGE_RW);
	set_page_directory_entry(1023, V2P(page_directory), PAGE_PRESENT | PAGE_RW);

	uint32_t kernel_pd_index = KERNEL_VIRTUAL_BASE / (PAGE_SIZE * PAGE_ENTRIES);
	set_page_directory_entry(kernel_pd_index, V2P(page_table),
							 PAGE_PRESENT | PAGE_RW);

	uint32_t pd_addr = V2P(page_directory);
	asm volatile("mov %0, %%cr3" : : "r"(pd_addr));

	uint32_t cr0;
	asm volatile("mov %%cr0, %0" : "=r"(cr0));
	cr0 |= 0x80000000; // Set PG bit
	asm volatile("mov %0, %%cr0" : : "r"(cr0));
	early_boot_next_free = EARLY_KMALLOC_START;
	next_free_page = KERNEL_END - 0xC0000000;
}

void *alloc_cpages(size_t count) {
	for (size_t i = 0; i < (physical_bitmap.size * 32) - (count + 1); i++) {
		bool isContig = true;
		for (size_t j = 0; j < count; j++) {
			if (BITMAP_TEST(&physical_bitmap, i + j)) {
				isContig = false;
				continue;
			}
		}
		if (isContig) {
			for (size_t j = 0; j < count; j++)
				BITMAP_SET(&physical_bitmap, i + j);
			return (void *)(i * PAGE_SIZE);
		}
	}
	return NULL;
}

void *alloc_page(void) {
	for (size_t i = 0; i < physical_bitmap.size * 32; i++) {
		if (!BITMAP_TEST(&physical_bitmap, i)) {
			BITMAP_SET(&physical_bitmap, i);
			return (void *)(i * PAGE_SIZE);
		}
	}
	return NULL;
}

#define RECURSIVE_PAGE_DIR_BASE 0xFFFFF000
#define RECURSIVE_PAGE_TABLE_BASE 0xFFC00000

uintptr_t virtual_to_physical(void *virtual_addr) {
	uint32_t pd_index = (uint32_t)virtual_addr >> 22;
	uint32_t pt_index = ((uint32_t)virtual_addr >> 12) & 0x3FF;
	uint32_t page_offset = (uint32_t)virtual_addr & 0xFFF;

	uint32_t *pde = (uint32_t *)(RECURSIVE_PAGE_DIR_BASE + (pd_index * 4));
	if (!(*pde & PAGE_PRESENT))
		return 0;

	uint32_t *pte = (uint32_t *)(RECURSIVE_PAGE_TABLE_BASE +
								 (pd_index * 0x1000) + (pt_index * 4));
	if (!(*pte & PAGE_PRESENT))
		return 0;

	return (*pte & ~0xFFF) | page_offset;
}

void unmap_page(void *virtual_addr) {
	uint32_t pd_index = (uint32_t)virtual_addr >> 22;
	uint32_t pt_index = ((uint32_t)virtual_addr >> 12) & 0x3FF;

	uint32_t *pte = (uint32_t *)(RECURSIVE_PAGE_TABLE_BASE +
								 (pd_index * 0x1000) + (pt_index * 4));
	*pte = 0;

	asm volatile("invlpg (%0)" : : "r"(virtual_addr) : "memory");
}

void free_page(void *addr) {
	uint32_t phy_adr = virtual_to_physical(addr);

	size_t page_index = phy_adr / PAGE_SIZE;

	if ((uint32_t)addr % PAGE_SIZE != 0)
		return;

	if (page_index >= (physical_bitmap.size * 32))
		return;

	if (!BITMAP_TEST(&physical_bitmap, page_index))
		return; // DOUBLE FREE

	kmemset(addr, 0, PAGE_SIZE);
	unmap_page(addr);
	add_free(addr, PAGE_SIZE);
	BITMAP_CLEAR(&physical_bitmap, page_index);
}

void map_page(void *physical_addr, void *virtual_addr, uint32_t flags) {
	uint32_t pd_index = (uint32_t)virtual_addr >> 22;
	uint32_t pt_index = ((uint32_t)virtual_addr >> 12) & 0x3FF;

	uint32_t *pde = (uint32_t *)(RECURSIVE_PAGE_DIR_BASE + (pd_index * 4));

	if (!(*pde & PAGE_PRESENT)) {
		// void *new_table = alloc_cpages(1);
		void *new_table =
			virtual_to_physical(alloc_early_boot_memory(PAGE_SIZE));

		if (!new_table)
			return;
		kmemset(new_table, 0, PAGE_SIZE);
		*pde = (uint32_t)new_table | PAGE_PRESENT | PAGE_RW;
	}

	uint32_t *pte = (uint32_t *)(RECURSIVE_PAGE_TABLE_BASE +
								 (pd_index * 0x1000) + (pt_index * 4));
	*pte = ((uint32_t)physical_addr) | flags;

	asm volatile("invlpg (%0)" : : "r"(virtual_addr) : "memory");
}

void *get_cpages(size_t count) {
	void *ptr = find_free(count * PAGE_SIZE);
	if (!ptr)
		return NULL;
	void *pages = alloc_cpages(count);
	if (!pages)
		return NULL;
	for (size_t i = 0; i < count; i++) {
		map_page(pages + (i * PAGE_SIZE), ptr + (i * PAGE_SIZE),
				 PAGE_PRESENT | PAGE_RW);
	}
	return ptr;
}

void *get_pages(size_t count) {
	void *ptr = find_free(count * PAGE_SIZE);
	if (!ptr)
		return NULL;
	for (size_t i = 0; i < count; i++) {
		void *page = alloc_page();
		if (!page)
			return NULL;
		// kprint("MAP [%p] to [%p]\n", page, ptr + (i * PAGE_SIZE));
		map_page(page, ptr + (i * PAGE_SIZE), PAGE_PRESENT | PAGE_RW);
	}
	return ptr;
}
