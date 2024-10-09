#include "paging.h"

uint32_t page_directory[PAGE_DIRECTORY_SIZE] __attribute__((aligned(4096)));
uint32_t page_table[PAGE_TABLE_SIZE] __attribute__((aligned(4096)));

void enablePaging(uint32_t *page_directory) {
	asm volatile("mov %0, %%cr3" ::"r"(page_directory));

	uint32_t cr0;
	asm volatile("mov %%cr0, %0\n"
				 "or $0x80000000, %0\n"
				 "mov %0, %%cr0\n"
				 : "=r"(cr0));
}

static inline uint32_t *_native_get_page_directory() {
	uint32_t ret;
	asm volatile("mov %%cr3, %0" : "=b"(ret));

	return (uint32_t *)ret;
}

uint32_t virt_to_phys(uint32_t virt_addr) {
	uint32_t *pd = NULL;
	uint32_t *pt = NULL;
	uint32_t pde = 0;

	uint32_t index = virt_addr / PAGE_SIZE;
	uint32_t pageDirI = (index / 1024) % 1024;
	uint32_t pageTableI = index % 1024;

	// we are referencing our virtual memory alloc'd page directory
	pd = _native_get_page_directory(); // identity mapped physical address
	pde = (uint32_t)pd[pageDirI];	   // identity mapped physical address
	pt = (uint32_t *)(pde & 0xfffff000);

	return pt[pageTableI] & 0xfffff000;
}

#include "kernel.h"

void init_paging() {
	for (int i = 0; i < PAGE_TABLE_SIZE; i++) {
		page_table[i] = (i * PAGE_SIZE) | PAGE_PRESENT | PAGE_RW; // RW, Présent
	}

	page_directory[0] =
		((uint32_t)page_table) | PAGE_PRESENT | PAGE_RW; // RW, Présent

	for (int i = 1; i < PAGE_DIRECTORY_SIZE; i++) {
		page_directory[i] = 0; // Ne pointe vers rien
	}

	kprint("%p - %p\n", page_directory, virt_to_phys(page_directory));

	// enablePaging(virt_to_phys(page_directory));
}
