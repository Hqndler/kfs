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

void *alloc_page(void) {
	if (allocated_pages >= MAX_PAGES)
		return NULL;

	void *page = (void *)next_free_page;
	next_free_page += PAGE_SIZE;
	allocated_pages++;

	return page;
}

void map_page(void *physical_addr, void *virtual_addr, uint32_t flags) {
	uint32_t pd_index = (uint32_t)virtual_addr >> 22;
	uint32_t pt_index = ((uint32_t)virtual_addr >> 12) & 0x3FF;

	uint32_t *page_table;

	if (!(page_directory[pd_index] & PAGE_PRESENT)) {
		page_table = (uint32_t *)alloc_page();
		if (!page_table) {
			return;
		}
		kmemset(page_table, 0, PAGE_SIZE);
		page_directory[pd_index] =
			((uint32_t)page_table) | PAGE_PRESENT | PAGE_RW | PAGE_USER;
	}
	else
		page_table = (uint32_t *)(page_directory[pd_index] & ~0xFFF);

	page_table[pt_index] = ((uint32_t)physical_addr) | flags;

	asm volatile("invlpg (%0)" : : "r"(virtual_addr) : "memory");
}

void *alloc_and_map_pages(size_t num_pages, void *virtual_addr) {
	for (size_t i = 0; i < num_pages; i++) {
		void *physical_page = alloc_page();
		if (!physical_page) {
			return NULL;
		}
		map_page(physical_page, virtual_addr,
				 PAGE_PRESENT | PAGE_RW | PAGE_USER);
		virtual_addr = (char *)virtual_addr + PAGE_SIZE;
	}
	return (char *)virtual_addr - (num_pages * PAGE_SIZE);
}

void *kernel_allocate_pages(size_t num_pages) {
	static void *next_free_virtual = (void *)KERNEL_HEAP_START;
	void *allocated = alloc_and_map_pages(num_pages, next_free_virtual);
	if (allocated)
		next_free_virtual = (char *)next_free_virtual + (num_pages * PAGE_SIZE);
	return allocated;
}
