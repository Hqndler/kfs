#pragma once
#include <stddef.h>
#include <stdint.h>

#include "kernel.h"
#include "virtual_manager.h"
#include <stdint.h>

#define PAGE_PRESENT 0x1
#define PAGE_RW 0x2
#define PAGE_USER 0x4

#define PAGE_DIRECTORY_SIZE 1024
#define PAGE_TABLE_SIZE 1024

#define PAGE_SIZE 4096
#define PAGE_ENTRIES 1024
#define KERNEL_VIRTUAL_BASE 0xC0000000
#define KERNEL_PHYSICAL_BASE 0x00100000

#define KERNEL_VIRTUAL_BASE 0xC0000000
#define KERNEL_HEAP_START (KERNEL_VIRTUAL_BASE + 0x1000000)

#define MAX_PAGES 1048576

#define BITMAP_SET(bm, i) ((bm)->data[(i) / 32] |= (1U << ((i) % 32)))
#define BITMAP_CLEAR(bm, i) ((bm)->data[(i) / 32] &= ~(1U << ((i) % 32)))
#define BITMAP_TEST(bm, i) ((bm)->data[(i) / 32] & (1U << ((i) % 32)))

extern uint32_t page_directory[PAGE_DIRECTORY_SIZE];
extern uint32_t page_table[PAGE_TABLE_SIZE];
extern uintptr_t next_free_page;
extern size_t allocated_pages;

#define V2P(addr) ((uint32_t)addr - KERNEL_VIRTUAL_BASE)

void init_paging();
void map_page(void *physical_addr, void *virtual_addr, uint32_t flags);
void free_page(void *addr);
void *alloc_page(void);
void *alloc_cpages(size_t count);
